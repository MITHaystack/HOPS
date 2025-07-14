#ifndef MHO_Taggable_HH__
#define MHO_Taggable_HH__

#include <string>
#include <utility>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Serializable.hh"
#include "MHO_Types.hh"

namespace hops
{

/*!
 *@file MHO_Taggable.hh
 *@class MHO_Taggable
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 16 11:17:19 2020 -0400
 *@brief A wrapper class for a json object, provides an interface that accepts tagging with key:value pairs
 */

/**
 * @brief Class MHO_Taggable
 */
class MHO_Taggable: public MHO_JSONWrapper, virtual public MHO_Serializable
{
    public:
        MHO_Taggable(): MHO_JSONWrapper(){};

        MHO_Taggable(const MHO_Taggable& copy): MHO_JSONWrapper(copy){};

        virtual ~MHO_Taggable(){};

        MHO_Taggable& operator=(const MHO_Taggable& rhs)
        {
            if(this != &rhs)
            {
                fObject = rhs.fObject;
            }
            return *this;
        }

        /**
         * @brief Copies tags from rhs to this instance if rhs is not empty and has valid object.
         * 
         * @param rhs Reference to const MHO_Taggable object whose tags will be copied
         * @note This is a virtual function.
         */
        virtual void CopyTags(const MHO_Taggable& rhs)
        {
            if(this != &rhs && &fObject && !(rhs.fObject.empty()))
            {
                fObject = rhs.fObject;
            }
        }

        /**
         * @brief Clears all tags from the object.
         */
        void ClearTags() { fObject.clear(); }

        /**
         * @brief Copies the contents of another MHO_Taggable object into this object.
         * 
         * @param copy_from_obj Reference to the MHO_Taggable object whose contents will be copied
         */
        void CopyFrom(const MHO_Taggable& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fObject.clear();
                fObject = copy_from_obj.fObject;
            }
        }

        /**
         * @brief Copies the contents of this object to another MHO_Taggable object if they are not the same.
         * 
         * @param copy_to_obj Reference to an MHO_Taggable object where data will be copied
         */
        void CopyTo(MHO_Taggable& copy_to_obj) const
        {
            if(this != &copy_to_obj)
            {
                copy_to_obj.fObject.clear();
                copy_to_obj.fObject = fObject;
            }
        }

        /**
         * @brief Getter for all meta data stored in this object as a json object
         * 
         * @return JSON object containing metadata
         */
        mho_json GetMetaDataAsJSON() const { return fObject; }

        //completely replaces the key:value (tags) data in this object with the infomration in the passed json object (use with caution)
        /**
         * @brief Setter for meta data as json
         * 
         * @param obj Input JSON object to replace fObject data
         */
        void SetMetaDataAsJSON(mho_json obj) { fObject = obj; }

    public: //MHO_Serializable interface
        /**
         * @brief Getter for version
         * 
         * @return MHO_ClassVersion version number
         * @note This is a virtual function.
         */
        virtual MHO_ClassVersion GetVersion() const override { return 0; };

        /**
         * @brief Getter for serialized size
         * 
         * @return Total serialized size as uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number

            //compute the serialized size of fObject in CBOR encoding.
            //this is a somewhat inconvenient waste of time to encode the data
            //just so we can find out the size (should we cache this serialized data?)
            std::vector< std::uint8_t > data = mho_json::to_cbor(fObject);
            uint64_t size = data.size();

            total_size += sizeof(uint64_t);            //for the encoded data-size parameter
            total_size += sizeof(uint64_t);            //for parameter that specifies the type of the JSON binary encoding
            total_size += size * sizeof(std::uint8_t); //for the actual data

            return total_size;
        }

        template< typename XStream > friend XStream& operator>>(XStream& s, MHO_Taggable& aData)
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
                    MHO_ObjectStreamState< XStream >::SetUnknown(s);
            }
            return s;
        }

        template< typename XStream > friend XStream& operator<<(XStream& s, const MHO_Taggable& aData)
        {
            switch(aData.GetVersion())
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                    break;
                default:
                    msg_error("containers", "error, cannot stream out MHO_Taggable object with unknown version: "
                                                << aData.GetVersion() << eom);
            }
            return s;
        }

    private:
        /**
         * @brief Replaces fObject data using input stream s of type XStream&.
         * 
         * @param s Input stream of type XStream&
         * @return No return value (void)
         */
        template< typename XStream > void StreamInData_V0(XStream& s) { s >> fObject; };

        /**
         * @brief Generates a UUID by hashing the class name and returning it as an MHO_UUID.
         * 
         * @tparam XStream Template parameter XStream
         * @return MHO_UUID representing the hashed class name
         * @note This is a virtual function.
         */
        /**
         * @brief Serializes fObject data into XStream using operator<<.
         * 
         * @tparam XStream Template parameter XStream
         * @param s Output stream of type XStream&
         * @return void
         */
        template< typename XStream > void StreamOutData_V0(XStream& s) const { s << fObject; };

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

} // namespace hops

#endif /*! end of include guard: MHO_Taggable */
