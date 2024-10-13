#ifndef MHO_PyUnaryTableOperator_HH__
#define MHO_PyUnaryTableOperator_HH__

#include "MHO_PyTableContainer.hh"
#include "MHO_UnaryOperator.hh"

#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace hops
{

/*!
 *@file  MHO_PyUnaryTableOperator.hh
 *@class  MHO_PyUnaryTableOperator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Sep 23 16:03:48 2021 -0400
 *@brief
 */

class MHO_PyUnaryTableOperator: public MHO_Operator
{
    public:
        MHO_PyUnaryTableOperator(): fHelper(nullptr), fInitialized(false){};

        virtual ~MHO_PyUnaryTableOperator() { delete fHelper; };

        template< typename XTableType > void SetInput(XTableType* in)
        {
            if(fHelper != nullptr)
            {
                delete fHelper;
                fHelper = nullptr;
            }
            fHelper = new helper_specific< XTableType >(in);
        };

        void SetModuleFunctionName(std::string module_name, std::string function_name)
        {
            fModuleName = module_name;
            fFunctionName = function_name;
        }

        //for now this does nothing, as the interpreter lifetime is confined to
        //the Execute function, we may want to re-think this.
        virtual bool Initialize() override
        {
            if(fHelper != nullptr)
            {
                fInitialized = true;
            }
            return fInitialized;
        };

        virtual bool Execute() override
        {
            if(fInitialized)
            {
                fHelper->SetModuleName(fModuleName);
                fHelper->SetFunctionName(fFunctionName);
                fHelper->exe();
            }
            return false;
        }

    private:
        class helper_base
        {
            public:
                helper_base(){};
                virtual ~helper_base(){};

                void SetModuleName(std::string mod) { fModuleName = mod; }

                void SetFunctionName(std::string func) { fFunctionName = func; };

                virtual bool exe() = 0;

            protected:
                std::string fModuleName;
                std::string fFunctionName;
        };

        template< typename XTableType > class helper_specific: public helper_base
        {
            public:
                helper_specific(XTableType* ptr): fPtr(ptr){};
                virtual ~helper_specific(){};

                virtual bool exe() override
                {
                    if(!(fPtr->template HasExtension< MHO_PyTableContainer< XTableType > >()))
                    {
                        fPtr->template MakeExtension< MHO_PyTableContainer< XTableType > >();
                    }
                    //assume the python interpreter is already running (should we use try/catch?)
                    auto mod = py::module::import(fModuleName.c_str());
                    auto extension = fPtr->template AsExtension< MHO_PyTableContainer< XTableType > >();
                    MHO_PyTableContainer< XTableType >* container =
                        dynamic_cast< MHO_PyTableContainer< XTableType >* >(extension);
                    if(container != nullptr)
                    {
                        mod.attr(fFunctionName.c_str())(*container);
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

            private:
                XTableType* fPtr;
        };

        helper_base* fHelper;

        bool fInitialized;
        std::string fModuleName;
        std::string fFunctionName;
};

} // namespace hops

#endif /*! end of include guard: MHO_PyUnaryTableOperator */
