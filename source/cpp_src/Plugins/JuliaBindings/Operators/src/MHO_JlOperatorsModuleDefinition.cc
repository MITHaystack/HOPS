#include "jlcxx/jlcxx.hpp"

#include "MHO_JlGenericOperator.hh"
#include "MHO_JlOperator.hh"
#include "MHO_Operator.hh"

using namespace hops;

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
    // ------------------------------------------------------------------
    // MHO_Operator base (abstract; not directly constructable from Julia)
    // ------------------------------------------------------------------
    mod.add_type< MHO_Operator >("MHO_Operator")
        .method("initialize", &MHO_Operator::Initialize)
        .method("execute",    &MHO_Operator::Execute);

    // ------------------------------------------------------------------
    // MHO_JlOperator: concrete operator with Julia callbacks
    // Subtype of MHO_Operator in Julia.
    // ------------------------------------------------------------------
    mod.add_type< MHO_JlOperator >("JlOperator", jlcxx::julia_base_type< MHO_Operator >())
        .constructor<>()
        .method("set_initialize_fn", &hops::MHO_JlOperator::SetInitializeFunction)
        .method("set_execute_fn",    &hops::MHO_JlOperator::SetExecuteFunction)
        .method("initialize",        &hops::MHO_JlOperator::Initialize)
        .method("execute",           &hops::MHO_JlOperator::Execute);

    // ------------------------------------------------------------------
    // MHO_JlGenericOperator: calls a Julia function(fd) per Execute().
    // The fringe data interface is passed directly as the first argument.
    // ------------------------------------------------------------------
    mod.add_type< MHO_JlGenericOperator >("JlGenericOperator",
                                           jlcxx::julia_base_type< MHO_Operator >())
        .constructor<>()
        .method("set_julia_function",  &hops::MHO_JlGenericOperator::SetJuliaFunction)
        .method("set_module_path",     &hops::MHO_JlGenericOperator::SetModulePath)
        .method("set_function_name",   &hops::MHO_JlGenericOperator::SetFunctionName)
        .method("get_module_path",     &hops::MHO_JlGenericOperator::GetModulePath)
        .method("get_function_name",   &hops::MHO_JlGenericOperator::GetFunctionName)
        .method("initialize",          &hops::MHO_JlGenericOperator::Initialize)
        .method("execute",             &hops::MHO_JlGenericOperator::Execute);

}
