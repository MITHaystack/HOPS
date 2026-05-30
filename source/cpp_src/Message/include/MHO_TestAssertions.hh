#ifndef MHO_TestAssertions_HH__
#define MHO_TestAssertions_HH__

/*!
 *@file MHO_TestAssertions.hh
 *@class MHO_TestAssertions
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jul 15 11:30:15 2021 -0400
 *@brief some run time assertions, and macros for unit tests
 */

#include "MHO_Message.hh"
#include <cmath>
#include <iostream>
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

// The macros below are intended for use inside an int-returning test main():
// on failure they print a diagnostic and "return 1", so the executable's exit
// code reports the failure to ctest, this is intended for use in unit test executabes.

#define REQUIRE(cond)                                                                                                          \
    do                                                                                                                         \
    {                                                                                                                          \
        if(!(cond))                                                                                                            \
        {                                                                                                                      \
            std::cerr << "FAIL: " #cond " @ " << __FILE__ << ":" << __LINE__ << std::endl;                                     \
            return 1;                                                                                                          \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

//require that two streamable values compare equal; prints both operands on failure.
//works for any type with operator== and operator<< (std::string, numeric types, etc.)
#define REQUIRE_EQUAL(a, b)                                                                                                    \
    do                                                                                                                         \
    {                                                                                                                          \
        if(!((a) == (b)))                                                                                                      \
        {                                                                                                                      \
            std::stringstream _hops_sa;                                                                                        \
            std::stringstream _hops_sb;                                                                                        \
            _hops_sa << (a);                                                                                                   \
            _hops_sb << (b);                                                                                                   \
            std::cerr << "FAIL: " #a " == " #b " @ " << __FILE__ << ":" << __LINE__ << " (\"" << _hops_sa.str()                \
                      << "\" vs \"" << _hops_sb.str() << "\")" << std::endl;                                                   \
            return 1;                                                                                                          \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

//require that evaluating the expression throws a std::exception (e.g. via HOPS_THROW)
#define REQUIRE_THROWS(expr)                                                                                                   \
    do                                                                                                                         \
    {                                                                                                                          \
        bool _hops_caught = false;                                                                                             \
        try                                                                                                                    \
        {                                                                                                                      \
            (void)(expr);                                                                                                      \
        }                                                                                                                      \
        catch(const std::exception&)                                                                                           \
        {                                                                                                                      \
            _hops_caught = true;                                                                                               \
        }                                                                                                                      \
        if(!_hops_caught)                                                                                                      \
        {                                                                                                                      \
            std::cerr << "FAIL: expected exception from " #expr " @ " << __FILE__ << ":" << __LINE__ << std::endl;             \
            return 1;                                                                                                          \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

//compare two floating-point values up to an absolute tolerance
#define CHECK_CLOSE(a, b, tol)                                                                                                 \
    do                                                                                                                         \
    {                                                                                                                          \
        if(std::fabs((a) - (b)) > (tol))                                                                                       \
        {                                                                                                                      \
            std::cerr << "FAIL: |" #a " - " #b "| > " #tol " @ " << __FILE__ << ":" << __LINE__ << " (" << (a) << " vs "       \
                      << (b) << ")" << std::endl;                                                                              \
            return 1;                                                                                                          \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

//compare two complex (or other std::abs-able) values up to an absolute tolerance on the magnitude of their difference;
//std::abs is used instead of std::fabs so this works for std::complex as well as real types
#define REQUIRE_CLOSE_CPLX(a, b, tol)                                                                                          \
    do                                                                                                                         \
    {                                                                                                                          \
        if(std::abs((a) - (b)) > (tol))                                                                                        \
        {                                                                                                                      \
            std::cerr << "FAIL: |" #a " - " #b "| > " #tol " @ " << __FILE__ << ":" << __LINE__                                \
                      << " (diff=" << std::abs((a) - (b)) << ")" << std::endl;                                                 \
            return 1;                                                                                                          \
        }                                                                                                                      \
    }                                                                                                                          \
    while(0)

#endif /*! end of include guard: MHO_TestAssertions */
