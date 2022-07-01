#ifndef MHO_MKJSONDateConverter_HH__
#define MHO_MKJSONDateConverter_HH__

//include the mk4 IO library
//The messy ifndef guards around the brackets are needed because the dfio library
//can optionally be compiled as either c or c++
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    #include "mk4_data.h"
    #include "mk4_dfio.h"

#ifndef HOPS3_USE_CXX
}
#endif

//include the json library stuff through the header wrapper 
//that lives inc the utilities library 
#include "MHO_JSONHeaderWrapper.hh"


namespace hops 
{

/**
*@file MHO_MK4JSONDateConverter.hh
*@author V. Pfeiffer - violetp@mit.edu
* A function that accepts the date struct and returns a JSON string with that data.
**/

/* convert struct to JSON string
* @param t pointer
* @return JSON string
*/

json convertDateToJSON(const date &t);

}

#endif /* end of include guard: MHO_MKJSONDateConverter_HH__ */
