#ifndef MHO_SelfName_HH__
#define MHO_SelfName_HH__

#include <atomic>
#include <cstdint>
#include <thread>

namespace hops
{

namespace selfname
{
/*!*
 *@file MHO_SelfName.hh
 *@brief constexpr to stip path prefix from __FILE__ macros
 *@author J. Barrett - barrettj@mit.edu
 */

//needed for stripping the path prefix from __FILE__ macro contents
//see https://stackoverflow.com/questions/31050113
constexpr const char* str_end(const char* str)
{
    return *str ? str_end(str + 1) : str;
}

/**
 * @brief Checks if a string starts with '/'. Used for stripping path prefix from __FILE__.
 * 
 * @param str Input string to check
 * @return True if string starts with '/', false otherwise
 */
constexpr bool str_slash(const char* str)
{
    return *str == '/' ? true : (*str ? str_slash(str + 1) : false);
}

/**
 * @brief Checks if a string starts with '/', recursively.
 * 
 * @param str Input string to check.
 * @return True if string starts with '/', false otherwise.
 */
constexpr const char* r_slash(const char* str)
{
    return *str == '/' ? (str + 1) : r_slash(str - 1);
}

/**
 * @brief Returns the base name (file name without path) from a given string.
 * 
 * @param str Input string containing file path and name.
 * @return constexpr char* representing the base name of the input string.
 */
constexpr const char* file_basename(const char* str)
{
    return str_slash(str) ? r_slash(str_end(str)) : str;
}

} // namespace selfname

} // namespace hops

#endif /*! end of include guard: MHO_SelfName_HH__ */
