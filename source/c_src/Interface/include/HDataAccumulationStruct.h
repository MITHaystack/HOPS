#ifndef HDataAccumulationStruct_H__
#define HDataAccumulationStruct_H__

#include <stdint.h>
#include <stdlib.h>

/*
*File: HDataAccumulationStruct.h
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

//values for the state_flag element
//to indicate the state of the noise diode
//it might be possible this could be expanded upon to handle other situations (vane cal?)
#define H_NOISE_DIODE_OFF 0
#define H_NOISE_DIODE_ON 1

struct HDataAccumulationStruct
{
    double sum_x; // of x for each sample value x during the accumulation period
    double sum_x2; //sum of the x*x for each sample value x during the accumulation period
    double count; //should just be stop_index - start_index, but stored here for convenience
    uint64_t state_flag; //indicates the data state (diode on/off, etc)
    uint64_t start_index; //start index of the first sample in the accumulation period
    uint64_t stop_index; //stop index of the last sample in the accumulation period
};

#endif /* end of include guard: HDataAccumulationStruct */
