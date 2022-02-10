#ifndef MHO_ContainerJSON_HH__
#define MHO_ContainerJSON_HH__

/*
*File: MHO_ContainerJSON.hh
*Class: MHO_ContainerJSON
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Converts a given ndarray into a JSON representation 
* this isn't really intended for data transport/storage, but only as
* conversion to an ascii-like representation for human inspection
*/

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ExtensibleElement.hh"

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_TableContainer.hh"

#include "MHO_ContainerDictionary.hh"



namespace hops
{

template< typename XContainerType > 
class MHO_ContainerJSON
{
    public:

        MHO_ContainerJSON(MHO_ExtensibleElement* element)
        {
            fContainer = dynamic_cast< XContainerType* >(element);
            ConstructJSONRepresentation(fContainer);
        }

        virtual ~MHO_ContainerJSON(){}

        json* GetJSON(){return &fJSON;}

    protected:

        void ConstructJSONRepresentation(XContainerType* container)
        {
            MHO_ContainerDictionary cdict;
            std::string class_name = cdict.GetClassNameFromObject(*container);
            std::string class_uuid = (cdict.GetUUIDFromClassName(class_name)).as_string();

            //container must inherit from MHO_NDArrayWrapper
            fJSON["class_name"] = class_name;
            fJSON["class_uuid"] = class_uuid;
            fJSON["rank"] = container->GetRank();
            fJSON["total_size"] = container->GetSize();
            json dim_array = container->GetDimensionArray();
            json stride_array = container->GetStrideArray();
            fJSON["dimensions"] = dim_array;
            fJSON["strides"] = stride_array;
            //data goes out flat-packed into 1-d array
            json data;
            for(auto it = container->cbegin(); it != container->cend(); it++)
            {
                InsertElement(*it, data);
            }
            fJSON["data"] = data;

            IfTableDumpAxes(container, &fJSON);
        };

        //generic data insertion
        template< typename XValueType >
        void InsertElement(const XValueType& value, json& data)
        {
            data.push_back(value);
        }

        //complex<> element data insertion
        void InsertElement(const std::complex<double>& value, json& data)
        {
            data.push_back( {value.real(), value.imag()} );
        }

        void InsertElement(const std::complex<float>& value, json& data)
        {
            data.push_back( {value.real(), value.imag()} );
        }


        ////////////////////////////////////////////////////////////////////////
        //SFINAE specializations for various container types 
        ////////////////////////////////////////////////////////////////////////

        //default...does nothing
        template< typename XCheckType = XContainerType >
        typename std::enable_if< !std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableDumpAxes(const XContainerType* /*in*/, json* /*out*/){};

        //use SFINAE to generate specialization for MHO_TableContainer types
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of<MHO_TableContainerBase, XCheckType>::value, void >::type
        IfTableDumpAxes(const XContainerType* in, json* out)
        {
            AxisDumper axis_dumper(out);
            for(std::size_t idx=0; idx < in->GetRank(); idx++)
            {
                axis_dumper.SetIndex(idx);
                apply_at< typename XContainerType::axis_pack_tuple_type, AxisDumper>(*in, idx, axis_dumper);
            }
        }

        class AxisDumper
        {
            public:
                AxisDumper(json* json_ptr):
                    fAxisJSON(json_ptr),
                    fIndex(0)
                {};
                ~AxisDumper(){};

                void SetIndex(std::size_t idx){fIndex = idx;}

                template< typename XAxisType >
                void operator()(const XAxisType& axis)
                {
                    json j;
                    j["total_size"] = axis.GetSize();
                    //data goes out flat-packed into 1-d array
                    json data;
                    for(auto it = axis.cbegin(); it != axis.cend(); it++)
                    {
                        data.push_back(*it);
                    }
                    j["data"] = data;
                    std::stringstream ss;
                    ss << "axis_" << fIndex;
                    (*fAxisJSON)[ss.str().c_str()] = j;

                    //TODO FIXME --- we need to dump the axis labels too!
                }

            private:

                json* fAxisJSON;
                std::size_t fIndex;
        };

    private:
        XContainerType* fContainer;
        json fJSON;

};



}//end of hops namespace

#endif /* end of include guard: MHO_ContainerJSON */
