#ifndef MHO_PythonPluginInterface_HH__
#define MHO_PythonPluginInterface_HH__

#include "MHO_FringeFitter.hh"
#include "MHO_OperatorBuilderManager.hh"

#include "MHO_PythonOperatorBuilder.hh"

namespace hops
{

/*
 * @brief Class MHO_PythonPluginInterface
 */

class MHO_PythonPluginInterface: public MHO_FringeFitterVisitor
{

    private:
        MHO_PythonPluginInterface();
        virtual ~MHO_PythonPluginInterface();

        static MHO_PythonPluginInterface* fPythonPluginInterface;


    public:

        //singleton interface
        /**
         * @brief Getter for instance
         *
         * @return MHO_PythonPluginInterface* singleton instance
         * @note This is a static function.
         */
        static MHO_PythonPluginInterface* GetInstance();

        virtual void Visit(MHO_FringeFitter* fitter) override;

    protected:

};

} // namespace hops

#endif /* end of include guard: MHO_PythonPluginInterface_HH__ */
