#ifndef mho_string_HH__
#define mho_string_HH__

/*
*@file: mho_string.hh
*@class: mho_string
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: 
* This class is a strong alias to std::string, it's ONLY purpose is to
* allow the template typename deduction mechanism to return the value 
* "mho_string" as a class name instead of the ugly:
* std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >
*/

#include <string>

namespace hops{

class mho_string: public std::string 
{
    using std::string::string;

    using std::string::value_type;	
    using std::string::traits_type;
    using std::string::allocator_type;
    using std::string::reference;
    using std::string::const_reference;
    using std::string::pointer;
    using std::string::const_pointer;
    using std::string::iterator;
    using std::string::const_iterator;
    using std::string::reverse_iterator;
    using std::string::const_reverse_iterator;
    using std::string::difference_type;
    using std::string::size_type;

    using std::string::begin;
    using std::string::end;
    using std::string::rbegin;
    using std::string::rend;
    using std::string::cbegin;
    using std::string::cend;
    using std::string::crbegin;
    using std::string::crend;

    //using std::string::operator =;


    using std::string::size;
    using std::string::length;
    using std::string::max_size;
    using std::string::resize;
    using std::string::capacity;
    using std::string::reserve;
    using std::string::clear;
    using std::string::empty;
    using std::string::shrink_to_fit;

    using std::string::operator[];
    using std::string::at;
    using std::string::back;
    using std::string::front;

    using std::string::operator+=;
    using std::string::append;
    using std::string::push_back;
    using std::string::assign;
    using std::string::insert;
    using std::string::erase;
    using std::string::replace;
    using std::string::swap;
    using std::string::pop_back;

    using std::string::c_str;
    using std::string::data;
    using std::string::get_allocator;
    using std::string::copy;
    using std::string::find;
    using std::string::rfind;
    using std::string::find_first_of;
    using std::string::find_last_of;
    using std::string::find_first_not_of;
    using std::string::find_last_not_of;
    using std::string::substr;
    using std::string::compare;

    using std::string::npos;
};

}

#endif /* end of include guard: mho_string */