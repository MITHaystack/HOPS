#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>

#ifndef MHO_TemplateTypenameDeduction_HH__
    #define MHO_TemplateTypenameDeduction_HH__

/*!
*@file MHO_TemplateTypenameDeduction.hh
*@class
*@author J. Barrett - barrettj@mit.edu
*@date Wed Apr 21 13:40:18 2021 -0400
*@brief
* Main idea from blog post here: https://bitwizeshift.github.io/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/
* but re-worked so that it can work with C++11.
* However, class name deduction is now done at run-time rather than compile time, because the compiler names
* are very ugly (especially when std::string is involved). We also need to deal with the fact that the exact
* class name string can be different depending on compiler. Especially in the special case of std::string,
* where the compiler name for this class can depend on whether it has been used inside of a std::tuple.
* For example: the program TestTemplateTypenameDeduction outputs the following for clang and gcc (note the difference
* in the second print-out, which is the case where std::string has been used as the argument of a tuple). In this
* case clang is actually pretty well behaved (hiding the default template arguments in both cases), but GCC decides
* switch it up). Much of the code below is in order to catch this and covert both ugly statments into plain old "std::string".

    The compiler is: clang.
     MHO_RawCompilerName< std::string >() = std::__cxx11::basic_string<char>
     MHO_TupleElementNameWithoutSpaces< std::string >() = std::__cxx11::basic_string<char>
     MHO_ClassName<std::string>()  = std::string
     MHO_ClassName< std::tuple< std::string > >()  = std::tuple<std::string>

    The compiler is: GCC.
     MHO_RawCompilerName< std::string >() = std::__cxx11::basic_string<char>
     MHO_TupleElementNameWithoutSpaces< std::string >() = std::__cxx11::basic_string<char,std::char_traits<char>,std::allocator<char>>
     MHO_ClassName<std::string>()  = std::string
     MHO_ClassName< std::tuple< std::string > >()  = std::tuple<std::string>
*/

//these constants are needed to locate the class name within a template decl.
/**
 * @brief Returns a compiler-specific prefix string for template type T.
 *
 * @return std::string containing compiler-specific prefix for template type T.
 */
template< typename T > std::string compiler_func_prefix()
{
    #if defined(__clang__)
    constexpr const char* compiler_func_prefix = "[T = ";
    #elif defined(__GNUC__)
    constexpr const char* compiler_func_prefix = "with T = ";
    #else
        #error Unsupported compiler
    #endif
    return std::string(compiler_func_prefix);
}

//these constants are needed to locate the end of the class name within a template decl.
/**
 * @brief Returns a string suffix for function names based on compiler type.
 *
 * @return std::string containing either "]" (for Clang) or ";" (for GCC).
 */
template< typename T > std::string compiler_func_suffix()
{
    #if defined(__clang__)
    constexpr const char* compiler_func_suffix = "]";
    #elif defined(__GNUC__)
    constexpr const char* compiler_func_suffix = ";";
    #else
        #error Unsupported compiler
    #endif
    return std::string(compiler_func_suffix);
}

//the constants refer to the compiler macros which print out a function's full name
/**
 * @brief Returns a string containing the full name of the current function using compiler-specific macros.
 *
 * @return std::string containing the full name of the current function.
 */
template< typename T > std::string compiler_func_function()
{
    #if defined(__clang__)
    constexpr const char* function = __PRETTY_FUNCTION__;
    #elif defined(__GNUC__)
    constexpr const char* function = __PRETTY_FUNCTION__;
    #else
        #error Unsupported compiler
    #endif
    return std::string(function);
}

//does the bare minimum processing on the compiler macro output to strip out a name for XClassType
/**
 * @brief Returns a string representation of the compiler name for given template type XClassType.
 *
 * @return std::string containing the compiler name prefixed by type information
 */
template< typename XClassType > std::string MHO_RawCompilerName()
{
    std::string prefix = compiler_func_prefix< XClassType >();
    std::string suffix = compiler_func_suffix< XClassType >();
    std::string function = compiler_func_function< XClassType >();
    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string class_name = function.substr(start, (end - start));
    return class_name;
};

