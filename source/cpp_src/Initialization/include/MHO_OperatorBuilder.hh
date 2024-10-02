#ifndef MHO_OperatorBuilder_HH__
#define MHO_OperatorBuilder_HH__

#include <string>
#include <utility>

#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_OperatorBuilder.hh
 *@class MHO_OperatorBuilder
 *@date Wed May 31 17:11:03 2023 -0400
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_OperatorBuilder
{

    public:
        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                            MHO_ParameterStore* pstore = nullptr)
            : fOperatorToolbox(toolbox), fContainerStore(cstore), fParameterStore(pstore){};

        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox

        virtual void SetToolbox(MHO_OperatorToolbox* toolbox) { fOperatorToolbox = toolbox; }

        virtual void SetParameterStore(MHO_ParameterStore* pstore) { fParameterStore = pstore; }

        virtual void SetContainerStore(MHO_ContainerStore* cstore) { fContainerStore = cstore; }

        //json config for this operator (parsed from the control file and format directives)
        virtual void SetFormat(const mho_json& format) { fFormat = format; } //operator format

        virtual void SetConditions(const mho_json& cond) { fConditions = cond; } //conditional statements

        virtual void SetAttributes(const mho_json& attr) { fAttributes = attr; } //configuration parameters

        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false and operator is not b
        virtual bool Build() = 0;

    protected:
        //provided for derived class to validate fAttributes against fFormat and/or fConditions
        //but the default tries to check a few things
        virtual bool IsConfigurationOk()
        {
            TODO_FIXME_MSG("TODO FIXME -- improve checks on operator attributes in IsConfigurationOk)")
            bool ok = true;
            //for compound statements, check that the fields are present
            if(fFormat["statement_type"] == "operator")
            {
                if(fFormat.contains("type") && fFormat["type"].get< std::string >() == "compound")
                {

                    for(auto it = fFormat["fields"].begin(); it != fFormat["fields"].end(); it++)
                    {
                        if(!fAttributes["value"].contains(*it))
                        {
                            msg_error("initialization", "missing attribute called " << *it << " in parameters of operator: "
                                                                                    << fFormat["name"].get< std::string >()
                                                                                    << ", will not build " << eom);
                            return false;
                        }
                    }
                }
            }

            return ok;
        }

        //constructed operator gets stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //data container and parameters stores
        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParameterStore;

        //provided for the configuration of the operator that is to be built
        mho_json fFormat;     //optional
        mho_json fConditions; //optional
        mho_json fAttributes; //required
};

} // namespace hops

#endif /*! end of include guard: MHO_OperatorBuilder_HH__ */
