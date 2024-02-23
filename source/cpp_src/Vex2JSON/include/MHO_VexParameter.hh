#ifndef MHO_VexParameter_HH__
#define MHO_VexParameter_HH__

/*!
*@file  MHO_VexParameter.hh
*@class  MHO_VexParameter
*@author  J. Barrett - barrettj@mit.edu 
*
*@date 
*@brief 
*/

#include <string>

namespace hops
{

class MHO_VexParameter
{
    public:

        MHO_VexParameter():
            fToken(""),
            fUnits("")
        {};

        MHO_VexParameter(std::string token, std::string units = ""):
            fToken(token),
            fUnits(units)
        {};

        MHO_VexParameter(std::string token, std::string units = ""):
            fToken(token),
            fUnits(units)
        {};

        virtual ~MHO_VexParameter();

        void SetTokenString(std::string token){fToken = token;}
        void SetUnitsString(std::string units){fUnits = unit;};

    private:

        std::string fToken;
        std::string fUnits;

};

}

#endif /*! end of include guard: MHO_VexParameter */