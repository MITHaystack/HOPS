#ifndef MHO_ContainerJSONConverter_HH__
#define MHO_ContainerJSONConverter_HH__

#include "MHO_ClassIdentity.hh"
#include "MHO_ExtensibleElement.hh"
#include "MHO_JSONHeaderWrapper.hh"

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_ScalarContainer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_Taggable.hh"
#include "MHO_VectorContainer.hh"

#include "MHO_NumpyTypeCode.hh"

namespace hops
{

/*!
 *@file MHO_ContainerJSONConverter.hh
 *@class MHO_ContainerJSONConverter
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Feb 18 14:17:16 2022 -0500
 *@brief Converts a given ndarray-based container into a JSON representation
 * this isn't really intended for data transport/storage, but only as
 * conversion to an ascii-like representation for human inspection/debugging
 * @details the (integer) code specifing the level-of-detail in the output is as follows:
 * 0 = basic quantities (rank, dimensions, etc.)
 * 1 = basic quantities plus tags
 * 2 = basic quantities plus the axes (if the object is a table)
 * 3 = basic quantities plus axes with interval labels
 * 4 = everything including the main data array
 */

//verbosity controlling enum
enum MHO_JSONVerbosityLevel : int
{
    eJSONBasicLevel = 0,      //basic quantities (rank, dimensions, etc.)
    eJSONTagsLevel,           //basic quantities plus tags
    eJSONAxesLevel,           //basic quantities plus the axes (if the object is a table)
    eJSONAxesWithLabelsLevel, //basic quantities plus axes with interval labels
    eJSONAllLevel             //everything including the main data array
};

//short hand aliases
static const MHO_JSONVerbosityLevel eJSONBasic = MHO_JSONVerbosityLevel::eJSONBasicLevel;
static const MHO_JSONVerbosityLevel eJSONTags = MHO_JSONVerbosityLevel::eJSONTagsLevel;
static const MHO_JSONVerbosityLevel eJSONWithAxes = MHO_JSONVerbosityLevel::eJSONAxesLevel;
static const MHO_JSONVerbosityLevel eJSONWithLabels = MHO_JSONVerbosityLevel::eJSONAxesWithLabelsLevel;
static const MHO_JSONVerbosityLevel eJSONAll = MHO_JSONVerbosityLevel::eJSONAllLevel;

using hops::eJSONAll;
using hops::eJSONBasic;
using hops::eJSONTags;
using hops::eJSONWithAxes;
using hops::eJSONWithLabels;

/**
 * @brief Fills a JSON object with metadata from a Taggable map.
 * 
 * @param map Const reference to MHO_Taggable map containing metadata.
 * @param obj_tags (mho_json&)
 */
inline void FillJSONFromTaggable(const MHO_Taggable* map, mho_json& obj_tags)
{
    bool ok;
    obj_tags = map->GetMetaDataAsJSON();
}

/**
 * @brief Class MHO_JSONConverter
 */
class MHO_JSONConverter
{
    public:
        MHO_JSONConverter(): fLOD(eJSONBasic), fRank(0), fRawByteSize(0), fRawData(nullptr), fRawDataDescriptor(""){};
        virtual ~MHO_JSONConverter(){};

        /**
         * @brief Setter for level of detail
         * 
         * @param level New level of detail (eJSONBasicLevel, eJSONTagsLevel, or eJSONAxesLevel)
         */
        void SetLevelOfDetail(int level) { fLOD = level; };

        /**
         * @brief Getter for json
         * 
         * @return Pointer to mho_json object
         */
        mho_json* GetJSON() { return &fJSON; }

        /**
         * @brief Setter for object to convert
         * 
         * @param !obj Parameter description
         * @note This is a virtual function.
         */
        virtual void SetObjectToConvert(MHO_Serializable* /*!obj*/) = 0;
        /**
         * @brief Constructs a JSON representation if fContainer is not nullptr.
         * @note This is a virtual function.
         */
        virtual void ConstructJSONRepresentation() = 0;


