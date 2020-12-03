#ifndef MHOAxisPack_HH__
#define MHOAxisPack_HH__

/*
*File: MHOAxisPack.hh
*Class: MHOAxisPack
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 17-08-2020 11:32
*Description: set of axes (XAxisTypeS are expected to be MHOVectorContainers)
*/

#include "MHOMeta.hh"

namespace hops{

template< typename...XAxisTypeS >
class MHOAxisPack:  public std::tuple< XAxisTypeS... >
{
    public:

        MHOAxisPack():
            std::tuple< XAxisTypeS... >()
        {};

        MHOAxisPack(const std::size_t* dim):
            std::tuple< XAxisTypeS... >()
        {
            resize_axis_pack(dim);
        };

        //copy constructor
        MHOAxisPack( const MHOAxisPack& obj ):
            std::tuple< XAxisTypeS... >(obj)
        {};


        virtual ~MHOAxisPack(){};

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

#endif /* end of include guard: MHOAxisPack */
