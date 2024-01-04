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

        MHO_Taggable()
        {
            this->SetObject(&fTags); //MHO_JSONWrapper
        };
        MHO_Taggable(const MHO_Taggable& copy)
        {
            fTags = copy.fTags;
            this->SetObject(&fTags);
        };
        
        virtual ~MHO_Taggable(){};

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

        std::size_t MapSize() const {return fTags.size(); }

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
    
        //declare the friend class which extends this interface 
        // friend class MHO_IndexLabelInterface;
    
    protected:
        
        //object in which the data is stashed
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
            
            total_size += sizeof(uint64_t);//for the encoded data-size parameter
            total_size += sizeof(uint64_t);//for parameter that specifies the type of the encoding
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
