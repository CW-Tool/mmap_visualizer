#ifndef String_hpp__
#define String_hpp__

#include <string>
#include <cstdio>
#include <type_traits>

template< typename ...Args >
std::string Format( const char * format, Args... args )
{
    size_t bufferSize = std::snprintf( nullptr, 0, format, std::forward< Args >( args )... ) + 1;

    std::string result;
    result.resize( bufferSize );

    bufferSize = std::snprintf( const_cast< char * >( result.c_str() ), result.size(), format, std::forward< Args >( args )... );
    result.resize( bufferSize );

    return result;
}

#endif // String_hpp__
