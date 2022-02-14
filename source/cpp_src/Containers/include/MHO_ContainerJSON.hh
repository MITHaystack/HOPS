#ifndef MHO_ContainerJSON_HH__
#define MHO_ContainerJSON_HH__

/*
*File: MHO_ContainerJSON.hh
*Class: MHO_ContainerJSON
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Converts a given ndarray-based container into a JSON representation 
* this isn't really intended for data transport/storage, but only as
* conversion to an ascii-like representation for human inspection/debugging
*/

#include "MHO_ClassIdentity.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ExtensibleElement.hh"

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_VectorContainer.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

enum
MHO_ContainerJSONLevel: int
{
    eJSONBasicLevel = 0, //basic quantities (name, rank, dimensions, etc.)
    eJSONAxesLevel = 1, //basic quantities plus the axes
    eJSONAxesWithLabelsLevel = 2, //basic quantities plus axes with interval labels
    eJSONAllLevel = 3 //everything including the main data array
};

//short hand aliases
static const MHO_ContainerJSONLevel eJSONBasic = MHO_ContainerJSONLevel::eJSONBasicLevel;
static const MHO_ContainerJSONLevel eJSONWithAxes = MHO_ContainerJSONLevel::eJSONAxesLevel;
static const MHO_ContainerJSONLevel eJSONWithLabels = MHO_ContainerJSONLevel::eJSONAxesWithLabelsLevel;
static const MHO_ContainerJSONLevel eJSONAll = MHO_ContainerJSONLevel::eJSONAllLevel;


using hops::eJSONBasic;
using hops::eJSONWithAxes;
using hops::eJSONWithLabels;
using hops::eJSONAll;

template< typename XContainerType > 
class MHO_ContainerJSON
{
    public:

        MHO_ContainerJSON(MHO_ExtensibleElement* element)
        {
            fLOD = 0;
            fContainer = dynamic_cast< XContainerType* >(element);
        }

        virtual ~MHO_ContainerJSON(){}

        void SetLevelOfDetail(int level){fLOD = level;};

        json* GetJSON(){return &fJSON;}

        void ConstructJSONRepresentation()
        {
            std::string class_name = MHO_ClassIdentity::ClassName<XContainerType>();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass<XContainerType>().as_string();
            //container must inherit from MHO_NDArrayWrapper
            if(fLOD >= eJSONBasic)
            {
                fJSON["class_name"] =  class_name;
                fJSON["class_uuid"] = class_uuid;
                fJSON["name"] = fContainer->GetName();
                fJSON["units"] = fContainer->GetUnits();
                fJSON["rank"] = fContainer->GetRank();
                fJSON["total_size"] = fContainer->GetSize();
                json dim_array = fContainer->GetDimensionArray();
                json stride_array = fContainer->GetStrideArray();
                fJSON["dimensions"] = dim_array;
                fJSON["strides"] = stride_array;
            }

            //data goes out flat-packed into 1-d array
            if(fLOD >= eJSONAll)
            {
                json data;
                for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
                {
                    InsertElement(*it, data);
                }
                fJSON["data"] = data;
            }

            if(fLOD >= eJSONWithAxes)
            {
                IfTableDumpAxes(fContainer, &fJSON);
            }
        };

    protected:

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
            AxisDumper axis_dumper(out,fLOD);
            for(std::size_t idx=0; idx < in->GetRank(); idx++)
            {
                axis_dumper.SetIndex(idx);
                apply_at< typename XContainerType::axis_pack_tuple_type, AxisDumper>(*in, idx, axis_dumper);
            }
        }

        class AxisDumper
        {
            public:
                AxisDumper(json* json_ptr, int level):
                    fAxisJSON(json_ptr),
                    fIndex(0),
                    fLOD(level)
                {};
                ~AxisDumper(){};

                void SetIndex(std::size_t idx){fIndex = idx;}

                template< typename XAxisType >
                void operator()(const XAxisType& axis)
                {
                    json j;
                    std::string class_name = MHO_ClassIdentity::ClassName<XContainerType>();
                    std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass<XContainerType>().as_string();
                    if(fLOD >= eJSONBasic)
                    {
                        j["class_name"] = class_name;
                        j["class_uuid"] = class_uuid;
                        j["name"] = axis.GetName();
                        j["units"] = axis.GetUnits();
                        j["rank"] = axis.GetRank();
                        j["total_size"] = axis.GetSize();
                    }

                    //data goes out flat-packed into 1-d array
                    json data;
                    for(auto it = axis.cbegin(); it != axis.cend(); it++)
                    {
                        data.push_back(*it);
                    }
                    j["data"] = data;

                    if(fLOD >= eJSONWithLabels)
                    {
                        //dump the axis labels too
                        json jilabels;
                        MHO_Interval<std::size_t> all(0, axis.GetSize() ); 
                        std::vector< const MHO_IntervalLabel* > labels = axis.GetIntervalsWhichIntersect(&all);
                        for(auto it = labels.begin(); it != labels.end(); it++)
                        {
                            json label_obj;
                            label_obj["lower_bound"] = (*it)->GetLowerBound();
                            label_obj["upper_bound"] = (*it)->GetUpperBound();

                            bool ok;
                            std::vector< std::string > keys; 
                            //only do the types in the "MHO_CommonLabelMap"
                            keys = (*it)->DumpKeys<char>();
                            for(auto k = keys.begin(); k != keys.end(); k++)
                            {
                                char c; ok = (*it)->Retrieve(*k, c); 
                                if(ok){label_obj[*k] = std::string(&c,1);}
                            }
                            keys = (*it)->DumpKeys<bool>();
                            for(auto k = keys.begin(); k != keys.end(); k++)
                            {
                                bool b; ok = (*it)->Retrieve(*k, b); if(ok){label_obj[*k] = b;}
                            }
                            keys = (*it)->DumpKeys<int>();
                            for(auto k = keys.begin(); k != keys.end(); k++)
                            {
                                int i; ok = (*it)->Retrieve(*k, i); if(ok){label_obj[*k] = i;}
                            }
                            keys = (*it)->DumpKeys<double>();
                            for(auto k = keys.begin(); k != keys.end(); k++)
                            {
                                double d; ok = (*it)->Retrieve(*k, d); if(ok){label_obj[*k] = d;}
                            }
                            keys = (*it)->DumpKeys<std::string>();
                            for(auto k = keys.begin(); k != keys.end(); k++)
                            {
                                std::string s; ok = (*it)->Retrieve(*k, s); if(ok){label_obj[*k] = s;}
                            }
                            jilabels.push_back(label_obj);
                        }
                        j["labels"] = jilabels;
                    }

                    std::stringstream ss;
                    ss << "axis_" << fIndex;
                    (*fAxisJSON)[ss.str().c_str()] = j;
                }

            private:

                json* fAxisJSON;
                std::size_t fIndex;
                int fLOD;
        };

    private:

        int fLOD;
        XContainerType* fContainer;
        json fJSON;

};



}//end of hops namespace

#endif /* end of include guard: MHO_ContainerJSON */
