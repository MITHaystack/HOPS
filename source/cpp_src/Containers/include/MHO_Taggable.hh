#ifndef MHO_Taggable_HH__
#define MHO_Taggable_HH__

/*
*File: MHO_Taggable.hh
*Class: MHO_Taggable
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/


#include <string>
#include <utility>

#include "MHO_Types.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Serializable.hh"

namespace hops
{


class MHO_Taggable: virtual public MHO_Serializable
{
    public:

        MHO_Taggable(){};
        MHO_Taggable(const MHO_Taggable& copy)
        {
            fTags = copy.fTags;
        };
        
        virtual ~MHO_Taggable(){};

        //old MHO_Taggable inteface

        bool HasKey(const std::string& key) const
        {
            auto it = fTags.find(key);
            if(it != fTags.end()){return true;}
            return false;
        }

        bool HasKey(const char* char_key) const
        {
            std::string key(char_key);
            return HasKey(key);
        }

        MHO_Taggable& operator=(const MHO_Taggable& rhs)
        {
            if(this != &rhs)
            {
                fTags = rhs.fTags;
            }
            return *this;
        }

        virtual void CopyTags(const MHO_Taggable& rhs)
        {
            if(this != &rhs)
            {
                fTags = rhs.fTags;
            }
        }

        void ClearTags()
        {
            fTags.clear();
        }
    
        //end of old MHO_Taggable inteface 
        
        //start of multi-type map interface 
        
        

        std::size_t MapSize() const {return fTags.size(); }

        template< typename XValueType> 
        void Insert(const std::string& key, const XValueType& value)
        {
            fTags[key] = value;//allow replacement of values
        }

        void Clear()
        {
            fTags.clear();
        }

        template< typename XValueType> 
        bool Retrieve(const std::string& key, XValueType& value) const
        {
            auto iter = fTags.find(key);
            if(iter == fTags.end()){return false;}
            else
            {
                mho_json test;
                test["test"] = value;
                if(test["test"].type() == fTags[key].type()) //this is a major KLUDGE, TODO FIXME
                {
                    value = fTags[key].get<XValueType>();
                    return true;
                }
                else 
                {
                    return false;
                }
            }
        }

        std::vector<std::string> DumpKeys() const
        {
            std::vector< std::string > keys;
            for(auto iter = fTags.begin(); iter != fTags.end(); iter++)
            {
                keys.push_back(iter.key());
            }
            return keys;
        }

        void DumpMap() const
        {
            for(auto iter = fTags.begin(); iter != fTags.end(); iter++)
            {
                std::cout<<iter.key()<<" : "<<iter.value()<<std::endl;
            }
        }

        bool ContainsKey(const std::string& key) const
        {
            auto iter = fTags.find(key);
            if(iter == fTags.end()){return false;}
            else{return true;}
        }

        void CopyFrom(const MHO_Taggable& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fTags.clear();
                fTags = copy_from_obj.fTags;
            }
        }

        void CopyTo(MHO_Taggable& copy_to_obj) const
        {
            if(this != &copy_to_obj)
            {
                copy_to_obj.fTags.clear();
                copy_to_obj.fTags = fTags;
            }
        }

        
        mho_json GetDataAsJSON() const {return fTags;}
        //end of multi-type map interface 
    
    
    private:
        
        mho_json fTags;

    public: //MHO_Serializable interface

        virtual MHO_ClassVersion GetVersion() const override {return 0;};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number

            //compute the serialized size of fTags in CBOR encoding.
            //this is a somewhat inconvenient waste of time to encode the data 
            //just so we can find out the size (should we cache this serialized data?)
            std::vector<std::uint8_t> data = mho_json::to_cbor(fTags);
            uint64_t size = data.size();
            
            total_size += sizeof(uint64_t);//for the encoded data size parameter 
            total_size += size*sizeof(std::uint8_t);

            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_Taggable& aData)
        {
            MHO_ClassVersion vers;
            s >> vers;
            switch(vers)
            {
                case 0:
                    aData.StreamInData_V0(s);
                break;
                default:
                    MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                    //Flag this as an unknown object version so we can skip over this data
                    MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            return s;
        }

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_Taggable& aData)
        {
            switch( aData.GetVersion() )
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers",
                        "error, cannot stream out MHO_Taggable object with unknown version: "
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            s >> fTags;
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << fTags;
        };

        virtual MHO_UUID DetermineTypeUUID() const override
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(*this);
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }

};

}

#endif /* end of include guard: MHO_Taggable */















