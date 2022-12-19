#ifndef MHO_AxisPack_HH__
#define MHO_AxisPack_HH__

/*
*File: MHO_AxisPack.hh
*Class: MHO_AxisPack
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 17-08-2020 11:32
*Description: set of axes (XAxisTypeS are expected to be MHO_VectorContainers)
*/

#include "MHO_Axis.hh"
#include "MHO_Meta.hh"

namespace hops{

template< typename...XAxisTypeS >
class MHO_AxisPack:  public std::tuple< XAxisTypeS... >, virtual public MHO_Serializable
{
    public:

        MHO_AxisPack():
            std::tuple< XAxisTypeS... >()
        {};

        MHO_AxisPack(const std::size_t* dim):
            std::tuple< XAxisTypeS... >()
        {
            resize_axis_pack(dim);
        };

        //copy constructor
        MHO_AxisPack( const MHO_AxisPack& obj ):
            std::tuple< XAxisTypeS... >(obj)
        {};


        virtual ~MHO_AxisPack(){};

        //declare some convenience types
        typedef std::integral_constant< std::size_t, sizeof...(XAxisTypeS) > NAXES;
        typedef std::tuple< XAxisTypeS... > axis_pack_tuple_type;

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            compute_total_size(total_size);
            return total_size;
        }

        virtual 

        //assignment operator
        MHO_AxisPack& operator=(const MHO_AxisPack& rhs)
        {
            this->copy(rhs);
            return *this;
        }

    protected:

        //inductive access to all elements of the tuple, so we can re-size them from an array
        template<std::size_t N = 0>
        typename std::enable_if< ( N == sizeof...(XAxisTypeS) ), void >::type
        resize_axis_pack( const std::size_t* /*dim*/){}; //terminating case, do nothing

        template<std::size_t N = 0>
        typename std::enable_if< ( N < sizeof...(XAxisTypeS) ), void>::type
        resize_axis_pack(const std::size_t* dim)
        {
            //resize the N-th element of the tuple
            std::get<N>(*this).Resize(dim[N]);
            //now run the induction
            resize_axis_pack<N + 1>(dim);
        }

        // //inductive access to all elements of the tuple, so we compute total size for streaming
        template<std::size_t N = 0>
        typename std::enable_if< ( N == sizeof...(XAxisTypeS) ), void >::type
        compute_total_size(uint64_t& /*total_size*/) const {}; //terminating case, do nothing

        template<std::size_t N = 0>
        typename std::enable_if< ( N < sizeof...(XAxisTypeS) ), void >::type
        compute_total_size(uint64_t& total_size) const
        {
            total_size += std::get<N>(*this).GetSerializedSize();
            //now run the induction
            compute_total_size<N + 1>(total_size);
        }

        //for copying the full tuple from one axis pack to another
        template<std::size_t N = 0>
        typename std::enable_if< ( N == sizeof...(XAxisTypeS) ), void >::type
        copy(const MHO_AxisPack&) const {}; //terminating case, do nothing

        template<std::size_t N = 0>
        typename std::enable_if< ( N < sizeof...(XAxisTypeS) ), void >::type
        copy(const MHO_AxisPack& rhs)
        {
            std::get<N>(*this).Copy( std::get<N>(rhs) );
            copy<N + 1>(rhs);
        }

    public:


        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_AxisPack& aData)
        {
            s << aData.GetVersion();
            // ostream_tuple<0, XStream, XAxisTypeS... >(s, static_cast< const std::tuple< XAxisTypeS... >& >(aData) );
            ostream_tuple(s, aData);
            return s;
        }

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_AxisPack& aData)
        {
            MHO_ClassVersion vers;
            s >> vers;
            if( vers != aData.GetVersion() )
            {
                MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                //Flag this as an unknown object version so we can skip over this data
                MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            else
            {
                istream_tuple(s, aData);
            }
            return s;
        }

};

////////////////////////////////////////////////////////////////////////////////
//enumerate some of the most commonly used axis-pack types here:
//in order to keep the number of possible axis-pack types from getting out of hand
//we limit the number of axes to <4, and only declare for the following POD types:
//int (Int), double (Double), string (String)
//more types are certainly possible, but they should be declared only as needed

//TODO FIXME -- find away to loop over macro arguments so we dont need so many
//boilerplate definitions


using Int = int;
using Double = double;
using String = std::string;

#define DefAxisPack1(TYPE1)                                                    \
using MHO_AxisPack_##TYPE1 =                                                   \
MHO_AxisPack< MHO_Axis##TYPE1 >;

#define DefAxisPack2(TYPE1, TYPE2)                                             \
using MHO_AxisPack_##TYPE1##_##TYPE2 =                                         \
MHO_AxisPack< MHO_Axis##TYPE1, MHO_Axis##TYPE2 >;

#define DefAxisPack3(TYPE1, TYPE2, TYPE3)                                      \
using MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 =                               \
MHO_AxisPack< MHO_Axis##TYPE1, MHO_Axis##TYPE2, MHO_Axis##TYPE3 >;

#define DefAxisPack4(TYPE1, TYPE2, TYPE3, TYPE4)                               \
using MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 =                     \
MHO_AxisPack< MHO_Axis##TYPE1, MHO_Axis##TYPE2, MHO_Axis##TYPE3, MHO_Axis##TYPE4  >;

////////////////////////////////////////////////////////////////////////////////

DefAxisPack1(Int);
DefAxisPack1(Double);
DefAxisPack1(String);

////////////////////////////////////////////////////////////////////////////////

DefAxisPack2(Int, Int);
DefAxisPack2(Int, Double);
DefAxisPack2(Int, String);
DefAxisPack2(Double, Int);
DefAxisPack2(Double, Double);
DefAxisPack2(Double, String);
DefAxisPack2(String, Int);
DefAxisPack2(String, Double);
DefAxisPack2(String, String);

