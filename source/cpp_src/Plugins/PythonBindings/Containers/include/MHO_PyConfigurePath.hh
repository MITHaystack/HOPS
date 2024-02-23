#ifndef MHO_PyConfigurePath_HH__
#define MHO_PyConfigurePath_HH_

#include <string>
#include <sstream>

#include "MHO_Message.hh"

//pybind11 stuff to interface with python
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;


namespace hops
{

static void configure_pypath()
{
    if( Py_IsInitialized() == 0 )
    {
        //the internal python interpreter has not been started, bail out
        msg_warn("python_bindings", "python interpreter not running/initialized, cannot configure path " << eom);
    }
    else
    {
        //make sure our python plugin directories are in our search paths
        //only do this once on a per-executable level, since these settings are global
        //(e.g. we don't want each individual class messing with the search paths)
        std::stringstream pyss;
        pyss << "import sys\n";
        std::string default_path = STRING(HOPS_DEFAULT_PLUGINS_DIR);
        if(default_path.back() != '/'){default_path.push_back('/');}
        msg_info("main", "adding HOPS_DEFAULT_PLUGINS_DIR to search path: "<< default_path << eom);
        pyss << "sys.path.append(\"" << default_path <<  "\") \n";
        const char* user_plugin_env = std::getenv("HOPS_USER_PLUGINS_DIR");
        if(user_plugin_env != nullptr)
        {
            std::string user_specified_path = std::string(user_plugin_env);
            if(user_specified_path.back() != '/'){user_specified_path.push_back('/');}
            msg_info("main", "adding HOPS_USER_PLUGINS_DIR to search path: "<< user_specified_path << eom);
            pyss << "sys.path.append(\"" << user_specified_path <<  "\") \n";
        }
        py::exec( pyss.str().c_str() );
    }
}

}//end namespace


#endif /* end of include guard: MHO_PyConfigurePath_HH__ */