// #ifndef MHO_Taggable_HH__
// #define MHO_Taggable_HH__
// 
// /*
// *File: MHO_Taggable.hh
// *Class: MHO_Taggable
// *Author: J. Barrett
// *Email: barrettj@mit.edu
// *Date:
// *Description:
// */
// 
// 
// #include <string>
// #include <utility>
// 
// #include "MHO_Types.hh"
// #include "MHO_MultiTypeMap.hh"
// #include "MHO_Serializable.hh"
// 
// namespace hops
// {
// 
// 
// class MHO_Taggable:
//     public MHO_CommonLabelMap,
//     virtual public MHO_Serializable
// {
//     public:
// 
//         MHO_Taggable():MHO_CommonLabelMap(){};
//         MHO_Taggable(const MHO_Taggable& copy):MHO_CommonLabelMap(copy){};
//         virtual ~MHO_Taggable(){};
// 
//         bool HasKey(const std::string& key) const
//         {
//             if(this->ContainsKey<char>(key)){return true;}
//             if(this->ContainsKey<bool>(key)){return true;}
//             if(this->ContainsKey<int>(key)){return true;}
//             if(this->ContainsKey<double>(key)){return true;}
//             if(this->ContainsKey<std::string>(key)){return true;}
//             return false;
//         }
// 
//         bool HasKey(const char* char_key) const
//         {
//             std::string key(char_key);
//             return HasKey(key);
//         }
// 
//         MHO_Taggable& operator=(const MHO_Taggable& rhs)
//         {
//             if(this != &rhs)
//             {
//                 this->CopyFrom<char>(rhs);
//                 this->CopyFrom<bool>(rhs);
//                 this->CopyFrom<int>(rhs);
//                 this->CopyFrom<double>(rhs);
//                 this->CopyFrom<std::string>(rhs);
//             }
//             return *this;
//         }
// 
//         virtual void CopyTags(const MHO_Taggable& rhs)
//         {
//             if(this != &rhs)
//             {
//                 this->CopyFrom<char>(rhs);
//                 this->CopyFrom<bool>(rhs);
//                 this->CopyFrom<int>(rhs);
//                 this->CopyFrom<double>(rhs);
//                 this->CopyFrom<std::string>(rhs);
//             }
//         }
// 
//         void ClearTags()
//         {
//             this->Clear<char>();
//             this->Clear<bool>();
//             this->Clear<int>();
//             this->Clear<double>();
//             this->Clear<std::string>();
//         }
// 
//     public: //MHO_Serializable interface
// 
//         virtual MHO_ClassVersion GetVersion() const override {return 0;};
// 
//         virtual uint64_t GetSerializedSize() const override
//         {
//             uint64_t total_size = 0;
//             total_size += sizeof(MHO_ClassVersion); //version number
// 
//             total_size += cm_aggregate_serializable_item_size<char>(*this);
//             total_size += cm_aggregate_serializable_item_size<bool>(*this);
//             total_size += cm_aggregate_serializable_item_size<int>(*this);
//             total_size += cm_aggregate_serializable_item_size<double>(*this);
//             total_size += cm_aggregate_serializable_item_size<std::string>(*this);
// 
//             return total_size;
//         }
// 
//         template<typename XStream> friend XStream& operator>>(XStream& s, MHO_Taggable& aData)
//         {
//             MHO_ClassVersion vers;
//             s >> vers;
//             switch(vers)
//             {
//                 case 0:
//                     aData.StreamInData_V0(s);
//                 break;
//                 default:
//                     MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
//                     //Flag this as an unknown object version so we can skip over this data
//                     MHO_ObjectStreamState<XStream>::SetUnknown(s);
//             }
//             return s;
//         }
// 
//         template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_Taggable& aData)
//         {
//             switch( aData.GetVersion() )
//             {
//                 case 0:
//                     s << aData.GetVersion();
//                     aData.StreamOutData_V0(s);
//                 break;
//                 default:
//                     msg_error("containers",
//                         "error, cannot stream out MHO_Taggable object with unknown version: "
//                         << aData.GetVersion() << eom );
//             }
//             return s;
//         }
// 
//     private:
// 
//         template<typename XStream> void StreamInData_V0(XStream& s)
//         {
//             cm_stream_importer<XStream, char>(s, *this);
//             cm_stream_importer<XStream, bool>(s, *this);
//             cm_stream_importer<XStream, int>(s, *this);
//             cm_stream_importer<XStream, double>(s, *this);
//             cm_stream_importer<XStream, std::string>(s, *this);
//         };
// 
//         template<typename XStream> void StreamOutData_V0(XStream& s) const
//         {
//             cm_stream_exporter<XStream, char>(s, *this);
//             cm_stream_exporter<XStream, bool>(s, *this);
//             cm_stream_exporter<XStream, int>(s, *this);
//             cm_stream_exporter<XStream, double>(s, *this);
//             cm_stream_exporter<XStream, std::string>(s, *this);
//         };
// 
//         virtual MHO_UUID DetermineTypeUUID() const override
//         {
//             MHO_MD5HashGenerator gen;
//             gen.Initialize();
//             std::string name = MHO_ClassIdentity::ClassName(*this);
//             gen << name;
//             gen.Finalize();
//             return gen.GetDigestAsUUID();
//         }
// 
// };
// 
// }
// 
// #endif /* end of include guard: MHO_Taggable */
