#include "MHO_ChannelizedRotationFunctor.hh"


namespace hops
{

const std::complex<double> MHO_ChannelizedRotationFunctor::cI = std::complex<double>(0.0, 1.0);


MHO_ChannelizedRotationFunctor::MHO_ChannelizedRotationFunctor():
    fRefFrequency(0.0),
    fDelayRate(0.0),
    fSingleBandDelay(0.0),
    fTimeAxis(nullptr),
    fFreqAxis(nullptr)
{}

MHO_ChannelizedRotationFunctor::~MHO_ChannelizedRotationFunctor(){};

void
MHO_ChannelizedRotationFunctor::SetAxisPack(ch_baseline_axis_pack* axes)
{
    fTimeAxis = &(std::get<CH_TIME_AXIS>(*axes));
    fFreqAxis = &(std::get<CH_FREQ_AXIS>(*axes));
};

// void
// MHO_ChannelizedRotationFunctor::operator( input_iterator& input, output_iterator& output)
// {
//
// }


}//end of hops namespace
