#include "MHOVisibilityChannelizer.hh"


namespace hops
{

MHOVisibilityChannelizer::MHOVisibilityChannelizer()
{
    fInitialized = false;
}

MHOVisibilityChannelizer::~MHOVisibilityChannelizer(){}

bool
MHOVisibilityChannelizer::Initialize()
{

    fInitialized = false;
    if(this->fInput != nullptr && this->fOutput != nullptr)
    {

        std::cout<<"initializing channelizer"<<std::endl;
        //we want to re-organize the data in the input array (rank = 3)
        //into another array (with rank-4) with the data split up by channel
        std::size_t input_size = this->fInput->GetSize();
        if(input_size == 0){return false;}
        else
        {
            //now we need figure out the dimensions of the ouput array
            std::size_t input_dim[baseline_data_type::rank::value];
            this->fInput->GetDimensions(input_dim);

            //and determine the number of unique channel labels
            auto* freq_axis = &(std::get<FREQ_AXIS>( *(this->fInput) ) );
            std::vector< MHOIntervalLabel* > channel_labels = freq_axis->GetIntervalsWithKey(std::string("channel"));
            std::size_t num_channels = channel_labels.size();

            //then we need to make sure all channels have the same size
            std::set<std::size_t> channel_sizes;
            for(auto iter = channel_labels.begin(); iter != channel_labels.end(); iter++)
            {
                std::cout<<"inserting channel of size: "<< (*iter)->GetLength()<<std::endl;
                channel_sizes.insert( (*iter)->GetLength() );
            }

            if(channel_sizes.size() != 1)
            {
                std::cout<<"Error channel sizes are not the same!"<<std::endl;
            }
            std::size_t channel_length = *( channel_sizes.begin() );

            //finally we can re-size the output array so that it is ready
            //to recieve data from each channel
            std::size_t output_dim[ch_baseline_data_type::rank::value];
            output_dim[CH_POLPROD_AXIS] = input_dim[POLPROD_AXIS];
            output_dim[CH_CHANNEL_AXIS] = num_channels;
            output_dim[CH_TIME_AXIS] = input_dim[TIME_AXIS];
            output_dim[CH_FREQ_AXIS] = channel_length;

            std::cout<<"calling resize"<<std::endl;
            // this->fOutput->Resize(output_dim);
            this->fOutput->Resize(output_dim[0], output_dim[1], output_dim[2], output_dim[3]);


            return true;
        }
    }
    return false;
}

bool
MHOVisibilityChannelizer::ExecuteOperation()
{
    //we are going to loop over
    return false;
}



}
