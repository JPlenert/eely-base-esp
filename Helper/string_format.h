// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef StringFormat_h
#define StringFormat_h

#include <string>

#include <memory>
#include <stdexcept>

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#endif


