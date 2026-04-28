#ifndef MHO_PythonOperatorBuilder_HH__
#define MHO_PythonOperatorBuilder_HH__

#include <memory>

#include "MHO_OperatorBuilder.hh"
#include "MHO_PyGenericOperator.hh"

namespace hops
{

/*!
 *@file MHO_PythonOperatorBuilder.hh
 *@class MHO_PythonOperatorBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Sep 18 17:22:44 2023 -0400
 *@brief
 */

class MHO_PythonOperatorBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_PythonOperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_PythonOperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_PythonOperatorBuilder(){};

        virtual bool Build() override
        {
            if(IsConfigurationOk())
            {
                std::unique_ptr< MHO_PyGenericOperator > op(new MHO_PyGenericOperator());

                //pass the data container and parameter stores to the python operator
                // op->SetParameterStore(this->fParameterStore);
                // op->SetContainerStore(this->fContainerStore);
                op->SetFringeData(this->fFringeData);
                op->SetOperatorToolbox(this->fOperatorToolbox);

                //retrieve pass the module/function name info from the control file
                std::string op_name = this->fFormat["name"].get< std::string >();
                std::string op_category = this->fFormat["operator_category"].get< std::string >();
                std::string module_path = fAttributes["value"]["module_path"].get< std::string >();
                std::string function_name = fAttributes["value"]["function_name"].get< std::string >();
                double priority = this->fFormat["priority"].get< double >();
                if(fAttributes["value"].contains("override_priority"))
                {
                    double override_priority = fAttributes["value"]["override_priority"].get< double >();
                    msg_debug("initialization", "python operator: " << op_name << " priority overridden from " << priority
                                                                    << " to " << override_priority << eom);
                    priority = override_priority;
                }

                op->SetPriority(priority);
                std::string full_name = module_path + ":" + function_name;
                op->SetName(full_name);
                op->SetModulePath(module_path);
                op->SetFunctionName(function_name);

                //TODO handle naming scheme for multiple python operators (should they have a name parameter?)
                bool replace_duplicates = false;
                this->fOperatorToolbox->AddOperator(std::move(op), full_name, op_category, replace_duplicates);
                return true;
            }
            else
            {
                msg_error("initialization", "cannot build python operator." << eom);
                return false;
            }
        }

        virtual bool IsConfigurationOk() override
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
                        //check if the field is optional
                        //for python operators, 'override_priority' is optional,
                        //and the default value is provided by the format
                        //if it is option, the field name will be preceeded by a '!'
                        //control file example:
                        //      python_calibration example4 set_pc_phase_offset_y 3.1

                        bool is_required = true;
                        std::string field_name = (*it).get< std::string >();
                        //std::cout<<"py op builder, field name = "<<field_name<<std::endl;
                        if(!field_name.empty() && field_name[0] == '!')
                        {
                            is_required = false;
                        }
                        if(!fAttributes["value"].contains(*it) && is_required)
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

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_PythonOperatorBuilder_HH__ */
