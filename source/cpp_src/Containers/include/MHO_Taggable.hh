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
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Serializable.hh"

namespace hops
{


class MHO_Taggable:
    public MHO_JSONWrapper,
    virtual public MHO_Serializable
{
    public:

        MHO_Taggable():
            MHO_JSONWrapper()
        {
            fObject["test"] = "test";
        };
        MHO_Taggable(const MHO_Taggable& copy):
            MHO_JSONWrapper(copy)
        {
        };
        
        virtual ~MHO_Taggable(){};

        MHO_Taggable& operator=(const MHO_Taggable& rhs)
        {
            if(this != &rhs)
            {
                fObject = rhs.fObject;
            }
            return *this;
        }

        virtual void CopyTags(const MHO_Taggable& rhs)
        {
            if(this != &rhs && !(rhs.fObject.empty()) )
            {
                fObject = rhs.fObject;
            }
        }

        void ClearTags()
        {
            fObject.clear();
        }

        void CopyFrom(const MHO_Taggable& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fObject.clear();
                fObject = copy_from_obj.fObject;
            }
        }

        void CopyTo(MHO_Taggable& copy_to_obj) const
        {
            if(this != &copy_to_obj)
            {
                copy_to_obj.fObject.clear();
                copy_to_obj.fObject = fObject;
            }
        }

        mho_json GetMetaDataAsJSON() const {return fObject;}

        //completely replaces fObject data (use with caution)
        void SetMetaDataAsJSON(mho_json obj){fObject = obj;}
    
    protected:
        
        //object in which the data is stashed
        mho_json fObject;

    public: //MHO_Serializable interface

        virtual MHO_ClassVersion GetVersion() const override {return 0;};

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number

            //compute the serialized size of fObject in CBOR encoding.
            //this is a somewhat inconvenient waste of time to encode the data 
            //just so we can find out the size (should we cache this serialized data?)
            std::vector<std::uint8_t> data = mho_json::to_cbor(fObject);
            uint64_t size = data.size();
            
            total_size += sizeof(uint64_t);//for the encoded data-size parameter
            total_size += sizeof(uint64_t);//for parameter that specifies the type of the JSON binary encoding
            total_size += size*sizeof(std::uint8_t); //for the actual data

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
            s >> fObject;
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            s << fObject;
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
