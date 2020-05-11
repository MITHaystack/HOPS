#ifndef HStateStruct_HH__
#define HStateStruct_HH__

/*
*File: HStateStruct.hh
*Class: HStateStruct
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <string>

namespace hops
{

enum HStatusCode: int
{
    idle = 0,
    recording,
    pending,
    error,
    unknown
};

struct HStateStruct
{
    int status_code;
    std::string status_message;
};

}

#endif /* end of include guard: HStateStruct */
