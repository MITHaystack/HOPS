#ifndef MHO_JuliaPluginInterface_HH__
#define MHO_JuliaPluginInterface_HH__

#include "MHO_FringeFitter.hh"
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_JuliaOperatorBuilder.hh"


namespace hops
{

class MHO_JuliaPluginInterface: public MHO_FringeFitterVisitor
{
    public:
        MHO_JuliaPluginInterface();
        virtual ~MHO_JuliaPluginInterface();

        virtual void Visit(MHO_FringeFitter* fitter) override;

    protected:

        void Initialize();
        void Finalize();

        static bool fInitialized;

};

} // namespace hops

#endif /* end of include guard: MHO_JuliaPluginInterface_HH__ */
