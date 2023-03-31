//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   str_comparatos.h
/// @author Petr Vanek

#pragma once

#include <algorithm>
#include <inttypes.h>
#include <string>
#include <string_view>
#include <typeinfo>


template<typename T>
struct EqComparator {
    EqComparator( const std::locale& loc ) : _loc(loc) {}
    bool operator()(T ch1, T ch2) {
        return std::toupper(ch1, _loc) == std::toupper(ch2, _loc);
    }
private:
    const std::locale& _loc;
};


template<typename T>
int findInsensitiveStr(const T& str1, const T& str2, const std::locale& loc = std::locale()) 
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(), 
        str2.begin(), str2.end(), EqComparator<typename T::value_type>(loc) );
    if ( it != str1.end() ) {
        return it - str1.begin();
    } 
     
    return T::npos; 
}

template<typename T>
bool compareInsensitiveStr(const T& str1, const T& str2, const std::locale& loc = std::locale())
{
    auto rc = std::equal( str1.begin(), str1.end(), 
        str2.begin(), str2.end(), EqComparator<typename T::value_type>(loc) );
       
    return rc; 
}
