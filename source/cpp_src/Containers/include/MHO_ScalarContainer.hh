#ifndef MHO_ScalarContainer_HH__
#define MHO_ScalarContainer_HH__

#include <complex>
#include <string>

#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_Serializable.hh"
#include "MHO_Taggable.hh"

namespace hops
{

/*!
 *@file MHO_ScalarContainer.hh
 *@class MHO_ScalarContainer
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 16 11:17:19 2020 -0400
 *@brief MHO_ScalarContainer, and wrapper class for storage of a scalar object
 */

template< typename XValueType >
class MHO_ScalarContainer: public MHO_ScalarContainerBase, public MHO_NDArrayWrapper< XValueType, 0 >, public MHO_Taggable
{
    public:
        MHO_ScalarContainer(): MHO_NDArrayWrapper< XValueType, 0 >(){};

        virtual ~MHO_ScalarContainer(){};

        MHO_ScalarContainer(MHO_ScalarContainer& obj): MHO_NDArrayWrapper< XValueType, 0 >(obj), MHO_Taggable(obj){};

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
         * @return Serialized size as a uint64_t.
         * @note This is a virtual function.
         */
        virtual uint64_t GetSerializedSize() const override { return ComputeSerializedSize(); }

        /**
         * @brief Calculates and returns the serialized size of an object.
         * 
         * @return The serialized size as a uint64_t.
         * @note This is a virtual function.
         */
        uint64_t ComputeSerializedSize() const
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version
            total_size += sizeof(XValueType);       //contents
            total_size += MHO_Taggable::GetSerializedSize();
            return total_size;
        }

        /**
         * @brief Setter for value
         * 
         * @param value Input value of type const XValueType& to be assigned internally
         */
        void SetValue(const XValueType& value) { fData = value; };

        /**
         * @brief Getter for value
         * 
         * @return Current value of type XValueType.
         */
        XValueType GetValue() { return fData; };

        //have to make base class functions visible
        using MHO_NDArrayWrapper< XValueType, 0 >::SetData;
        using MHO_NDArrayWrapper< XValueType, 0 >::GetData;

        /**
         * @brief Getter for size
         * 
         * @return Size as std::size_t
         */
        std::size_t GetSize() const { return 1; };

    public:
        template< typename XStream > friend XStream& operator>>(XStream& s, MHO_ScalarContainer& aData)
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

        template< typename XStream > friend XStream& operator<<(XStream& s, const MHO_ScalarContainer& aData)
        {
            switch(aData.GetVersion())
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                    break;
                default:
                    msg_error("containers", "error, cannot stream out MHO_ScalarContainer object with unknown version: "
                                                << aData.GetVersion() << eom);
            }
            return s;
        }

    private:
        /**
         * @brief Reads data from stream and stores it in object's fData and MHO_Taggable fields.
         * 
         * @param s Input stream of type XStream&
         * @return void
         */
        template< typename XStream > void StreamInData_V0(XStream& s)
        {
            s >> this->fData;
            s >> static_cast< MHO_Taggable& >(*this);
        };

        /**
         * @brief Serializes data and object metadata to an output stream.
         * 
         * @param s Output stream of type XStream&.
         * @return No return value (void).
         */
        template< typename XStream > void StreamOutData_V0(XStream& s) const
        {
            s << this->fData;
            s << static_cast< const MHO_Taggable& >(*this);
        };

        /**
         * @brief Generates a UUID by hashing the class name and returning it.
         * 
         * @return MHO_UUID representing the hashed class name.
         * @note This is a virtual function.
         */
        virtual MHO_UUID DetermineTypeUUID() const override
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(*this);
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }

    protected:
        using MHO_NDArrayWrapper< XValueType, 0 >::fData;
};

//specialization for string elements
//(NOTE: we need to use 'inline' to satisfy one-definiton rule, otherwise we have to stash this in a .cc file)
/**
 * @brief Function MHO_ScalarContainer<std::string>::ComputeSerializedSize
 * 
 * @return Return value (uint64_t MHO_ScalarContainer< std::string)
 */
template<> inline uint64_t MHO_ScalarContainer< std::string >::ComputeSerializedSize() const
{
    uint64_t total_size = 0;
    total_size += sizeof(MHO_ClassVersion); //version
    total_size += sizeof(uint64_t);         //string size parameter
    total_size += fData.size();             //size of the string
    total_size += MHO_Taggable::GetSerializedSize();
    return total_size;
}

////////////////////////////////////////////////////////////////////////////////
//using declarations for all basic 'plain-old-data' types
using MHO_ScalarBool = MHO_ScalarContainer< bool >;
using MHO_ScalarChar = MHO_ScalarContainer< char >;
using MHO_ScalarUChar = MHO_ScalarContainer< unsigned char >;
using MHO_ScalarShort = MHO_ScalarContainer< short >;
using MHO_ScalarUShort = MHO_ScalarContainer< unsigned short >;
using MHO_ScalarInt = MHO_ScalarContainer< int >;
using MHO_ScalarUInt = MHO_ScalarContainer< unsigned int >;
using MHO_ScalarLong = MHO_ScalarContainer< long >;
using MHO_ScalarULong = MHO_ScalarContainer< unsigned long >;
using MHO_ScalarLongLong = MHO_ScalarContainer< long long >;
using MHO_ScalarULongLong = MHO_ScalarContainer< unsigned long long >;
using MHO_ScalarFloat = MHO_ScalarContainer< float >;
using MHO_ScalarDouble = MHO_ScalarContainer< double >;
using MHO_ScalarLongDouble = MHO_ScalarContainer< long double >;
using MHO_ScalarComplexFloat = MHO_ScalarContainer< std::complex< float > >;
using MHO_ScalarComplexDouble = MHO_ScalarContainer< std::complex< double > >;
using MHO_ScalarComplexLongDouble = MHO_ScalarContainer< std::complex< long double > >;
using MHO_ScalarString = MHO_ScalarContainer< std::string >;

////////////////////////////////////////////////////////////////////////////////

} // namespace hops

#endif /*! end of include guard: MHO_ScalarContainer */
