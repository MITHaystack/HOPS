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
MHO_WeightChannelizer::Initialize()
{
    fInitialized = false;
    if(this->fInput != nullptr && this->fOutput != nullptr)
    {
        //we want to re-organize the data in the input array (rank = 3)
        //into another array (with rank-4) with the data split up by channel
        std::size_t input_size = this->fInput->GetSize();
        if(input_size == 0){return false;}
        else
        {
            //now we need figure out the dimensions of the ouput array
            std::size_t input_dim[baseline_weight_type::rank::value];
            this->fInput->GetDimensions(input_dim);

            //and determine the number of unique channel labels
            auto* freq_axis = &(std::get<FREQ_AXIS>( *(this->fInput) ) );
            std::vector< MHO_IntervalLabel* > channel_labels = freq_axis->GetIntervalsWithKey(std::string("channel"));
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
            std::size_t output_dim[ch_baseline_weight_type::rank::value];
            output_dim[CH_POLPROD_AXIS] = input_dim[POLPROD_AXIS];
            output_dim[CH_CHANNEL_AXIS] = num_channels;
            output_dim[CH_TIME_AXIS] = input_dim[TIME_AXIS];
            output_dim[CH_FREQ_AXIS] = channel_length;

            this->fOutput->Resize(output_dim[0], output_dim[1], output_dim[2], output_dim[3]);

            auto* in_pp_axis = &(std::get<POLPROD_AXIS>( *(this->fInput) ) );
            auto* in_time_axis = &(std::get<TIME_AXIS>( *(this->fInput) ) );
            auto* in_freq_axis = &(std::get<FREQ_AXIS>( *(this->fInput) ) );

            auto* out_pp_axis = &(std::get<CH_POLPROD_AXIS>( *(this->fOutput) ) );
            auto* out_channel_axis = &(std::get<CH_CHANNEL_AXIS>( *(this->fOutput) ) );
            auto* out_time_axis = &(std::get<CH_TIME_AXIS>( *(this->fOutput) ) );
            auto* out_freq_axis = &(std::get<CH_FREQ_AXIS>( *(this->fOutput) ) );

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
MHO_WeightChannelizer::ExecuteOperation()
{
    if(fInitialized)
    {
        auto* in_pp_axis = &(std::get<POLPROD_AXIS>( *(this->fInput) ) );
        auto* in_time_axis = &(std::get<TIME_AXIS>( *(this->fInput) ) );
        auto* in_freq_axis = &(std::get<FREQ_AXIS>( *(this->fInput) ) );
        auto* out_channel_axis = &(std::get<CH_CHANNEL_AXIS>( *(this->fOutput) ) );


        //shorthand
        auto in = this->fInput;
        auto out = this->fOutput;

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
