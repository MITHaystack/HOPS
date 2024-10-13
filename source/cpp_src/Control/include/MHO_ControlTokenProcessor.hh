#ifndef MHO_ControlTokenProcessor_HH__
#define MHO_ControlTokenProcessor_HH__

#include "MHO_ControlDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"

#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file  MHO_ControlTokenProcessor.hh
 *@class  MHO_ControlTokenProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jun 13 22:27:21 2022 -0400
 *@brief
 */

class MHO_ControlTokenProcessor
{
    public:
        MHO_ControlTokenProcessor();
        virtual ~MHO_ControlTokenProcessor();

        mho_json ProcessInt(const MHO_Token& token);
        mho_json ProcessString(const MHO_Token& token);
        mho_json ProcessReal(const MHO_Token& token);
        mho_json ProcessListInt(const std::vector< MHO_Token >& tokens);
        mho_json ProcessListString(const std::vector< MHO_Token >& tokens);
        mho_json ProcessFixedLengthListString(const std::vector< MHO_Token >& tokens);
        mho_json ProcessListReal(const std::vector< MHO_Token >& tokens);
        mho_json ProcessBool(const MHO_Token& token);

    private:
        bool ConvertFloat(const MHO_Token& token, double& val);
        bool ConvertInteger(const MHO_Token& token, int& val);
        bool ConvertBool(const MHO_Token& token, bool& val);
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlTokenProcessor */
