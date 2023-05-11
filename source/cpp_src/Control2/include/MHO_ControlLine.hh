#ifndef MHO_ControlLine_HH__
#define MHO_ControlLine_HH__

/*
*@file: MHO_ControlLine.hh
*@class: MHO_ControlLine
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>

namespace hops
{

struct MHO_ControlLine
{
    std::size_t fLineNumber;
    std::size_t fStatementNumber;
    std::string fContents;
    bool fIsLiteral;
};

}

#endif /* end of include guard: MHO_ControlLine */