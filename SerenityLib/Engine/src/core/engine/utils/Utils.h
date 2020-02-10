#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H



#if _WIN32 || _WIN64
    #if _WIN64
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif

#if __GNUC__
    #if __x86_64__ || __ppc64__
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif

#ifdef ENVIRONMENT64

#else
#endif
#include <core/engine/math/Numbers.h>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

typedef std::uint32_t   uint;

typedef unsigned char   uchar;
typedef unsigned short  ushort;

typedef std::uint64_t   uint64;
typedef std::uint32_t   uint32;

template <typename Stream> void readUint32tBigEndian(std::uint32_t& out, Stream& stream) {
    std::uint8_t buf[4];
    stream.read((char *)buf, 4);
    out  = (std::uint32_t)buf[0] << 24;
    out |= (std::uint32_t)buf[1] << 16;
    out |= (std::uint32_t)buf[2] << 8;
    out |= (std::uint32_t)buf[3];
}
template <typename Stream> void readUint16tBigEndian(std::uint16_t& out, Stream& stream) {
    std::uint8_t buf[2];
    stream.read((char *)buf, 2);
    out  = (std::uint32_t)buf[0] << 8;
    out |= (std::uint32_t)buf[1];
}
template <typename Stream> void writeUint32tBigEndian(std::uint32_t& in, Stream& stream) {
    std::uint8_t buf[4];
    buf[0] = (in & 0xff000000) >> 24;
    buf[1] = (in & 0x00ff0000) >> 16;
    buf[2] = (in & 0x0000ff00) >> 8;
    buf[3] = (in & 0x000000ff);
    stream.write((char*)buf, sizeof(buf));
}
template <typename Stream> void writeUint16tBigEndian(std::uint16_t& in, Stream& stream) {
    std::uint8_t buf[2];
    buf[0] = (in & 0xff00) >> 8;
    buf[1] = (in & 0x00ff);
    stream.write((char*)buf, sizeof(buf));
}

//specifies if a specific pointer element is in a vector
template<typename E, typename B> bool isInVector(std::vector<B*>& v, E* e) {
    for (auto& item : v) {
        if (item == e)
            return true;
    }
    return false;
}

//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B*>& v, const E* e){
    v.erase(std::remove(v.begin(), v.end(), e), v.end());
}
//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B>& v,const E& e){
    v.erase(std::remove(v.begin(), v.end(), e), v.end());
}

//clears a vector, reset its size to zero, and removes the elements from memory. does NOT delete pointer elements
template <typename E> void vector_clear(E& t){ 
    t.clear();
    E().swap(t);
    t.shrink_to_fit(); 
}

//formats a number to have commas to represent thousandth places
template<typename T> const std::string convertNumToNumWithCommas(const T& n){
    std::string r = std::to_string(n);
    int p = static_cast<int>(r.length()) - 3;
    while(p > 0){
        r.insert(p, ",");
        p -= 3;
    }
    return r;
}

#endif