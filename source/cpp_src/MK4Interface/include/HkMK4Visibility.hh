#ifndef HkMK4Visibility_HH__
#define HkMK4Visibility_HH__

/*
*File: HkMK4Visibility.hh
*Class: HkMK4Visibility
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 17-08-2020 11:27
*Description: visibility type for testing purposes
*/

#include <string>
#include <complex>

#include "HkVectorContainer.hh"
#include "HkTensorContainer.hh"


namespace hops
{

using frequency_axis_type = HkVectorContainer<double>;
using time_axis_type = HkVectorContainer<double>;
using channel_axis_type =  HkVectorContainer< std::string >;

using MK4VisibilityAxes = HkAxisPack< channel_axis_type, frequency_axis_type, time_axis_type>;
using visibility_type = std::complex<double>;
using MK4Visibility = HkTensorContainer< visibility_type, MK4VisibilityAxes::rank::value, MK4VisibilityAxes>;

}

#endif /* end of include guard: HkMK4Visibility */