//same as MHO_RawCompilerName, except space characters are stripped out by regex
/**
 * @brief Extracts and strips spaces from class name in compiler function for given XClassType.
 *
 * @return String containing class name without spaces
 */
template< typename XClassType > std::string MHO_RawCompilerNameWithoutSpaces()
{
    std::string prefix = compiler_func_prefix< XClassType >();
    std::string suffix = compiler_func_suffix< XClassType >();
    std::string function = compiler_func_function< XClassType >();
    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string class_name = function.substr(start, (end - start));
    //remove spaces
    class_name.erase(std::remove(class_name.begin(), class_name.end(), ' '), class_name.end());
    return class_name;
};

//extracts the name of a class when used within a std::tuple<>
/**
 * @brief Extracts and returns the name of a class when used within a std::tuple<.
 *
 * @return std::string containing the class name without spaces
 */
template< typename XClassType > std::string MHO_TupleElementNameWithoutSpaces()
{
    //this is needed because GCC represents a std::string when used within
    //a tuple differently (adding hidden default template parameters)
    //than it does in an arbitrary template (why??)

    std::string prefix = compiler_func_prefix< std::tuple< XClassType > >();
    std::string suffix = compiler_func_suffix< std::tuple< XClassType > >();
    std::string function = compiler_func_function< std::tuple< XClassType > >();

    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string tuple_class_name = function.substr(start, (end - start));

    //strip out all of the spaces in the class name
    tuple_class_name.erase(std::remove(tuple_class_name.begin(), tuple_class_name.end(), ' '), tuple_class_name.end());
    std::string tmp = tuple_class_name;

    std::string tuple_prefix = "std::tuple<";
    std::string tuple_suffix = ">";

    start = tmp.find(tuple_prefix) + tuple_prefix.size();
    end = tmp.rfind(tuple_suffix);
    std::string class_name = tmp.substr(start, (end - start));

    return class_name;
};

//this template class is what we use to determine the name of class throughout the
//rest of the code, it does some string processing to make sure std::string
//comes out with a sensible name whenever it appears
/**
 * @brief Determines and returns the name of class XClassType as a string for postprocessing in MHO.
 *
 * @return std::string containing the name of class XClassType
 */
template< typename XClassType > std::string MHO_ClassName()
{
    std::string class_name = MHO_RawCompilerNameWithoutSpaces< XClassType >();
    std::string string_tuple_name = MHO_TupleElementNameWithoutSpaces< std::string >();
    std::string string_name = MHO_RawCompilerNameWithoutSpaces< std::string >();
    std::string new_string_name = "string"; //could use std::string, but that is too verbose

    //first strip out the any tuple dependent std::string name, and replace with std::string
    std::string tmp = class_name;
    std::size_t strname_loc = std::string::npos;
    do
    {
        strname_loc = tmp.find(string_tuple_name);
        if(strname_loc != std::string::npos)
        {
            tmp.replace(strname_loc, string_tuple_name.length(), new_string_name);
        }
    }
    while(strname_loc != std::string::npos);

    //next strip out any non-tuple dependent std::string names and replace with std::string
    strname_loc = std::string::npos;
    do
    {
        strname_loc = tmp.find(string_name);
        if(strname_loc != std::string::npos)
        {
            tmp.replace(strname_loc, string_name.length(), new_string_name);
        }
    }
    while(strname_loc != std::string::npos);

    //finally, lets also get rid of the hops:: namespace prefix on all the class names
    std::string hops_nmspc = "hops::";
    size_t pos;
    while((pos = tmp.find(hops_nmspc)) != std::string::npos)
    {
        tmp.erase(pos, hops_nmspc.length());
    }
    std::string name = tmp;

    return name;
};

//specialization for std::string to keep things from
//getting really unwieldly, aka:
// std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >
/**
 * @brief Specialization for std::string to return class name prefix.
 *
 * @return std::string containing compiler-specific prefix for class name.
 */
template<> inline std::string MHO_ClassName< std::string >()
{
    std::string name = "std::string";
    return name;
};

#endif /*! end of include guard:  */