        /**
         * @brief Getter for rank, needed for access to raw data in table containers this is a bit of a hack for 'hops2flat'
         * 
         * @return Current value of fRank as std::size_t
         */
        std::size_t GetRank() const {return fRank;}
        /**
         * @brief Getter for raw byte size
         * 
         * @return std::size_t representing the raw byte size.
         */
        std::size_t GetRawByteSize() const {return fRawByteSize;};
        /**
         * @brief Getter for raw data
         * 
         * @return Pointer to char representing the raw data.
         */
        const char* GetRawData() const {return fRawData;};
        /**
         * @brief Getter for raw data descriptor
         * 
         * @return Current raw data descriptor as a string.
         */
        std::string GetRawDataDescriptor() const {return fRawDataDescriptor;}

    protected:
        
        /**
         * @brief Inserts an element into a JSON data list (helper functions for generic data insertion for elements of a list)
         * 
         * @param value The value to insert of type XValueType.
         * @param data (mho_json&)
         * @return No return value (void)
         */
        template< typename XValueType > void InsertElement(const XValueType& value, mho_json& data) { data.push_back(value); }

        /**
         * @brief Inserts a complex value into a JSON data array, 
         * this is a specialization for complex<> element data insertion, and 
         * is needed because mho_json doesn't have a first-class complex type
         * 
         * @tparam XValueType Template parameter XValueType
         * @param value The complex value to insert.
         * @param data (mho_json&)
         */

        void InsertElement(const std::complex< long double >& value, mho_json& data)
        {
            data.push_back({value.real(), value.imag()});
        }

        /**
         * @brief Inserts a complex double value into an mho_json data structure.
         * 
         * @param value The complex double value to insert.
         * @param data (mho_json&)
         */
        void InsertElement(const std::complex< double >& value, mho_json& data)
        {
            data.push_back({value.real(), value.imag()});
        }

        /**
         * @brief Inserts a complex float value into the given mho_json data structure.
         * 
         * @param value The complex float value to insert.
         * @param data (mho_json&)
         */
        void InsertElement(const std::complex< float >& value, mho_json& data) { data.push_back({value.real(), value.imag()}); }

        //data
        int fLOD;
        mho_json fJSON;
        
