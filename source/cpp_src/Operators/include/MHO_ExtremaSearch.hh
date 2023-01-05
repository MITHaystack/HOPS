#ifndef MHO_ExtremaSearch_HH__
#define MHO_ExtremaSearch_HH__

#include <algorithm>
#include <cstdint>
#include <limits>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

/*
*File: MHO_ExtremaSearch.hh
*Class: MHO_ExtremaSearch
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:

*/


namespace hops
{

template< class XArgType>
class MHO_ExtremaSearch:
    public MHO_UnaryOperator< XArgType >
{
    public:

        MHO_ExtremaSearch(){};
        virtual ~MHO_ExtremaSearch(){};

        double GetMax(){return fMax;}
        double GetMin(){return fMin;}

        std::size_t GetMaxLocation(){return fMaxLocation;}
        std::size_t GetMinLocation(){return fMinLocation;}

    protected:

        virtual bool InitializeInPlace(XArgType* /*in*/) override {return true;};

        virtual bool ExecuteInPlace(XArgType* in) override
        {
            Search(in);
            return true;
        }

        virtual bool InitializeOutOfPlace(const XArgType* /*in*/, XArgType* /*out*/) override {return true;};

        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* /*out*/) override
        {
            msg_warn("operators", "MHO_ExtremaSearch was passed an additional (unused) argument. " << eom );
            return ExecuteInPlace(in);
        }

    private:


        // basic floating point types
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< std::is_floating_point<XCheckType>::value, void >::type
        Search(XArgType* in)
        {   
            using fp_value_type = typename XArgType::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits<fp_value_type>::min();
            fMin = std::numeric_limits<fp_value_type>::max();
            std::size_t N = in->GetSize(); 
            fp_value_type value;
            for(std::size_t i=0; i<N; i++)
            {
                value = (*in)[i];
                if( value > fMax ){fMax = value; fMaxLocation = i; }
                if( value < fMin ){fMin = value; fMinLocation = i; }
            }
        };

        //specialization for complex types
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< is_complex< XCheckType >::value, void >::type
        Search(XArgType* in)
        {
            using fp_value_type = typename XArgType::value_type::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits<fp_value_type>::min();
            fMin = std::numeric_limits<fp_value_type>::max();
            std::size_t N = in->GetSize();
            fp_value_type value;
            for(std::size_t i=0; i<N; i++)
            {
                value = std::abs( (*in)[i] );
                if( value > fMax ){fMax = value; fMaxLocation = i; }
                if( value < fMin ){fMin = value; fMinLocation = i; }
            }
        };


        double fMax;
        double fMin;
        std::size_t fMaxLocation;
        std::size_t fMinLocation;

};







};




#endif /* MHO_ExtremaSearch_H__ */
