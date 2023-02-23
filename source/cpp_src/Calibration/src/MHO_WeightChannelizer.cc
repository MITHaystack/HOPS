#include "MHO_WeightChannelizer.hh"


namespace hops
{

//the ordering operator for channel labels to sort by frequency;
class chan_label_freq_predicate
{
    public:
        chan_label_freq_predicate(){};
        virtual ~chan_label_freq_predicate(){};

    virtual bool operator()(const MHO_IntervalLabel* a, const MHO_IntervalLabel* b)
    {
        double a_frq, b_frq;
        a->Retrieve(std::string("sky_freq"), a_frq);
        b->Retrieve(std::string("sky_freq"), b_frq);
        return a_frq < b_frq;
    }
};


MHO_WeightChannelizer::MHO_WeightChannelizer()
{
    fInitialized = false;
}

MHO_WeightChannelizer::~MHO_WeightChannelizer(){}

bool
MHO_WeightChannelizer::InitializeImpl(const uch_weight_store_type* in, weight_store_type* out)
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
            std::size_t input_dim[uch_weight_store_type::rank::value];
            in->GetDimensions(input_dim);

            //and determine the number of unique channel labels
            auto* freq_axis = &(std::get<UCH_FREQ_AXIS>( *(in) ) );
            std::vector< const MHO_IntervalLabel* > channel_labels = freq_axis->GetIntervalsWithKey(std::string("channel"));
            std::size_t num_channels = channel_labels.size();

            //make sure the are sorted by sky frequency
            chan_label_freq_predicate sort_pred;
            std::sort( channel_labels.begin(), channel_labels.end(), sort_pred);

            //then we need to make sure all channels have the same size
            std::set<std::size_t> channel_sizes;
            for(auto iter = channel_labels.begin(); iter != channel_labels.end(); iter++)
            {
                double sf;
                (*iter)->Retrieve(std::string("sky_freq"), sf);
                msg_debug("calibration", "Inserting channel of size: " << (*iter)->GetLength() << "with sky freq: " << sf << eom);
                channel_sizes.insert( (*iter)->GetLength() );
            }

            if(channel_sizes.size() != 1)
            {
                msg_warn("calibration", "Channel sizes are not a uniform number of spectral points." << eom);
            }
            std::size_t channel_length = *( channel_sizes.begin() );

            //finally we can re-size the output array so that it is ready
            //to recieve data from each channel
            std::size_t output_dim[weight_store_type::rank::value];
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

            //label the output channel axis with channel id's
            for(std::size_t ch=0; ch<num_channels; ch++)
            {
                int channel_id;
                if( channel_labels[ch]->Retrieve(std::string("channel"), channel_id ) )
                {
                    out_channel_axis->at(ch) = channel_id;
                }
                else
                {
                    msg_warn("calibration", "Warning channel id: "<< channel_id << " not found in channel labels." << eom);
                }
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
MHO_WeightChannelizer::ExecuteImpl(const uch_weight_store_type* in, weight_store_type* out)
{
    if(fInitialized)
    {
        auto* in_pp_axis = &(std::get<UCH_POLPROD_AXIS>( *(in) ) );
        auto* in_time_axis = &(std::get<UCH_TIME_AXIS>( *(in) ) );
        auto* in_freq_axis = &(std::get<UCH_FREQ_AXIS>( *(in) ) );
        auto* out_channel_axis = &(std::get<CHANNEL_AXIS>( *(out) ) );

        //pack the data into the appropriate place
        for(std::size_t ch=0; ch<out_channel_axis->GetSize(); ch++)
        {
            auto ch_label = in_freq_axis->GetFirstIntervalWithKeyValue(std::string("channel"), out_channel_axis->at(ch));
            if( ch_label )
            {
                std::size_t low = ch_label->GetLowerBound();
                std::size_t up = ch_label->GetUpperBound();
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
                char net_sb;
                int channel_id;

                MHO_IntervalLabel fresh_ch_label;
                ch_label->Retrieve(std::string("sky_freq"), sky_freq);
                ch_label->Retrieve(std::string("bandwidth"), bw);
                ch_label->Retrieve(std::string("net_sideband"), net_sb);
                ch_label->Retrieve(std::string("channel"), channel_id);

                fresh_ch_label.Insert(std::string("sky_freq"), sky_freq);
                fresh_ch_label.Insert(std::string("bandwidth"), bw);
                fresh_ch_label.Insert(std::string("net_sideband"), net_sb);
                fresh_ch_label.Insert(std::string("channel"), channel_id);

                out_channel_axis->InsertLabel(fresh_ch_label);
            }
        }
        return true;
    }
    else
    {
        return false;
    }

}



}//end of namespace
