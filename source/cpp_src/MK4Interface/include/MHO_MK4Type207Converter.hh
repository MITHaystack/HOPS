#ifndef MHO_MKType207Converter_HH__
#define MHO_MKType207Converter_HH__

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

//include the mho_json library stuff through the header wrapper
//that lives inc the utilities library
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{
/*!*
>>>>>>> 2244689b19d877f8c1684530a28c9cba07f7d6dd
*@file MHO_MK4Type207Converter.hh
*@author V. Pfeiffer - violetp@mit.edu
* A function that accepts the data from a type 207 file in the form of a struct and converts it to a JSON object.
**/

/*! convert struct to JSON string
 * @param t pointer
 * @return JSON string
 */

mho_json convertToJSON(const type_207& t);

} // namespace hops

#endif /*! end of include guard: MHO_MKType207Converter */
