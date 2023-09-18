#ifndef MHO_PyAxisHelpers_HH__
#define MHO_PyAxisHelpers_HH__

#include <complex>
#include <string>
#include <sstream>
#include <array>
#include <tuple>
#include <type_traits>

#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_TemplateTypenameDeduction.hh"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> //this is important to have for std::complex<T> support!
#include <pybind11/stl.h>
namespace py = pybind11;


/*
*File: MHO_PyTableContainer.hh
*Class: MHO_PyTableContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Fri Sep 15 10:03:38 PM EDT 2023
*Description: A set of helper classes/functions for access to the coordinate 
* axes of a table container to provide python access/interfacing
*/

namespace hops 
{

    template< typename XDumpType > 
    void DumpValuesToPyDict(MHO_SingleTypeMap< std::string , XDumpType >* map, py::dict& dump)
    {
        std::vector< std::string > keys;
        if(map != nullptr)
        {
            keys = map->DumpKeys();
            for(auto it = keys.begin(); it != keys.end(); it++)
            {
                XDumpType val;
                map->Retrieve(*it, val);
                dump[it->c_str()] = val;
            }
        }
    }

    template< typename XTableType >
    py::dict GetTableTags(XTableType* table)
    {
        py::dict tags;
        DumpValuesToPyDict<bool>(dynamic_cast< MHO_SingleTypeMap< std::string , bool >* >(table), tags);
        DumpValuesToPyDict<char>(dynamic_cast< MHO_SingleTypeMap< std::string , char >* >(table), tags);
        DumpValuesToPyDict<int>(dynamic_cast< MHO_SingleTypeMap< std::string , int >* >(table), tags);
        DumpValuesToPyDict<double>(dynamic_cast< MHO_SingleTypeMap< std::string , double >* >(table), tags);
        DumpValuesToPyDict<std::string>(dynamic_cast< MHO_SingleTypeMap< std::string , std::string >* >(table), tags);
        return tags;
    }

    py::dict GetIntervalLabelTags(MHO_IntervalLabel* label)
    {
        py::dict tags;
        DumpValuesToPyDict<bool>(dynamic_cast< MHO_SingleTypeMap< std::string , bool >* >(label), tags);
        DumpValuesToPyDict<char>(dynamic_cast< MHO_SingleTypeMap< std::string , char >* >(label), tags);
        DumpValuesToPyDict<int>(dynamic_cast< MHO_SingleTypeMap< std::string , int >* >(label), tags);
        DumpValuesToPyDict<double>(dynamic_cast< MHO_SingleTypeMap< std::string , double >* >(label), tags);
        DumpValuesToPyDict<std::string>(dynamic_cast< MHO_SingleTypeMap< std::string , std::string >* >(label), tags);
        return tags;
    }

    //helper class to act as a python-list filling functor (to return copies)
    class PyListFiller
    {
        public:
            PyListFiller(py::list* alist):fList(alist){};
            ~PyListFiller(){};

            template< typename XAxisType >
            void operator()(const XAxisType& axis)
            {
                for(size_t i=0; i<axis.GetSize(); i++){fList->append(axis[i]);}
            }

        private:
            py::list* fList;
    };

    
    //helper class to act as a python-list filling functor (to return copies)
    class PyIntervalLabelListFiller
    {
        public:
            PyIntervalLabelListFiller(py::list* alist):fList(alist){};
            ~PyIntervalLabelListFiller(){};
    
            template< typename XAxisType >
            void operator()(XAxisType& axis)
            {
                std::vector< MHO_IntervalLabel* > iLabels = axis.GetAllIntervalLabels();
                for(std::size_t i=0; i<iLabels.size(); i++)
                {
                    std::size_t low = iLabels[i]->GetLowerBound();
                    std::size_t up = iLabels[i]->GetUpperBound();
                    py::dict iLabelDict = GetIntervalLabelTags( iLabels[i] );
                    iLabelDict["lower_bound"] = low;
                    iLabelDict["uppper_bound"] = up;
                    fList->append(iLabelDict);
                }
            }
        private:
            py::list* fList;
    };

    //helper class that allows us to set the value of an axis label from python
    class PyAxisLabelModifier
    {
        public:

            //constructor accepts an coordinate index and a python object
            //and will attempt to case the object to the underyling axis-label type
            //and assign it at the location specified by the index
            PyAxisLabelModifier(std::size_t index, py::object* label_object):
                fIndex(index),
                fObject(label_object)
            {};

            ~PyAxisLabelModifier(){};

            template< typename XAxisType >
            void operator()(XAxisType& axis)
            {
                typename XAxisType::value_type label_value;
                label_value = fObject->cast< typename XAxisType::value_type >();

                //expect to get some sort of MHO_Axis
                if(fIndex < axis.GetSize() ){ axis(fIndex) = label_value; }
                else
                {
                    msg_fatal("python_bindings", "error axis coordinate index out of bounds: " << fIndex << " > " << axis.GetSize() << "."<< eom );
                    std::exit(1);
                }
            }

        private:
            std::size_t fIndex;
            py::object* fObject;

    };

}

#endif /* end of include guard: MHO_PyAxisHelpers_HH__ */
