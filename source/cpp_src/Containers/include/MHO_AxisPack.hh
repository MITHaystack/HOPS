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

        typedef std::integral_constant< std::size_t, sizeof...(XAxisTypeS) > NAXES;

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion);
            compute_total_size(total_size);
            return total_size;
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

}

#endif /* end of include guard: MHO_AxisPack */
