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

#include <cstdint>
#include <string>
#include <utility>

#include "MHO_MultiTypeMap.hh"
#include "MHO_Serializable.hh"

namespace hops
{


class MHO_Taggable:
    public MHO_CommonLabelMap,
    virtual public MHO_Serializable
{
    public:

        MHO_Taggable():MHO_CommonLabelMap(){};
        MHO_Taggable(const MHO_Taggable& copy):MHO_CommonLabelMap(copy){};
        virtual ~MHO_Taggable(){};

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



        MHO_Taggable& operator=(const MHO_Taggable& rhs)
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

        virtual void CopyTags(const MHO_Taggable& rhs)
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

            std::vector< std::string > keys;
            keys = this->DumpKeys< char >();
            total_size += sizeof(uint64_t); //for the number of keys
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += keys[i].size();
                total_size += sizeof(char);
            }
            keys.clear();

            keys = this->DumpKeys<bool>();
            total_size += sizeof(uint64_t);
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += keys[i].size();
                total_size += sizeof(bool);
            }
            keys.clear();

            keys = this->DumpKeys<int>();
            total_size += sizeof(uint64_t);
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += keys[i].size();
                total_size += sizeof(int);
            }
            keys.clear();

            keys = this->DumpKeys<double>();
            total_size += sizeof(uint64_t);
            for(std::size_t i=0; i<keys.size(); i++)
            {
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += keys[i].size();
                total_size += sizeof(double);
            }
            keys.clear();

            keys = this->DumpKeys<std::string>();
            total_size += sizeof(uint64_t);
            for(std::size_t i=0; i<keys.size(); i++)
            {
                std::string val;
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += keys[i].size();
                Retrieve(keys[i], val);
                total_size += sizeof(uint64_t);//every string get streamed with a 'size'
                total_size += val.size();
            }
            keys.clear();
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
            //then for each type in the multi-type map, we need to stream in
            //the size of the map and then each subsequent key-value pair
            std::size_t n_elem;
            s >> n_elem;
            for(std::size_t i=0; i<n_elem; i++)
            {
                std::string key;
                char val;
                s >> key;
                s >> val;
                this->Insert(key, val);
            }
            n_elem = 0;

            s >> n_elem;
            for(std::size_t i=0; i<n_elem; i++)
            {
                std::string key;
                bool val;
                s >> key;
                s >> val;
                this->Insert(key, val);
            }
            n_elem = 0;

            s >> n_elem;
            for(std::size_t i=0; i<n_elem; i++)
            {
                std::string key;
                int val;
                s >> key;
                s >> val;
                this->Insert(key, val);
            }
            n_elem = 0;

            s >> n_elem;
            for(std::size_t i=0; i<n_elem; i++)
            {
                std::string key;
                double val;
                s >> key;
                s >> val;
                this->Insert(key, val);
            }
            n_elem = 0;

            s >> n_elem;
            for(std::size_t i=0; i<n_elem; i++)
            {
                std::string key;
                std::string val;
                s >> key;
                s >> val;
                this->Insert(key, val);
            }
            n_elem = 0;
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            //then for each type in the multi-type map, we need to stream out
            //the size of the map and then each subsequent key-value pair
            std::vector< std::string > keys;
            keys = this->DumpKeys< char >();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                char val;
                this->Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = this->DumpKeys<bool>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                bool val;
                this->Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = this->DumpKeys<int>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                int val;
                this->Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = this->DumpKeys<double>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                double val;
                this->Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();

            keys = this->DumpKeys<std::string>();
            s << keys.size();
            for(std::size_t i=0; i<keys.size(); i++)
            {
                std::string val;
                this->Retrieve(keys[i], val);
                s << keys[i];
                s << val;
            }
            keys.clear();
        };


};

}

#endif /* end of include guard: MHO_Taggable */
