#ifndef MHO_TestAssertions_HH__
#define MHO_TestAssertions_HH__

/*
*File: MHO_TestAssertions.hh
*Class: MHO_TestAssertions
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <stdexcept>
#include <string>
#include <sstream>

#define HOPS_THROW                                                  \
{                                                                   \
    throw std::runtime_error(   std::string( __FILE__ )             \
                              + std::string( ":" )                  \
                              + std::to_string( __LINE__ )          \
                              + std::string( " in " )               \
                              + std::string( __PRETTY_FUNCTION__ )  \
    );                                                              \
}


#define HOPS_ASSERT_THROW( test_cond )                              \
{                                                                   \
  if( !( test_cond ) )                                              \
  {                                                                 \
    throw std::runtime_error(   std::string( __FILE__ )             \
                              + std::string( ":" )                  \
                              + std::to_string( __LINE__ )          \
                              + std::string( " in " )               \
                              + std::string( __PRETTY_FUNCTION__ )  \
    );                                                              \
  }                                                                 \
}

#define HOPS_ASSERT_EQUAL( a, b )                                   \
{                                                                   \
  if( ( a ) != ( b ) )                                              \
  {                                                                 \
    throw std::runtime_error(   std::string( __FILE__ )             \
                              + std::string( ":" )                  \
                              + std::to_string( __LINE__ )          \
                              + std::string( " in " )               \
                              + std::string( __PRETTY_FUNCTION__ )  \
                              + std::string( ": " )                 \
                              + std::to_string( (a) )               \
                              + std::string( " != " )               \
                              + std::to_string( (b) )               \
    );                                                              \
  }                                                                 \
}

//to_string is unpredictable for floats/doubles, so this instead
#define HOPS_ASSERT_FLOAT_LESS_THAN( a, b )                         \
{                                                                   \
  std::stringstream ssa; ssa << a;                                  \
  std::stringstream ssb; ssb << b;                                  \
  if( ( a ) > ( b ) )                                               \
  {                                                                 \
    throw std::runtime_error(   std::string( __FILE__ )             \
                              + std::string( ":" )                  \
                              + std::to_string( __LINE__ )          \
                              + std::string( " in " )               \
                              + std::string( __PRETTY_FUNCTION__ )  \
                              + std::string( ": " )                 \
                              + ssa.str()                           \
                              + std::string( " !< " )               \
                              + ssb.str()                           \
    );                                                              \
  }                                                                 \
}


#endif /* end of include guard: MHO_TestAssertions */
