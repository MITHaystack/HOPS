#include "MHO_VisibilityChannelizer.hh"


namespace hops
{

//the ordering operator for channel labels to sort by (center) frequency;
class chan_label_freq_predicate
{
    public:
        chan_label_freq_predicate(){};
        virtual ~chan_label_freq_predicate(){};

    virtual bool operator()(const mho_json& a, const mho_json& b)
    {
        double a_freq = a["sky_freq"];
        double b_freq = b["sky_freq"];
        double a_bw = a["bandwidth"];
        double b_bw = b["bandwidth"];
        std::string a_sb = a["net_sideband"];
        std::string b_sb = a["net_sideband"];
        double a_sign = 0;
        double b_sign = 0;
        if(a_sb == "U"){a_sign = 1.0;}
        if(a_sb == "L"){a_sign = -1.0;}
        if(b_sb == "U"){b_sign = 1.0;}
        if(b_sb == "L"){b_sign = -1.0;}
        //figure out the channel center frequencies and compare using that
        double a_center = a_freq + a_sign*(a_bw/2.0);
        double b_center = b_freq + b_sign*(b_bw/2.0);
        return a_center < b_center;
    }
};


MHO_VisibilityChannelizer::MHO_VisibilityChannelizer()
{
    fInitialized = false;
}

MHO_VisibilityChannelizer::~MHO_VisibilityChannelizer(){}

bool
MHO_VisibilityChannelizer::InitializeImpl(const uch_visibility_store_type* in, visibility_store_type* out)
{
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        //we want to re-organize the data in the input array (rank = 3)
        //into another array (with rank-4) with the data split up by channel
        std::size_t input_size = in->GetSize();
        if(input_size == 0){return false;}
        else
        {
            //now we need figure out the dimensions of the ouput array
            std::size_t input_dim[uch_visibility_store_type::rank::value];
            in->GetDimensions(input_dim);

            //and determine the number of unique channel labels
            auto* freq_axis = &(std::get<UCH_FREQ_AXIS>( *(in) ) );
            std::vector< mho_json > channel_labels = freq_axis->GetMatchingIntervalLabels(std::string("channel"));
            std::size_t num_channels = channel_labels.size();

            //make sure the are sorted by center frequency
            chan_label_freq_predicate sort_pred;
            std::sort( channel_labels.begin(), channel_labels.end(), sort_pred);

            //then we need to make sure all channels have the same size
            std::set<std::size_t> channel_sizes;
            for(auto iter = channel_labels.begin(); iter != channel_labels.end(); iter++)
            {
                double sf;
                sf = (*iter)["sky_freq"];
                int upper = (*iter)["upper_index"];
                int lower = (*iter)["lower_index"];
                int length = upper - lower;
                msg_debug("calibration", "inserting channel of size: " << length << " with sky freq: " << sf << eom);
                channel_sizes.insert( length );
            }

            if(channel_sizes.size() != 1)
            {
                msg_warn("calibration", "channel sizes are not a uniform number of spectral points, there are: "<< channel_sizes.size() <<" distinct sizes." << eom);
            }
            std::size_t channel_length = *( channel_sizes.begin() );

            //finally we can re-size the output array so that it is ready
            //to recieve data from each channel
            std::size_t output_dim[visibility_store_type::rank::value];
            output_dim[POLPROD_AXIS] = input_dim[UCH_POLPROD_AXIS];
            output_dim[CHANNEL_AXIS] = num_channels;
            output_dim[TIME_AXIS] = input_dim[UCH_TIME_AXIS];
            output_dim[FREQ_AXIS] = channel_length;

            out->Resize(output_dim[0], output_dim[1], output_dim[2], output_dim[3]);

            auto* in_pp_axis = &(std::get<UCH_POLPROD_AXIS>( *(in) ) );
            auto* in_time_axis = &(std::get<UCH_TIME_AXIS>( *(in) ) );
            auto* in_freq_axis = &(std::get<UCH_FREQ_AXIS>( *(in) ) );

            auto* out_pp_axis = &(std::get<POLPROD_AXIS>( *(out) ) );
            auto* out_channel_axis = &(std::get<CHANNEL_AXIS>( *(out) ) );
            auto* out_time_axis = &(std::get<TIME_AXIS>( *(out) ) );
            auto* out_freq_axis = &(std::get<FREQ_AXIS>( *(out) ) );

            //label the output array polarization axis
            for(std::size_t pp=0; pp<in_pp_axis->GetSize(); pp++)
            {
                out_pp_axis->at(pp) = in_pp_axis->at(pp);
            }

            //label the output channel axis with channel frequencies
            for(std::size_t ch=0; ch<num_channels; ch++)
            {
                double channel_sky_freq = channel_labels[ch]["sky_freq"];
                out_channel_axis->at(ch) = channel_sky_freq;
            }

            //label the ouput array time axis
            for(std::size_t t=0; t<in_time_axis->GetSize(); t++)
            {
                out_time_axis->at(t) = in_time_axis->at(t);
            }
            //label the output array frequency axis (common to all channels!)
            for(std::size_t f=0; f<channel_length; f++)
            {
                //this works temporarily, but may not be ideal
                out_freq_axis->at(f) = in_freq_axis->at(f) - in_freq_axis->at(0);
            }

            fInitialized = true;
            return fInitialized;
        }
    }
    return fInitialized;
}

