#ifndef MHO_KeyValueStore_HH__
#define MHO_KeyValueStore_HH__

#include <vector>
#include <complex>
#include <string>
#include <map>

#include "MHO_Types.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_Serializable.hh"

namespace hops
{


//TODO: Make sure this set of types is complete for data axis labeling needs
//Consider what other types might be needed (float? short? dates?)
//#pragma message("TODO FIXME -- need to specify fixed sized types in MHO_MultiTypeMap from cstdint for portability.")
typedef MHO_MultiTypeMap< std::string,

                          int64_t,
                          double,
                          std::complex<double>,
                          std::string,
                          std::vector<int64_t>,
                          std::vector<double>,
                          std::vector< std::complex<double> >
                        >
MHO_CommonKeyValueStore;


class MHO_KeyValueStore:
    public MHO_CommonKeyValueStore,
    virtual public MHO_Serializable
{
    public:

        MHO_KeyValueStore():MHO_CommonKeyValueStore(){};
        MHO_KeyValueStore(const MHO_KeyValueStore& copy):MHO_CommonKeyValueStore(copy){};
        virtual ~MHO_KeyValueStore(){};

        bool HasKey(const std::string& key) const
        {
            if(this->ContainsKey<char>(key)){return true;}
            if(this->ContainsKey<bool>(key)){return true;}
            if(this->ContainsKey<int>(key)){return true;}
            if(this->ContainsKey<double>(key)){return true;}
            if(this->ContainsKey<std::string>(key)){return true;}
            return false;
        }

        bool HasKey(const char* char_key) const
        {
            std::string key(char_key);
            return HasKey(key);
        }

        MHO_KeyValueStore& operator=(const MHO_KeyValueStore& rhs)
        {
            if(this != &rhs)
            {
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
                this->CopyFrom<std::string>(rhs);
            }
            return *this;
        }

        virtual void CopyTags(const MHO_KeyValueStore& rhs)
        {
            if(this != &rhs)
            {
                this->CopyFrom<char>(rhs);
                this->CopyFrom<bool>(rhs);
                this->CopyFrom<int>(rhs);
                this->CopyFrom<double>(rhs);
                this->CopyFrom<std::string>(rhs);
            }
        }

        void ClearTags()
        {
            this->Clear<char>();
            this->Clear<bool>();
            this->Clear<int>();
            this->Clear<double>();
            this->Clear<std::string>();
        }

    public: //MHO_Serializable interface

        virtual MHO_ClassVersion GetVersion() const override {return 0;};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number

            total_size += cm_aggregate_serializable_item_size<char>(*this);
            total_size += cm_aggregate_serializable_item_size<bool>(*this);
            total_size += cm_aggregate_serializable_item_size<int>(*this);
            total_size += cm_aggregate_serializable_item_size<double>(*this);
            total_size += cm_aggregate_serializable_item_size<std::string>(*this);

            return total_size;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_KeyValueStore& aData)
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

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_KeyValueStore& aData)
        {
            switch( aData.GetVersion() )
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers",
                        "error, cannot stream out MHO_KeyValueStore object with unknown version: "
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            cm_stream_importer<XStream, char>(s, *this);
            cm_stream_importer<XStream, bool>(s, *this);
            cm_stream_importer<XStream, int>(s, *this);
            cm_stream_importer<XStream, double>(s, *this);
            cm_stream_importer<XStream, std::string>(s, *this);
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            cm_stream_exporter<XStream, char>(s, *this);
            cm_stream_exporter<XStream, bool>(s, *this);
            cm_stream_exporter<XStream, int>(s, *this);
            cm_stream_exporter<XStream, double>(s, *this);
            cm_stream_exporter<XStream, std::string>(s, *this);
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


//
//
// template< typename XItemType >
// struct cm_size_calculator
// {
//     const XItemType* item;
//     uint64_t get_item_size()
//     {
//         return (uint64_t) sizeof(XItemType);
//     }
// };
//
// //string specialization
// template<>
// struct cm_size_calculator<std::string>
// {
//     const std::string* item;
//     uint64_t get_item_size()
//     {
//         //every string get streamed with a 'size'
//         uint64_t total_size = 0;
//         total_size += sizeof(uint64_t);
//         total_size += item->size();
//         return total_size;
//     }
// };
//
// //int vector specialization
// template<>
// struct cm_size_calculator< std::vector< int64_t > >
// {
//     const std::vector<int64_t>* item;
//     uint64_t get_item_size()
//     {
//         uint64_t total_size = 0;
//         total_size += sizeof(uint64_t);
//         total_size += sizeof(int64_t)*(item->size());
//         return total_size;
//     }
// };
//
// //double vector specialization
// template<>
// struct cm_size_calculator< std::vector< double > >
// {
//     const std::string* item;
//     uint64_t get_item_size()
//     {
//         //every string get streamed with a 'size'
//         uint64_t total_size = 0;
//         total_size += sizeof(uint64_t);
//         total_size += item->size();
//         return total_size;
//     }
// };
//
//
//
// template<typename XItemType >
// uint64_t cm_aggregate_serializable_item_size(const MHO_KeyValueStore& aMap)
// {
//     uint64_t total_size = 0;
//     std::vector< std::string > keys;
//     keys = aMap.DumpKeys< XItemType >();
//     total_size += sizeof(uint64_t); //for the number of keys
//
//     //calculate the size of each key and item in the map
//     cm_size_calculator<std::string> str_calc;
//     cm_size_calculator<XItemType> itm_calc;
//     for(std::size_t i=0; i<keys.size(); i++)
//     {
//         XItemType val;
//         aMap.Retrieve(keys[i], val);
//         str_calc.item = &(keys[i]);
//         total_size += str_calc.get_item_size();
//
//         itm_calc.item = &(val);
//         total_size += itm_calc.get_item_size();
//     }
//     return total_size;
// }
//
// template<typename XStream, typename XImportType >
// void cm_stream_importer(XStream& s, MHO_KeyValueStore& aMap)
// {
//     std::size_t n_elem;
//     s >> n_elem;
//     for(std::size_t i=0; i<n_elem; i++)
//     {
//         std::string key;
//         XImportType val;
//         s >> key;
//         s >> val;
//         aMap.Insert(key, val);
//     }
// }
//
// template<typename XStream, typename XExportType>
// void cm_stream_exporter(XStream& s, const MHO_KeyValueStore& aMap)
// {
//     std::vector< std::string > keys;
//     keys = aMap.DumpKeys< XExportType >();
//     s << (uint64_t) keys.size();
//     for(std::size_t i=0; i<keys.size(); i++)
//     {
//         XExportType val;
//         aMap.Retrieve(keys[i], val);
//         s << keys[i];
//         s << val;
//     }
// }
//
//
//







}//end namespace






#endif /* end of include guard: MHO_KeyValueStore_HH__ */
