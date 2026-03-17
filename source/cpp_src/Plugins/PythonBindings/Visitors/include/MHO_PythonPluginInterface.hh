#ifndef MHO_PythonPluginInterface_HH__
#define MHO_PythonPluginInterface_HH__

//pybind11 stuff to interface with python
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
#include "MHO_DefaultPythonPlotVisitor.hh"
#include "MHO_PyConfigurePath.hh"
#include "MHO_PyFringeDataInterface.hh"
#include "MHO_PythonOperatorBuilder.hh"


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
    public:
        MHO_PythonPluginInterface();
        virtual ~MHO_PythonPluginInterface();

        virtual void Visit(MHO_FringeFitter* fitter) override;

    protected:

        void Initialize();
        void Finalize();

        static bool fInitialized;

};

} // namespace hops

#endif /* end of include guard: MHO_PythonPluginInterface_HH__ */
