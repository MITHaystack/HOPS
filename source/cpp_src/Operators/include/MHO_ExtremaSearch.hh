#ifndef MHO_ExtremaSearch_HH__
#define MHO_ExtremaSearch_HH__



#include <algorithm>
#include <cstdint>
#include <limits>

#include "MHO_Meta.hh"
#include "MHO_Message.hh"
#include "MHO_InspectingOperator.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"


namespace hops
{

/*!
*@file MHO_ExtremaSearch.hh
*@class MHO_ExtremaSearch
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

template< class XArgType>
class MHO_ExtremaSearch:
    public MHO_InspectingOperator< XArgType >
{
    public:

        MHO_ExtremaSearch(){};
        virtual ~MHO_ExtremaSearch(){};

        double GetMax(){return fMax;}
        double GetMin(){return fMin;}

        std::size_t GetMaxLocation(){return fMaxLocation;}
        std::size_t GetMinLocation(){return fMinLocation;}

    protected:

        virtual bool InitializeImpl(const XArgType* /*!in*/) override {return true;};
        virtual bool ExecuteImpl(const XArgType* in) override { Search(in); return true;}

    private:

        // basic floating point types
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< std::is_floating_point<XCheckType>::value, void >::type
        Search(const XArgType* in)
        {
            using fp_value_type = typename XArgType::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits<fp_value_type>::min();
            fMin = std::numeric_limits<fp_value_type>::max();
            fp_value_type value;
            auto bit = in->cbegin();
            auto eit = in->cend();
            std::size_t i=0;
            for(auto it = bit; it != eit; it++)
            {
                value = *it;
                if( value > fMax ){fMax = value; fMaxLocation = i; }
                if( value < fMin ){fMin = value; fMinLocation = i; }
                i++;
            }
        };

        //specialization for complex types
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< is_complex< XCheckType >::value, void >::type
        Search(const XArgType* in)
        {
            using fp_value_type = typename XArgType::value_type::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits<fp_value_type>::min();
            fMin = std::numeric_limits<fp_value_type>::max();
            fp_value_type value;
            auto bit = in->cbegin();
            auto eit = in->cend();
            std::size_t i=0;
            for(auto it = bit; it != eit; it++)
            {
                value = std::abs(*it);
                if( value > fMax ){fMax = value; fMaxLocation = i; }
                if( value < fMin ){fMin = value; fMinLocation = i; }
                i++;
            }
        };


        double fMax;
        double fMin;
        std::size_t fMaxLocation;
        std::size_t fMinLocation;

};







};




#endif /*! MHO_ExtremaSearch_H__ */
