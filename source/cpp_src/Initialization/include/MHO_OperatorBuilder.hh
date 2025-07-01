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
#include "MHO_FringeData.hh"

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

        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : fOperatorToolbox(toolbox), 
              fFringeData(fdata),
              fContainerStore(fdata->GetContainerStore()), 
              fParameterStore(fdata->GetParameterStore()){};


        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox, 
                            MHO_ContainerStore* cstore = nullptr,
                            MHO_ParameterStore* pstore = nullptr)
            : fOperatorToolbox(toolbox), 
              fFringeData(nullptr), //ptr to fringe data object not provided
              fContainerStore(cstore),
              fParameterStore(pstore){};

        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox

        /**
         * @brief Setter for toolbox
         * 
         * @param toolbox Pointer to the MHO_OperatorToolbox object to be set.
         * @note This is a virtual function.
         */
        virtual void SetToolbox(MHO_OperatorToolbox* toolbox) { fOperatorToolbox = toolbox; }

        /**
         * @brief Setter for fringe data
         * 
         * @param fdata Pointer to MHO_FringeData structure
         * @note This is a virtual function.
         */
        virtual void SetFringeData(MHO_FringeData* fdata) { fFringeData = fdata; }

        /**
         * @brief Setter for parameter store
         * 
         * @param pstore Pointer to MHO_ParameterStore object
         * @note This is a virtual function.
         */
        virtual void SetParameterStore(MHO_ParameterStore* pstore) { fParameterStore = pstore; }

        /**
         * @brief Setter for container store
         * 
         * @param cstore Pointer to MHO_ContainerStore object
         * @note This is a virtual function.
         */
        virtual void SetContainerStore(MHO_ContainerStore* cstore) { fContainerStore = cstore; }

        //json config for this operator (parsed from the control file and format directives)
        /**
         * @brief Setter for format
         * 
         * @param format The new format for the operator.
         * @note This is a virtual function.
         */
        virtual void SetFormat(const mho_json& format) { fFormat = format; } //operator format

        /**
         * @brief Setter for conditions
         * 
         * @param cond Input conditions of type const mho_json&
         * @note This is a virtual function.
         */
        virtual void SetConditions(const mho_json& cond) { fConditions = cond; } //conditional statements

        /**
         * @brief Setter for attributes
         * 
         * @param attr Input attribute data of type const mho_json&
         * @note This is a virtual function.
         */
        virtual void SetAttributes(const mho_json& attr) { fAttributes = attr; } //configuration parameters

        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false and operator is not b
        /**
         * @brief Builds the object and passes it to toolbox if successful, otherwise returns false.
         * 
         * @return bool indicating success or failure
         * @note This is a virtual function.
         */
        virtual bool Build() = 0;

    protected:
        //provided for derived class to validate fAttributes against fFormat and/or fConditions
        //but the default tries to check a few things
        /**
         * @brief Function IsConfigurationOk
         * 
         * @return Return value (bool)
         * @note This is a virtual function.
         */
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

        //fringe data object (holds both container/parameter store, along with plot data)
        MHO_FringeData* fFringeData;

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
