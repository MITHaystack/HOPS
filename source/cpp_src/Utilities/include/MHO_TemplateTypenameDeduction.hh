#include <string>
#include <regex>
#include <tuple>
#include <iostream>


#ifndef MHO_TemplateTypenameDeduction_HH__
#define MHO_TemplateTypenameDeduction_HH__

/*
*File: MHO_TemplateTypenameDeduction.hh
*Class:
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
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
template <typename T>
std::string compiler_func_prefix()
{
#if defined(__clang__)
  constexpr const char* compiler_func_prefix   = "[T = ";
#elif defined(__GNUC__)
  constexpr const char* compiler_func_prefix   = "with T = ";
#else
    # error Unsupported compiler
#endif
    return std::string(compiler_func_prefix);
}

//these constants are needed to locate the end of the class name within a template decl.
template <typename T>
std::string compiler_func_suffix()
{
#if defined(__clang__)
  constexpr const char* compiler_func_suffix   = "]";
#elif defined(__GNUC__)
  constexpr const char* compiler_func_suffix   = ";";
#else
    # error Unsupported compiler
#endif
    return std::string(compiler_func_suffix);
}

//the constants refer to the compiler macros which print out a function's full name
template <typename T>
std::string compiler_func_function()
{
#if defined(__clang__)
  constexpr const char* function = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
  constexpr const char* function = __PRETTY_FUNCTION__;
#else
# error Unsupported compiler
#endif
    return std::string(function);
}

//does the bare minimum processing on the compiler macro output to strip out a name for XClassType
template<typename XClassType>
std::string MHO_RawCompilerName()
{
    std::string prefix = compiler_func_prefix<XClassType>();
    std::string suffix = compiler_func_suffix<XClassType>();
    std::string function = compiler_func_function<XClassType>();
    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string class_name = function.substr(start, (end - start));
    return class_name;
};

//same as MHO_RawCompilerName, except space characters are stripped out by regex
template<typename XClassType>
std::string MHO_RawCompilerNameWithoutSpaces()
{
    std::string prefix = compiler_func_prefix<XClassType>();
    std::string suffix = compiler_func_suffix<XClassType>();
    std::string function = compiler_func_function<XClassType>();
    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string class_name = function.substr(start, (end - start));
    std::string space = " ";
    std::string nothing = "";
    //strip out all of the spaces in the class name
    std::string tmp = std::regex_replace(class_name,
                                          std::regex(space),
                                          nothing);
    return tmp;
};

//extracts the name of a class when used within a std::tuple<>
template<typename XClassType>
std::string MHO_TupleElementNameWithoutSpaces()
{
    //this is needed because GCC represents a std::string when used within
    //a tuple differently (adding hidden default template parameters)
    //than it does in an arbitrary template (why??)

    std::string prefix = compiler_func_prefix< std::tuple<XClassType> >();
    std::string suffix = compiler_func_suffix< std::tuple<XClassType> >();
    std::string function = compiler_func_function< std::tuple<XClassType> >();

    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string tuple_class_name = function.substr(start, (end - start));

    std::string space = " ";
    std::string nothing = "";
    //strip out all of the spaces in the class name
    std::string tmp = std::regex_replace(tuple_class_name,
                                          std::regex(space),
                                          nothing);

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
template<typename XClassType>
std::string MHO_ClassName()
{
    std::string class_name = MHO_RawCompilerNameWithoutSpaces<XClassType>();
    std::string string_tuple_name = MHO_TupleElementNameWithoutSpaces<std::string>();
    std::string string_name = MHO_RawCompilerNameWithoutSpaces<std::string>();
    std::string new_string_name = "string"; //could use std::string, but that is too verbose

    //first strip out the any tuple dependent std::string name, and replace with std::string
    std::string tmp = class_name;
    std::size_t strname_loc = std::string::npos;
    do
    {
        strname_loc = tmp.find(string_tuple_name);
        if(strname_loc != std::string::npos)
        {
          tmp.replace(strname_loc,string_tuple_name.length(), new_string_name);
        }
    }
    while(strname_loc != std::string::npos );

    //next strip out any non-tuple dependent std::string names and replace with std::string
    strname_loc = std::string::npos;
    do
    {
        strname_loc = tmp.find(string_name);
        if(strname_loc != std::string::npos)
        {
            tmp.replace(strname_loc,string_name.length(),new_string_name);
        }
    }
    while(strname_loc != std::string::npos );

    //finally, lets also get rid of the hops:: namespace prefix on all the class names
    std::string hops_nmspc = "hops::";
    std::string nothing = "";
    std::string name = std::regex_replace(tmp,
                                          std::regex(hops_nmspc),
                                          nothing);

    return name;
};


//specialization for std::string to keep things from
//getting really unwieldly, aka:
// std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >
template<>
inline std::string MHO_ClassName<std::string>()
{
    std::string name = "std::string";
    return name;
};


#endif /* end of include guard:  */
