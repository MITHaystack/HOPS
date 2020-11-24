#include "MHOChannelizedRotationFunctor.hh"


namespace hops
{

const std::complex<double> MHOChannelizedRotationFunctor::cI = std::complex<double>(0.0, 1.0);


MHOChannelizedRotationFunctor::MHOChannelizedRotationFunctor():
    fRefFrequency(0.0),
    fDelayRate(0.0),
    fSingleBandDelay(0.0),
    fTimeAxis(nullptr),
    fFreqAxis(nullptr)
{}

MHOChannelizedRotationFunctor::~MHOChannelizedRotationFunctor(){};

void
MHOChannelizedRotationFunctor::SetAxisPack(ch_baseline_axis_pack* axes)
{
    fTimeAxis = &(std::get<CH_TIME_AXIS>(*axes));
    fFreqAxis = &(std::get<CH_FREQ_AXIS>(*axes));
};

// void
// MHOChannelizedRotationFunctor::operator( input_iterator& input, output_iterator& output)
// {
//
// }


}//end of hops namespace
