#ifndef MHO_TableContainer_HH__
#define MHO_TableContainer_HH__

#include <string>
#include <tuple>
#include <utility>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_Meta.hh"
#include "MHO_MultiTypeMap.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_Units.hh"
#include "MHO_VectorContainer.hh"

#include "MHO_FileStreamer.hh"

namespace hops
{

/*!
 *@file MHO_TableContainer.hh
 *@class MHO_TableContainer
 *@author J. Barrett - barrettj@mit.edu
 *@date Sun Jan 24 14:03:03 2021 -0500
 *@brief
 */

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainer: public MHO_TableContainerBase,
                          public MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >,
                          public XAxisPackType,
                          public MHO_Taggable
{
    public:
        MHO_TableContainer(): MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >(), XAxisPackType(){};

        MHO_TableContainer(const std::size_t* dim)
            : MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >(dim), XAxisPackType(dim){};

        //copy constructor
        MHO_TableContainer(const MHO_TableContainer& obj)
            : MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >(obj), XAxisPackType(obj), MHO_Taggable(obj){};

        //clone entire table, contents, axes and all
        MHO_TableContainer* Clone() { return new MHO_TableContainer(*this); }

        //clone table shape, but leave contents/axes empty
        MHO_TableContainer* CloneEmpty() { return new MHO_TableContainer(this->GetDimensions()); }

        virtual ~MHO_TableContainer(){};

        virtual MHO_ClassVersion GetVersion() const override { return 0; };

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            total_size += XAxisPackType::NAXES::value * sizeof(uint64_t);
            total_size += XAxisPackType::GetSerializedSize();
            total_size += MHO_Taggable::GetSerializedSize();
            total_size += (this->GetSize()) * sizeof(XValueType);
            return total_size;
        }

        //modify the Resize function to also resize the axes
        using XAxisPackType::resize_axis_pack;

        virtual void Resize(const std::size_t* dim) override
        {
            MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::Resize(dim);
            resize_axis_pack(dim);
        }

        //access to axis pack type alone
        XAxisPackType* GetAxisPack() { return this; }

        const XAxisPackType* GetAxisPack() const { return this; }

        //have to make base class functions visible
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::Resize;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetData;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetSize;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetDimensions;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetDimension;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetDimensionArray;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetOffsetForIndices;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::GetIndicesForOffset;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::SubView;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::SliceView;

        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::at;
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::operator();
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::operator[];
        using MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::ValueAt;

        //expensive copy (as opposed to the assignment operator,
        //pointers to exernally managed memory are not transferred)
        virtual void Copy(const MHO_TableContainer& rhs)
        {
            if(&rhs != this)
            {
                //copy the array
                MHO_NDArrayWrapper< XValueType, XAxisPackType::NAXES::value >::Copy(rhs);
                //copy the axis pack
                *(this->GetAxisPack()) = *(rhs.GetAxisPack());
                //finally copy the table tags
                this->CopyTags(rhs);
            }
        }

    public:
        template< typename XStream > friend XStream& operator>>(XStream& s, MHO_TableContainer& aData)
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

        template< typename XStream > friend XStream& operator<<(XStream& s, const MHO_TableContainer& aData)
        {
            switch(aData.GetVersion())
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                    break;
                default:
                    msg_error("containers", "error, cannot stream out MHO_TableContainer object with unknown version: "
                                                << aData.GetVersion() << eom);
            }
            return s;
        }

    private:
        template< typename XStream > void StreamOutData_V0(XStream& s) const
        {
            //first stream version and dimensions
            s << static_cast< const MHO_Taggable& >(*this);
            auto dims = this->GetDimensionArray();
            for(size_t i = 0; i < XAxisPackType::NAXES::value; i++)
            {
                s << (uint64_t)dims[i];
            }
            //then dump axes
            s << static_cast< const XAxisPackType& >(*this);

            //finally dump the array data
            std::size_t dsize = this->GetSize();
            auto data_ptr = this->GetData();
            for(size_t i = 0; i < dsize; i++)
            {
                s << data_ptr[i];
            }
        }

        template< typename XStream > void StreamInData_V0(XStream& s)
        {
            s >> static_cast< MHO_Taggable& >(*this);

            //next stream the axis-pack
            uint64_t tmp_dim;
            std::size_t dims[XAxisPackType::NAXES::value];
            for(size_t i = 0; i < XAxisPackType::NAXES::value; i++)
            {
                s >> tmp_dim;
                dims[i] = tmp_dim;
            }
            this->Resize(dims);

            //now stream in the axes
            s >> static_cast< XAxisPackType& >(*this);

            //ask for the file stream directly so we can optimize the read in chunks
            //this is not so bueno, breaking ecapsulation of the file streamer class
            auto fs_ptr = dynamic_cast< MHO_FileStreamer* >(&s);
            std::size_t dsize = this->GetSize();
            auto data_ptr = this->GetData();
            if(fs_ptr != nullptr)
            {
                std::fstream& pfile = fs_ptr->GetStream();
                pfile.read(reinterpret_cast< char* >(data_ptr), dsize * sizeof(XValueType));
            }
            else
            {
                //otherwise stream the mult-dim array data generically
                for(size_t i = 0; i < dsize; i++)
                {
                    s >> data_ptr[i];
                }
            }
        }

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

