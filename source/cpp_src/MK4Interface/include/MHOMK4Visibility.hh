#ifndef MHOMK4Visibility_HH__
#define MHOMK4Visibility_HH__

/*
*File: MHOMK4Visibility.hh
*Class: MHOMK4Visibility
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 17-08-2020 11:27
*Description: visibility type for TESTING purposes
*/

#include <string>
#include <complex>

#include "MHOVectorContainer.hh"
#include "MHOTensorContainer.hh"


namespace hops
{

using visibility_type = std::complex<double>;

using frequency_axis_type = MHOVectorContainer<double>;
using time_axis_type = MHOVectorContainer<double>;

using channel_axis_pack = MHOAxisPack< time_axis_type, frequency_axis_type>;
using channel_data_type = MHOTensorContainer< visibility_type, channel_axis_pack >;


//using channel_axis_type =  MHOVectorContainer< std::string >;
//using MK4VisibilityAxes = MHOAxisPack< channel_axis_type, frequency_axis_type, time_axis_type>;
//
//using MK4Visibility = MHOTensorContainer< visibility_type, MK4VisibilityAxes::rank::value, MK4VisibilityAxes>;

}

#endif /* end of include guard: MHOMK4Visibility */
