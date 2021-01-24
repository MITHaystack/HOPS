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
class MHO_AxisPack:  public std::tuple< XAxisTypeS... >
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

    protected:

        //inductive access to all elements of the tuple, so we can re-size them from an array
        template<std::size_t N = 0>
        typename std::enable_if< N == sizeof...(XAxisTypeS), void >::type
        resize_axis_pack( const std::size_t* /*dim*/)
        {
            //terminating case, do nothing
        }

        template<std::size_t N = 0>
        typename std::enable_if< N < sizeof...(XAxisTypeS), void>::type
        resize_axis_pack(const std::size_t* dim)
        {
            //resize the N-th element of the tuple
            std::get<N>(*this).Resize(dim[N]);
            //now run the induction
            resize_axis_pack<N + 1>(dim);
        }

};

}

#endif /* end of include guard: MHO_AxisPack */
