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
        };
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
            //data goes out flat packed into 1-d array
            json data;
            for(auto it = container->cbegin(); it != container->cend(); it++)
            {
                InsertElement(*it, data);
            }
            fJSON["data"] = data;

            //std::cout<<fJSON.dump(2)<<std::endl; //dump the json to terminal
        };

        template< typename XValueType >
        void InsertElement(const XValueType& value, json& data)
        {
            data.push_back(value);
        }

        void InsertElement(const std::complex<double>& value, json& data)
        {
            data.push_back( {value.real(), value.imag()} );
        }

        void InsertElement(const std::complex<float>& value, json& data)
        {
            data.push_back( {value.real(), value.imag()} );
        }

        MHO_ExtensibleElement* fElement;
        XContainerType* fContainer;
        json fJSON;

};


}//end of hops namespace

#endif /* end of include guard: MHO_ContainerJSON */
