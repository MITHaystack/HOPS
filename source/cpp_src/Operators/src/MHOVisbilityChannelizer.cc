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
        //we want to re-organize the data in the input array (rank = 3)
        //into another array (with rank-4) with the data split up by channel
        std::size_t input_size = this->fInput->GetSize();
        if(input_size == 0){return false;}
        else 
        {
            //now we need figure out the dimensions of the ouput array 
            std::size_t input_dim[baseline_data_type::rank::value]
            this->fInput->GetDimensions(input_dim);
            
            //and determine the number of unique channel labels

            //then we need to make sure all channels have the same size 

            //finally we can re-size the output array so that it is ready
            //to recieve data from each channel

            std::size_t input_dim[ch_baseline_data_type::rank::value]


        }



    }
}

bool 
MHOVisibilityChannelizer::ExecuteOperation()
{
    //we are going to loop over 
}



}