        //for extracting container raw data (with no meta data structures)
        std::size_t fRank;
        std::size_t fRawByteSize;
        const char* fRawData;
        std::string fRawDataDescriptor;
};

/**
 * @brief Class MHO_ContainerJSONConverter
 */
template< typename XContainerType > class MHO_ContainerJSONConverter: public MHO_JSONConverter
{
    public:
        MHO_ContainerJSONConverter(): MHO_JSONConverter() { fContainer = nullptr; }

        MHO_ContainerJSONConverter(MHO_ExtensibleElement* element): MHO_JSONConverter()
        {
            fContainer = dynamic_cast< XContainerType* >(element);
        }

        virtual ~MHO_ContainerJSONConverter() {}

        /**
         * @brief Setter for object to convert
         * 
         * @param obj Pointer to an MHO_Serializable object to be converted
         * @note This is a virtual function.
         */
        virtual void SetObjectToConvert(MHO_Serializable* obj) { fContainer = dynamic_cast< XContainerType* >(obj); };

        /**
         * @brief Constructs a JSON representation if fContainer is not nullptr.
         * @note This is a virtual function.
         */
        virtual void ConstructJSONRepresentation()
        {
            if(fContainer != nullptr)
            {
                ConstructJSON(fContainer);
            }
        }

    private:
        XContainerType* fContainer;

    protected:
        /**
         * @brief Constructs a JSON representation for an object of type XCheckType (unspecialized template doesn't do much) 
         * @details uses SFINAE to generate specialization for the rest of the container types
         * @param obj Pointer to the object of type XCheckType.
         * @return No return value (void)
         */
        template< typename XCheckType > void ConstructJSON(const XCheckType* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< XCheckType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XCheckType >().as_string();
            fJSON["class_name"] = class_name;
            fJSON["class_uuid"] = class_uuid;
            
            //for raw data extraction (not possible)
            fRank = 0;
            fRawByteSize = 0;
            fRawData = nullptr;
            fRawDataDescriptor = "";
        };

        /**
         * @brief Constructs a JSON representation for an XContainerType object (scalar specialization)
         * @tparam XCheckType Template parameter XCheckType
         * @param obj Pointer to const XContainerType object.
         */
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_ScalarContainerBase, XCheckType >::value, void >::type
        ConstructJSON(const XContainerType* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            if(fLOD >= eJSONBasic)
            {
                fJSON["class_name"] = class_name;
                fJSON["class_uuid"] = class_uuid;
                fJSON["rank"] = fContainer->GetRank();
                fJSON["total_size"] = fContainer->GetSize();
            }

            if(fLOD >= eJSONTags)
            {
                mho_json jtags;
                // FillJSONFromCommonMap(fContainer, jtags);
                FillJSONFromTaggable(fContainer, jtags);
                fJSON["tags"] = jtags;
            }

            //just one element
            if(fLOD >= eJSONAll)
            {
                mho_json data;
                InsertElement(fContainer->GetData(), data);
                fJSON["data"] = data;
            }
            
            //for raw data extraction (not possible)
            fRank = 0;
            fRawByteSize = 0;
            fRawData = nullptr;
            fRawDataDescriptor = "";
        };

        /**
         * @brief Constructs a JSON representation for an XContainerType object (vector specialization (but not an axis!)).
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param obj Pointer to the const XContainerType object to construct JSON from.
         */
        template< typename XCheckType = XContainerType >
        typename std::enable_if< (std::is_base_of< MHO_VectorContainerBase, XCheckType >::value &&
                                  !std::is_base_of< MHO_AxisBase, XCheckType >::value),
                                 void >::type
        ConstructJSON(const XContainerType* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            if(fLOD >= eJSONBasic)
            {
                fJSON["class_name"] = class_name;
                fJSON["class_uuid"] = class_uuid;
                fJSON["rank"] = fContainer->GetRank();
                fJSON["total_size"] = fContainer->GetSize();
                fJSON["dimensions"] = fContainer->GetDimensionArray();
            }

            if(fLOD >= eJSONTags)
            {
                mho_json jtags;
                FillJSONFromTaggable(fContainer, jtags);
                //FillJSONFromCommonMap(fContainer ,jtags);
                fJSON["tags"] = jtags;
            }

            //data goes out flat-packed into 1-d array
            if(fLOD >= eJSONAll)
            {
                mho_json data;
                for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
                {
                    InsertElement(*it, data);
                }
                fJSON["data"] = data;
            }
            
            //for raw data extraction 
            std::size_t elem_size = sizeof( typename XContainerType::value_type);
            std::size_t n_elem = fContainer->GetSize();
            fRank = XContainerType::rank::value;
            fRawByteSize = elem_size*n_elem;
            fRawData = reinterpret_cast<const char*>( fContainer->GetData() );
            fRawDataDescriptor = MHO_NumpyTypeCode< typename XContainerType::value_type >();
        };

        /**
         * @brief Constructs a JSON representation for an XContainerType object (axis specialization)
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param obj Pointer to const XContainerType object.
         */
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_AxisBase, XCheckType >::value, void >::type
        ConstructJSON(const XContainerType* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            if(fLOD >= eJSONBasic)
            {
                fJSON["class_name"] = class_name;
                fJSON["class_uuid"] = class_uuid;
                fJSON["rank"] = fContainer->GetRank();
                fJSON["total_size"] = fContainer->GetSize();
                fJSON["dimensions"] = fContainer->GetDimensionArray();
            }

            if(fLOD >= eJSONTags)
            {
                mho_json jtags;
                FillJSONFromTaggable(fContainer, jtags);
                //FillJSONFromCommonMap(fContainer ,jtags);
                fJSON["tags"] = jtags;
            }

            //data goes out flat-packed into 1-d array
            if(fLOD >= eJSONAll)
            {
                mho_json data;
                for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
                {
                    InsertElement(*it, data);
                }
                fJSON["data"] = data;
            }
            
            //for raw data extraction 
            std::size_t elem_size = sizeof( typename XContainerType::value_type);
            std::size_t n_elem = fContainer->GetSize();
            fRank = XContainerType::rank::value;
            fRawByteSize = elem_size*n_elem;
            fRawData = reinterpret_cast<const char*>( fContainer->GetData() );
            fRawDataDescriptor = MHO_NumpyTypeCode< typename XContainerType::value_type >();
        };


        /**
         * @brief Constructs a JSON representation for the given XContainerType object (table specialization).
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param obj Pointer to the const XContainerType object to construct JSON from.
         */
        template< typename XCheckType = XContainerType >
        typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
        ConstructJSON(const XContainerType* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< XContainerType >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XContainerType >().as_string();
            if(fLOD >= eJSONBasic)
            {
                fJSON["class_name"] = class_name;
                fJSON["class_uuid"] = class_uuid;
                fJSON["rank"] = fContainer->GetRank();
                fJSON["total_size"] = fContainer->GetSize();
                fJSON["dimensions"] = fContainer->GetDimensionArray();
                fJSON["strides"] = fContainer->GetStrideArray();
            }

            if(fLOD >= eJSONTags)
            {
                mho_json jtags;
                FillJSONFromTaggable(fContainer, jtags);
                //FillJSONFromCommonMap(fContainer, jtags);
                fJSON["tags"] = jtags;
            }

            //data goes out flat-packed into 1-d array
            if(fLOD >= eJSONAll)
            {
                mho_json data;
                for(auto it = fContainer->cbegin(); it != fContainer->cend(); it++)
                {
                    InsertElement(*it, data);
                }
                fJSON["data"] = data;
            }

            if(fLOD >= eJSONWithAxes)
            {
                AxisDumper axis_dumper(&fJSON, fLOD);
                for(std::size_t idx = 0; idx < obj->GetRank(); idx++)
                {
                    axis_dumper.SetIndex(idx);
                    apply_at< typename XContainerType::axis_pack_tuple_type, AxisDumper >(*obj, idx, axis_dumper);
                }
            }
            
            //for raw data extraction 
            std::size_t elem_size = sizeof( typename XContainerType::value_type );
            std::size_t n_elem = fContainer->GetSize();
            fRank = XContainerType::rank::value;
            fRawByteSize = elem_size*n_elem;
            fRawData = reinterpret_cast<const char*>( fContainer->GetData() );
            fRawDataDescriptor = MHO_NumpyTypeCode< typename XContainerType::value_type >();
        };

        /**
         * @brief Class AxisDumper - helper class needed to extract MHO_Axis objects from MHO_AxisPack objects inside MHO_TableContainer types
         */
        class AxisDumper
        {
            public:
                AxisDumper(mho_json* json_ptr, int level): fAxisJSON(json_ptr), fIndex(0), fLOD(level){};
                ~AxisDumper(){};

                /**
                 * @brief Setter for index
                 * 
                 * @param idx New value to set for internal index
                 */
                void SetIndex(std::size_t idx) { fIndex = idx; }

                template< typename XAxisType > void operator()(const XAxisType& axis)
                {
                    mho_json j;
                    std::string class_name = MHO_ClassIdentity::ClassName< XAxisType >();
                    std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< XAxisType >().as_string();
                    if(fLOD >= eJSONBasic)
                    {
                        j["class_name"] = class_name;
                        j["class_uuid"] = class_uuid;
                        j["rank"] = axis.GetRank();
                        j["total_size"] = axis.GetSize();
                        j["dimensions"] = axis.GetDimensionArray();
                    }

                    if(fLOD >= eJSONTags)
                    {
                        mho_json jtags;
                        FillJSONFromTaggable(&axis, jtags);
                        //FillJSONFromCommonMap(&axis, jtags);
                        j["tags"] = jtags;
                    }

                    //data goes out flat-packed into 1-d array
                    mho_json data;
                    for(auto it = axis.cbegin(); it != axis.cend(); it++)
                    {
                        data.push_back(*it);
                    }
                    j["data"] = data;

                    // if(fLOD >= eJSONWithLabels)
                    // {
                    //     //dump the axis labels too
                    //     mho_json jilabels;
                    //     MHO_Interval<std::size_t> all(0, axis.GetSize() );
                    //     std::vector< MHO_IntervalLabel > labels = axis.GetIntervalsWhichIntersect(all);
                    //     for(auto it = labels.begin(); it != labels.end(); it++)
                    //     {
                    //         mho_json label_obj;
                    //         label_obj["lower_bound"] = it->GetLowerBound();
                    //         label_obj["upper_bound"] = it->GetUpperBound();
                    //         FillJSONFromCommonMap(&(*it), label_obj);
                    //         jilabels.push_back(label_obj);
                    //     }
                    //     j["labels"] = jilabels;
                    // }

                    std::stringstream ss;
                    ss << "axis_" << fIndex;
                    (*fAxisJSON)[ss.str().c_str()] = j;
                };

            private:
                mho_json* fAxisJSON;
                std::size_t fIndex;
                int fLOD;
        };
};

/**
 * @brief Class MHO_ContainerJSONConverter<MHO_ObjectTags>
 */
template<> class MHO_ContainerJSONConverter< MHO_ObjectTags >: public MHO_JSONConverter
{
    public:
        MHO_ContainerJSONConverter(): MHO_JSONConverter() { fContainer = nullptr; }

        MHO_ContainerJSONConverter(MHO_ExtensibleElement* element): MHO_JSONConverter()
        {
            fContainer = dynamic_cast< MHO_ObjectTags* >(element);
        }

        virtual ~MHO_ContainerJSONConverter() {}

        /**
         * @brief Setter for object to convert
         * 
         * @param obj Pointer to an MHO_Serializable object to be converted
         * @note This is a virtual function.
         */
        virtual void SetObjectToConvert(MHO_Serializable* obj) { fContainer = dynamic_cast< MHO_ObjectTags* >(obj); };

        /**
         * @brief Constructs a JSON representation if fContainer is not nullptr.
         * @note This is a virtual function.
         */
        virtual void ConstructJSONRepresentation()
        {
            if(fContainer != nullptr)
            {
                ConstructJSON(fContainer);
            }
        }

    private:
        /**
         * @brief Constructs a JSON representation for an object and its container if it exists.
         * 
         * @param obj Pointer to an MHO_ObjectTags object.
         */
        void ConstructJSON(MHO_ObjectTags* obj)
        {
            fJSON.clear();
            std::string class_name = MHO_ClassIdentity::ClassName< MHO_ObjectTags >();
            std::string class_uuid = MHO_ClassIdentity::GetUUIDFromClass< MHO_ObjectTags >().as_string();
            fJSON["class_name"] = class_name;
            fJSON["class_uuid"] = class_uuid;

            std::vector< MHO_UUID > obj_uuids = obj->GetAllObjectUUIDs();
            for(std::size_t i = 0; i < obj_uuids.size(); i++)
            {
                fJSON["object_uuids"].push_back(obj_uuids[i].as_string());
            }

            mho_json jtags;
            FillJSONFromTaggable(obj, jtags);
            fJSON["tags"] = jtags;
            
            //for raw data extraction (not possible)
            fRank = 0;
            fRawByteSize = 0;
            fRawData = nullptr;
            fRawDataDescriptor = "tags";
        };

        MHO_ObjectTags* fContainer;
};

} // namespace hops

#endif
