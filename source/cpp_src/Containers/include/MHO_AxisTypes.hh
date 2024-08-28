#ifndef MHO_AxisTypes_HH__
#define MHO_AxisTypes_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_AxisTypes.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif

namespace hops 
{

//declare all of the different types of coordinate axes that are available in order 
//to construct table container types for baseline and station quantities

using pol_axis_type = MHO_Axis<std::string>; //axis for polarization labels (X, Y, R, L, etc.)
using polprod_axis_type = MHO_Axis<std::string>; //axis for polarization-product labels (XX, YX, XR, RL, RR, etc.)
using channel_axis_type = MHO_Axis<double>; //channels axis is sky_frequency of each channel (MHz)
using frequency_axis_type = MHO_Axis<double>; //frequency axis (MHz)
using time_axis_type = MHO_Axis<double>; //time and/or AP axis
using delay_rate_axis_type = MHO_Axis<double>; //delay rate axis

//station coordinate specific quantities
using coord_axis_type = MHO_Axis<std::string>; //coordinate name (delay, phase, parallatic_angle, az, el, u, v, w)
using coeff_axis_type = MHO_Axis<int>; //spline coefficient index 0,1,2...6(max)

}

#endif /* end of include guard: MHO_AxisTypes */