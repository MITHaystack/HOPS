#ifndef MHO_PythonOperatorBuilderBuilder_HH__
#define MHO_PythonOperatorBuilderBuilder_HH__

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
                MHO_PyGenericOperator* op = new MHO_PyGenericOperator();

                //pass the data container and parameter stores to the python operator
                // op->SetParameterStore(this->fParameterStore);
                // op->SetContainerStore(this->fContainerStore);
                op->SetFringeData(this->fFringeData);

                //retrieve pass the module/function name info from the control file
                std::string op_name = this->fFormat["name"].get< std::string >();
                std::string op_category = this->fFormat["operator_category"].get< std::string >();
                std::string module_name = fAttributes["value"]["module_name"].get< std::string >();
                std::string function_name = fAttributes["value"]["function_name"].get< std::string >();
                double priority = this->fFormat["priority"].get< double >();

                op->SetPriority(priority);
                op->SetName(module_name + ":" + function_name);
                op->SetModuleName(module_name);
                op->SetFunctionName(function_name);

                //TODO handle naming scheme for multiple python operators (should they have a name parameter?)
                bool replace_duplicates = false;
                this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
                return true;
            }
            else
            {
                msg_error("initialization", "cannot build python operator." << eom);
                return false;
            }
        }

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_PythonOperatorBuilderBuilder_HH__ */
