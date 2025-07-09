#ifndef MHO_ExtremaSearch_HH__
#define MHO_ExtremaSearch_HH__

#include <algorithm>
#include <cstdint>
#include <limits>

#include "MHO_InspectingOperator.hh"
#include "MHO_Message.hh"
#include "MHO_Meta.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_ExtremaSearch.hh
 *@class MHO_ExtremaSearch
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 5 09:13:09 2023 -0500
 *@brief
 */

/**
 * @brief Class MHO_ExtremaSearch
 */
template< class XArgType > class MHO_ExtremaSearch: public MHO_InspectingOperator< XArgType >
{
    public:
        MHO_ExtremaSearch(){};
        virtual ~MHO_ExtremaSearch(){};

        /**
         * @brief Getter for maximum value across the array
         * 
         * @return The current maximum value as a double.
         */
        double GetMax() { return fMax; }

        /**
         * @brief Getter for minimum value across the
         * 
         * @return The minimum value as a double.
         */
        double GetMin() { return fMin; }

        /**
         * @brief Getter for max location (offset into the array)
         * 
         * @return std::size_t - Maximum location value
         */
        std::size_t GetMaxLocation() { return fMaxLocation; }

        /**
         * @brief Getter for min location (offset into the array)
         * 
         * @return std::size_t representing the minimum location.
         */
        std::size_t GetMinLocation() { return fMinLocation; }

    protected:
        /**
         * @brief Initializes the operator 
         * 
         * @param !in Pointer to constant XArgType
         * @return True if initialization is successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool InitializeImpl(const XArgType* /*!in*/) override { return true; };

        /**
         * @brief Executes search operation using input argument and returns true.
         * 
         * @param in Input argument of type const XArgType* for search operation.
         * @return Boolean value indicating successful execution.
         * @note This is a virtual function.
         */
        virtual bool ExecuteImpl(const XArgType* in) override
        {
            Search(in);
            return true;
        }

    private:
        // basic floating point types
        /**
         * @brief Function Search - does the actual search, templated on value type
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param in (const XArgType*)
         * @return Return value (typename XCheckType >::value, void >::type)
         */
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< std::is_floating_point< XCheckType >::value, void >::type Search(const XArgType* in)
        {
            using fp_value_type = typename XArgType::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits< fp_value_type >::min();
            fMin = std::numeric_limits< fp_value_type >::max();
            fp_value_type value;
            auto bit = in->cbegin();
            auto eit = in->cend();
            std::size_t i = 0;
            for(auto it = bit; it != eit; it++)
            {
                value = *it;
                if(value > fMax)
                {
                    fMax = value;
                    fMaxLocation = i;
                }
                if(value < fMin)
                {
                    fMin = value;
                    fMinLocation = i;
                }
                i++;
            }
        };

        //specialization for complex types
        /**
         * @brief Function Search
         * 
         * @tparam XCheckType Template parameter XCheckType
         * @param in (const XArgType*)
         * @return Return value (typename is_complex< XCheckType >::value, void >::type)
         */
        template< typename XCheckType = typename XArgType::value_type >
        typename std::enable_if< is_complex< XCheckType >::value, void >::type Search(const XArgType* in)
        {
            using fp_value_type = typename XArgType::value_type::value_type;
            fMaxLocation = 0;
            fMinLocation = 0;
            fMax = std::numeric_limits< fp_value_type >::min();
            fMin = std::numeric_limits< fp_value_type >::max();
            fp_value_type value;
            auto bit = in->cbegin();
            auto eit = in->cend();
            std::size_t i = 0;
            for(auto it = bit; it != eit; it++)
            {
                value = std::abs(*it);
                if(value > fMax)
                {
                    fMax = value;
                    fMaxLocation = i;
                }
                if(value < fMin)
                {
                    fMin = value;
                    fMinLocation = i;
                }
                i++;
            }
        };

        double fMax;
        double fMin;
        std::size_t fMaxLocation;
        std::size_t fMinLocation;
};

}; // namespace hops

#endif /*! MHO_ExtremaSearch_H__ */