////////////////////////////////////////////////////////////////////////////////

DefAxisPack3(Int, Int, Int);
DefAxisPack3(Int, Int, Double);
DefAxisPack3(Int, Int, String);
DefAxisPack3(Int, Double, Int);
DefAxisPack3(Int, Double, Double);
DefAxisPack3(Int, Double, String);
DefAxisPack3(Int, String, Int);
DefAxisPack3(Int, String, Double);
DefAxisPack3(Int, String, String);

DefAxisPack3(Double, Int, Int);
DefAxisPack3(Double, Int, Double);
DefAxisPack3(Double, Int, String);
DefAxisPack3(Double, Double, Int);
DefAxisPack3(Double, Double, Double);
DefAxisPack3(Double, Double, String);
DefAxisPack3(Double, String, Int);
DefAxisPack3(Double, String, Double);
DefAxisPack3(Double, String, String);

DefAxisPack3(String, Int, Int);
DefAxisPack3(String, Int, Double);
DefAxisPack3(String, Int, String);
DefAxisPack3(String, Double, Int);
DefAxisPack3(String, Double, Double);
DefAxisPack3(String, Double, String);
DefAxisPack3(String, String, Int);
DefAxisPack3(String, String, Double);
DefAxisPack3(String, String, String);

////////////////////////////////////////////////////////////////////////////////

DefAxisPack4(Int, Int, Int, Int);
DefAxisPack4(Int, Int, Int, Double);
DefAxisPack4(Int, Int, Int, String);
DefAxisPack4(Int, Int, Double, Int);
DefAxisPack4(Int, Int, Double, Double);
DefAxisPack4(Int, Int, Double, String);
DefAxisPack4(Int, Int, String, Int);
DefAxisPack4(Int, Int, String, Double);
DefAxisPack4(Int, Int, String, String);

DefAxisPack4(Int, Double, Int, Int);
DefAxisPack4(Int, Double, Int, Double);
DefAxisPack4(Int, Double, Int, String);
DefAxisPack4(Int, Double, Double, Int);
DefAxisPack4(Int, Double, Double, Double);
DefAxisPack4(Int, Double, Double, String);
DefAxisPack4(Int, Double, String, Int);
DefAxisPack4(Int, Double, String, Double);
DefAxisPack4(Int, Double, String, String);

DefAxisPack4(Int, String, Int, Int);
DefAxisPack4(Int, String, Int, Double);
DefAxisPack4(Int, String, Int, String);
DefAxisPack4(Int, String, Double, Int);
DefAxisPack4(Int, String, Double, Double);
DefAxisPack4(Int, String, Double, String);
DefAxisPack4(Int, String, String, Int);
DefAxisPack4(Int, String, String, Double);
DefAxisPack4(Int, String, String, String);

////////////

DefAxisPack4(Double, Int, Int, Int);
DefAxisPack4(Double, Int, Int, Double);
DefAxisPack4(Double, Int, Int, String);
DefAxisPack4(Double, Int, Double, Int);
DefAxisPack4(Double, Int, Double, Double);
DefAxisPack4(Double, Int, Double, String);
DefAxisPack4(Double, Int, String, Int);
DefAxisPack4(Double, Int, String, Double);
DefAxisPack4(Double, Int, String, String);

DefAxisPack4(Double, Double, Int, Int);
DefAxisPack4(Double, Double, Int, Double);
DefAxisPack4(Double, Double, Int, String);
DefAxisPack4(Double, Double, Double, Int);
DefAxisPack4(Double, Double, Double, Double);
DefAxisPack4(Double, Double, Double, String);
DefAxisPack4(Double, Double, String, Int);
DefAxisPack4(Double, Double, String, Double);
DefAxisPack4(Double, Double, String, String);

DefAxisPack4(Double, String, Int, Int);
DefAxisPack4(Double, String, Int, Double);
DefAxisPack4(Double, String, Int, String);
DefAxisPack4(Double, String, Double, Int);
DefAxisPack4(Double, String, Double, Double);
DefAxisPack4(Double, String, Double, String);
DefAxisPack4(Double, String, String, Int);
DefAxisPack4(Double, String, String, Double);
DefAxisPack4(Double, String, String, String);

////////////

DefAxisPack4(String, Int, Int, Int);
DefAxisPack4(String, Int, Int, Double);
DefAxisPack4(String, Int, Int, String);
DefAxisPack4(String, Int, Double, Int);
DefAxisPack4(String, Int, Double, Double);
DefAxisPack4(String, Int, Double, String);
DefAxisPack4(String, Int, String, Int);
DefAxisPack4(String, Int, String, Double);
DefAxisPack4(String, Int, String, String);

DefAxisPack4(String, Double, Int, Int);
DefAxisPack4(String, Double, Int, Double);
DefAxisPack4(String, Double, Int, String);
DefAxisPack4(String, Double, Double, Int);
DefAxisPack4(String, Double, Double, Double);
DefAxisPack4(String, Double, Double, String);
DefAxisPack4(String, Double, String, Int);
DefAxisPack4(String, Double, String, Double);
DefAxisPack4(String, Double, String, String);

DefAxisPack4(String, String, Int, Int);
DefAxisPack4(String, String, Int, Double);
DefAxisPack4(String, String, Int, String);
DefAxisPack4(String, String, Double, Int);
DefAxisPack4(String, String, Double, Double);
DefAxisPack4(String, String, Double, String);
DefAxisPack4(String, String, String, Int);
DefAxisPack4(String, String, String, Double);
DefAxisPack4(String, String, String, String);


}

#endif /* end of include guard: MHO_AxisPack */