////////////////////////////////////////////////////////////////////////////////
//enumerate all of the table container types we might possibly use, this is not
//an exhaustive list and many other basic (POD)-based types are possible, if they
//are POD-based then additions should be listed here. If there are there are additional
//table container types which involve complicated classes or structs (which would
//would require an '#include' in this file, then they should be defined elsewhere.
//Since we are manily concerned with data (visibility and or cal) we only define
//tables for the following types:

using Int = int;
using Double = double;
using ComplexD = std::complex< double >;
using ComplexF = std::complex< float >;

#define DefTableContainers(TYPE)                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int >;                         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double = MHO_TableContainer< TYPE, MHO_AxisPack_Double >;                   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String = MHO_TableContainer< TYPE, MHO_AxisPack_String >;                   \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int >;                 \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double >;           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String = MHO_TableContainer< TYPE, MHO_AxisPack_Int_String >;           \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int >;           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double = MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double >;     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String = MHO_TableContainer< TYPE, MHO_AxisPack_Double_String >;     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int = MHO_TableContainer< TYPE, MHO_AxisPack_String_Int >;           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double = MHO_TableContainer< TYPE, MHO_AxisPack_String_Double >;     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String = MHO_TableContainer< TYPE, MHO_AxisPack_String_String >;     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int >;         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double >;   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String >;   \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int >;   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String >;                                                            \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int >;   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_Int_AxisPack_String_String =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String >;                                                            \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int >;   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String >;                                                            \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double >;                                                         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String >;                                                         \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double >;                                                         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String >;                                                         \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int = MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int >;   \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String >;                                                            \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double >;                                                         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String >;                                                         \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int >;                                                            \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double >;                                                         \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String =                                                      \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String >;                                                         \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_Int = MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_Int >; \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_Double =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_Double >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Int_String =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Int_String >;                                                           \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_Double_String =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_Double_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Int_String_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String =                                                         \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Int_String_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Int_String =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Int_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_Double_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_Double_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_Double_String_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_Double_String_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Int_String =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Int_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_Double_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_Double_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Int_String_String_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Int_String_String_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Int_String =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Int_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_Double_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_Double_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Int_String_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Int_String_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Int_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Int_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_Double_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_Double_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_Double_String_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_Double_String_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Int_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Int_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_Double_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_Double_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_Double_String_String_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_Double_String_String_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_Int =                                                        \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_Int >;                                                           \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_Double =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_Double >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Int_String =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Int_String >;                                                        \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_Double_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_Double_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Int_String_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Int_String_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Int_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Int_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_Double_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_Double_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_Double_String_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_Double_String_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_Int =                                                     \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_Int >;                                                        \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_Double =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_Double >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Int_String =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Int_String >;                                                     \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_Double_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_Double_String >;                                                  \
                                                                                                                               \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_Int =                                                  \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_Int >;                                                     \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_Double =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_Double >;                                                  \
    using MHO_TableContainer_##TYPE##_MHO_AxisPack_String_String_String_String =                                               \
        MHO_TableContainer< TYPE, MHO_AxisPack_String_String_String_String >;

DefTableContainers(Int);
DefTableContainers(Double);
DefTableContainers(ComplexD);
DefTableContainers(ComplexF);

} // namespace hops

#endif /*! end of include guard: MHO_TableContainer_HH__ */
