#include <string>
#include <iostream>

#ifndef MHO_TemplateTypenameDeduction_HH__
#define MHO_TemplateTypenameDeduction_HH__

/*
*File: SYMBOL.hh
*Class: SYMBOL
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
* Main idea from blog post here: https://bitwizeshift.github.io/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/
* but re-worked so that it can work with C++11, However, class name deduction is now done at run-time rather than compile time.
*/

template <typename T>
std::string compiler_func_prefix()
{
#if defined(__clang__)
  constexpr const char* compiler_func_prefix   = "[T = ";
#elif defined(__GNUC__)
  constexpr const char* compiler_func_prefix   = "with T = ";
#elif defined(_MSC_VER)
  constexpr const char* compiler_func_prefix   = "type_name_array<";
#else
# error Unsupported compiler
#endif
    return std::string(compiler_func_prefix);
}

template <typename T>
std::string compiler_func_suffix()
{
#if defined(__clang__)
  constexpr const char* compiler_func_suffix   = "]";
#elif defined(__GNUC__)
  constexpr const char* compiler_func_suffix   = ";";
#elif defined(_MSC_VER)
  constexpr const char* compiler_func_suffix   = ">(void)";
#else
# error Unsupported compiler
#endif
    return std::string(compiler_func_suffix);
}


template <typename T>
std::string compiler_func_function()
{
#if defined(__clang__)
  constexpr const char* function = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
  constexpr const char* function = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
  constexpr const char* function = __FUNCSIG__;
#else
# error Unsupported compiler
#endif
    return std::string(function);
}


//template class to carry around static class name
template<typename XClassType>
std::string MHO_ClassName()
{
    std::string prefix = compiler_func_prefix<XClassType>();
    std::string suffix = compiler_func_suffix<XClassType>();
    std::string function = compiler_func_function<XClassType>();

    std::size_t start = function.find(prefix) + prefix.size();
    std::size_t end = function.rfind(suffix);
    std::string name = function.substr(start, (end - start));

    return name;
};


#endif /* end of include guard:  */
