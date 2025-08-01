#ifndef MHO_VexParameter_HH__
#define MHO_VexParameter_HH__

#include <string>

namespace hops
{

/*!
 *@file  MHO_VexParameter.hh
 *@class  MHO_VexParameter
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu May 26 16:55:16 2022 -0400
 *@brief Stores a vex parameter value token (with associated units)
 */

/**
 * @brief Class MHO_VexParameter
 */
class MHO_VexParameter
{
    public:
        MHO_VexParameter(): fToken(""), fUnits(""){};

        MHO_VexParameter(std::string token, std::string units = ""): fToken(token), fUnits(units){};

        MHO_VexParameter(std::string token, std::string units = ""): fToken(token), fUnits(units){};

        virtual ~MHO_VexParameter();

        /**
         * @brief Setter for token string
         *
         * @param token Input token string to set
         */
        void SetTokenString(std::string token) { fToken = token; }

        /**
         * @brief Setter for units string
         *
         * @param units New units string to set
         */
        void SetUnitsString(std::string units) { fUnits = unit; };

    private:
        std::string fToken;
        std::string fUnits;
};

} // namespace hops

#endif /*! end of include guard: MHO_VexParameter */
