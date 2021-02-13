#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <vector>
#include <string>

namespace Engine {
    template<class CONTAINER> CONTAINER create_and_reserve(uint32_t size) {
        CONTAINER result;
        result.reserve(size);
        return result;
    }

    template<class DERIVED, class BASE> inline DERIVED* type_cast(BASE* inPtr) noexcept {
        return (inPtr && typeid(DERIVED) == typeid(*inPtr)) ? static_cast<DERIVED*>(inPtr) : nullptr;
    }
    template<class DERIVED, class BASE> inline bool type_compare(BASE* inPtr) noexcept {
        return inPtr && typeid(DERIVED) == typeid(*inPtr);
    }
}
template <class OutType, class Data> void readBigEndian(OutType& out, Data& dataBuffer, uint32_t inBufferSizeInBytes, uint32_t& offset) noexcept {
    out = (uint32_t)dataBuffer[offset + 0U] << (8U * (inBufferSizeInBytes - 1U));
    for (auto i = 1U; i < inBufferSizeInBytes; ++i) {
        out |= (uint32_t)dataBuffer[offset + i] << (8U * ((inBufferSizeInBytes - i) - 1U));
    }
    offset += inBufferSizeInBytes;
}
template <class OutType, class Stream> void readBigEndian(Stream& inStream, OutType& out, uint32_t inBufferSizeInBytes) noexcept {
    std::vector<uint8_t> buffer(inBufferSizeInBytes, 0);
    inStream.read((char*)buffer.data(), inBufferSizeInBytes);

    out = (uint32_t)buffer[0] << (8U * (inBufferSizeInBytes - 1U));
    for (auto i = 1U; i < inBufferSizeInBytes; ++i) {
        out |= (uint32_t)buffer[i] << (8U * ((inBufferSizeInBytes - i) - 1U));
    }
}
template <class OutType, class Stream> void readBigEndian(Stream& inStream, OutType& out) noexcept {
    readBigEndian(inStream, out, sizeof(out));
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType& in, uint32_t inBufferSizeInBytes) noexcept {
    std::vector<uint8_t> buffer(inBufferSizeInBytes, 0);
    uint64_t offset = 255U;
    for (int i = int(inBufferSizeInBytes) - 1; i >= 0; --i) {
        uint32_t shift = (8U * ((inBufferSizeInBytes - 1U) - i));
        buffer[i] = (in & (InType)offset) >> shift;
        offset = (offset * 255U) + offset;
    }
    inStream.write((char*)buffer.data(), buffer.size());
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType&& in, uint32_t inBufferSizeInBytes) noexcept {
    std::vector<uint8_t> buffer(inBufferSizeInBytes, 0);
    uint64_t offset = 255U;
    for (int i = int(inBufferSizeInBytes) - 1; i >= 0; --i) {
        uint32_t shift = (8U * ((inBufferSizeInBytes - 1U) - i));
        buffer[i]  = (in & (InType)offset) >> shift;
        offset     = (offset * 255U) + offset;
    }
    inStream.write((char*)buffer.data(), buffer.size());
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType& in) noexcept {
    writeBigEndian(inStream, in, sizeof(in));
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType&& in) noexcept {
    writeBigEndian(inStream, in, sizeof(in));
}
//specifies if a specific pointer element is in a vector
template<class E, class B> bool isInVector(std::vector<B*>& v, E* e) noexcept {
    for (auto& item : v) {
        if (item == e) {
            return true;
        }
    }
    return false;
}

//formats a number to have commas to represent thousandth places
template<class T> std::string convertNumToNumWithCommas(const T& n) noexcept {
    std::string r = std::to_string(n);
    int p         = (int)r.length() - 3;
    while (p > 0) {
        r.insert(p, ",");
        p -= 3;
    }
    return r;
}

#endif