bool
MHO_VisibilityChannelizer::ExecuteImpl(const uch_visibility_store_type* in, visibility_store_type* out)
{
    if(fInitialized)
    {
        auto* in_pp_axis = &(std::get<UCH_POLPROD_AXIS>( *(in) ) );
        auto* in_time_axis = &(std::get<UCH_TIME_AXIS>( *(in) ) );
        auto* in_freq_axis = &(std::get<UCH_FREQ_AXIS>( *(in) ) );
        auto* out_channel_axis = &(std::get<CHANNEL_AXIS>( *(out) ) );

        //pack the data into the appropriate place
        for(int ch=0; ch<out_channel_axis->GetSize(); ch++)
        {
            auto ch_label = in_freq_axis->GetFirstIntervalWithKeyValue("channel", ch);
            if( !(ch_label.empty()) )
            {
                std::size_t low = ch_label["lower_index"];
                std::size_t up = ch_label["upper_index"];
                for(std::size_t pp=0; pp<in_pp_axis->GetSize(); pp++)
                {
                    for(std::size_t t=0; t<in_time_axis->GetSize(); t++)
                    {
                        for(std::size_t f=low; f<up; f++)
                        {
                            std::size_t f_offset = f-low;
                            (*out)(pp, ch, t, f_offset) = (*in)(pp, t, f);
                        }
                    }
                }

                //attach a fresh channel label here:
                double sky_freq;
                double bw;
                std::string net_sb;
                int channel_id;
                std::string chan_id;
                std::string fband;

                mho_json fresh_ch_label;
                sky_freq = ch_label["sky_freq"];
                bw = ch_label["bandwidth"];
                net_sb = ch_label["net_sideband"];
                channel_id = ch_label["channel"];
                chan_id = ch_label["chan_id"];
                fband = ch_label["frequency_band"];

                fresh_ch_label["sky_freq"] = sky_freq;
                fresh_ch_label["bandwidth"] = bw;
                fresh_ch_label["net_sideband"] = net_sb;
                fresh_ch_label["channel"] = channel_id;
                fresh_ch_label["chan_id"] = chan_id;
                fresh_ch_label["frequency_band"] = fband;
                fresh_ch_label["index"] = ch;
                out_channel_axis->SetLabelObject(fresh_ch_label,ch);
            }
        }


        //polarization product axis
        auto* polprod_axis = &(std::get<POLPROD_AXIS>(*out));
        polprod_axis->Insert(std::string("name"), std::string("polarization_product") );

        //channel axis
        auto* ch_axis = &(std::get<CHANNEL_AXIS>(*out));
        ch_axis->Insert(std::string("name"), std::string("channel") );
        ch_axis->Insert(std::string("units"), std::string("MHz") );

        //AP axis
        auto* ap_axis = &(std::get<TIME_AXIS>(*out));
        ap_axis->Insert(std::string("name"), std::string("time") );
        ap_axis->Insert(std::string("units"), std::string("s") );

        //(sub-channel) frequency axis
        auto* sp_axis = &(std::get<FREQ_AXIS>(*out));
        sp_axis->Insert(std::string("name"), std::string("frequency") );
        sp_axis->Insert(std::string("units"), std::string("MHz") );

        return true;
    }
    else
    {
        msg_error("calibration", "visibility channelizer not initialized." << eom);
        return false;
    }

}



}//end of namespace
