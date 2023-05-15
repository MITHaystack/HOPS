#ifndef MHO_ControlTokenProcessor_HH__
#define MHO_ControlTokenProcessor_HH__

/*
*@file: MHO_ControlTokenProcessor.hh
*@class: MHO_ControlTokenProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "MHO_Message.hh"
#include "MHO_JSONHeaderWrapper.hh"

#include <cstdlib>
#include <vector>
#include <string>

namespace hops
{

class MHO_ControlTokenProcessor
{
    public:
        MHO_ControlTokenProcessor();
        virtual ~MHO_ControlTokenProcessor();

        mho_json ProcessInt(const std::string& element_name, const std::string& token);
        mho_json ProcessString(const std::string& element_name, const std::string& token);
        mho_json ProcessReal(const std::string& element_name, const std::string& token);
        mho_json ProcessListInt(const std::string& element_name, const std::vector< std::string >& tokens);
        mho_json ProcessListString(const std::string& element_name, const std::vector< std::string >& tokens);
        mho_json ProcessListReal(const std::string& element_name, const std::vector< std::string >& tokens);

    private:
};

}

#endif /* end of include guard: MHO_ControlTokenProcessor */
