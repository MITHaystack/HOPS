#ifndef MHO_VexLine_HH__
#define MHO_VexLine_HH__

#include <string>

namespace hops
{

/*!
*@file  MHO_VexLine.hh
*@class  MHO_VexLine
*@author  J. Barrett - barrettj@mit.edu
*@date Fri May 27 12:45:21 2022 -0400
*@brief
*/


struct MHO_VexLine
{
    std::size_t fLineNumber;
    std::size_t fStatementNumber;
    std::string fContents;
    bool fIsLiteral;
};

}

#endif /*! end of include guard: MHO_VexLine */
