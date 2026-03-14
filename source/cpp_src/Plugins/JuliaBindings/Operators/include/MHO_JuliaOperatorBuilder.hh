#ifndef MHO_JuliaOperatorBuilder_HH__
#define MHO_JuliaOperatorBuilder_HH__

#include "MHO_JlGenericOperator.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_JuliaOperatorBuilder.hh
 *@class MHO_JuliaOperatorBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@brief Constructs MHO_JlGenericOperator instances from control-file config.
 *
 * The control file entry must supply:
 *   "module_path":   string identifier (used for operator naming; informational only)
 *   "function_name": string identifier (used for operator naming; informational only)
 *
 * The actual Julia function pointer must be set via
 *   MHO_JlGenericOperator::SetJuliaFunction() before Initialize() is called.
 * This builder only creates the operator and registers it in the toolbox.
 */

class MHO_JuliaOperatorBuilder : public MHO_OperatorBuilder
{
    public:
        MHO_JuliaOperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_JuliaOperatorBuilder(MHO_OperatorToolbox* toolbox,
                                  MHO_ContainerStore* cstore = nullptr,
                                  MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_JuliaOperatorBuilder(){};

        virtual bool Build() override
        {
            if(!IsConfigurationOk())
            {
                msg_error("initialization", "cannot build Julia operator." << eom);
                return false;
            }

            auto* op = new MHO_JlGenericOperator();
            op->SetFringeData(this->fFringeData);

            std::string op_name      = this->fFormat["name"].get< std::string >();
            std::string op_category  = this->fFormat["operator_category"].get< std::string >();
            std::string module_path  = fAttributes["value"]["module_path"].get< std::string >();
            std::string func_name    = fAttributes["value"]["function_name"].get< std::string >();
            double priority          = this->fFormat["priority"].get< double >();

            op->SetPriority(priority);
            op->SetModulePath(module_path);
            op->SetFunctionName(func_name);
            op->SetName(module_path + ":" + func_name);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
            return true;
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_JuliaOperatorBuilder_HH__ */
