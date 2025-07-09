#ifndef MHO_TestAssertions_HH__
#define MHO_TestAssertions_HH__

/*!
 *@file MHO_TestAssertions.hh
 *@class MHO_TestAssertions
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jul 15 11:30:15 2021 -0400
 *@brief some run time assertions
 */

#include "MHO_Message.hh"
#include <sstream>
#include <stdexcept>
#include <string>

#define HOPS_THROW                                                                                                             \
    {                                                                                                                          \
        throw std::runtime_error(std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) + std::string(" in ") +   \
                                 std::string(__PRETTY_FUNCTION__));                                                            \
    }

#define HOPS_ASSERT_THROW(test_cond)                                                                                           \
    {                                                                                                                          \
        if(!(test_cond))                                                                                                       \
        {                                                                                                                      \
            throw std::runtime_error(std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) +                     \
                                     std::string(" in ") + std::string(__PRETTY_FUNCTION__));                                  \
        }                                                                                                                      \
    }

#define HOPS_ASSERT_EQUAL(a, b)                                                                                                \
    {                                                                                                                          \
        if((a) != (b))                                                                                                         \
        {                                                                                                                      \
            throw std::runtime_error(std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) +                     \
                                     std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::string(": ") +              \
                                     std::to_string((a)) + std::string(" != ") + std::to_string((b)));                         \
        }                                                                                                                      \
    }

//to_string is unpredictable for floats/doubles, so this instead
#define HOPS_ASSERT_FLOAT_LESS_THAN(a, b)                                                                                      \
    {                                                                                                                          \
        std::stringstream ssa;                                                                                                 \
        ssa << a;                                                                                                              \
        std::stringstream ssb;                                                                                                 \
        ssb << b;                                                                                                              \
        if((a) > (b))                                                                                                          \
        {                                                                                                                      \
            throw std::runtime_error(std::string(__FILE__) + std::string(":") + std::to_string(__LINE__) +                     \
                                     std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::string(": ") + ssa.str() +  \
                                     std::string(" !< ") + ssb.str());                                                         \
        }                                                                                                                      \
    }

#endif /*! end of include guard: MHO_TestAssertions */
