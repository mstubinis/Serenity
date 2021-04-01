#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <execution>

namespace Engine {
    struct string_hash {
        struct equals : public std::equal_to<> {
            using is_transparent = void;
        };
        using is_transparent = void;
        using key_equal      = std::equal_to<>;  // Pred to use
        using hash_type      = std::hash<std::string_view>;  // just a helper local type
        size_t operator()(std::string_view txt) const { return hash_type{}(txt); }
        size_t operator()(const std::string& txt) const { return hash_type{}(txt); }
        size_t operator()(const char* txt) const { return hash_type{}(txt); }
    };
    template <class Container, class Pred>
    inline constexpr void sort(Container& container, Pred sorter) noexcept { 
        std::sort(std::begin(container), std::end(container), sorter); 
    }
    template <class Container>
    inline constexpr void sort(Container& container) noexcept { 
        std::sort(std::begin(container), std::end(container), std::less<>{}); 
    }
    template <class ExecPolicy, class Container, class Pred, typename std::remove_reference_t<ExecPolicy>::_Standard_execution_policy = 0>
    inline void sort(ExecPolicy&& policy, Container& container, Pred sorter) noexcept { 
        std::sort(policy, std::begin(container), std::end(container), sorter); 
    }
    template <class ExecPolicy, class Container, typename std::remove_reference_t<ExecPolicy>::_Standard_execution_policy = 0>
    inline void sort(ExecPolicy&& policy, Container& container) noexcept {
        std::sort(policy, std::begin(container), std::end(container), std::less<>{}); 
    }


    template<class KEY, class VALUE> using unordered_string_map = std::unordered_map<KEY, VALUE, Engine::string_hash, Engine::string_hash::equals>;
    template<class KEY>              using unordered_string_set = std::unordered_set<KEY, Engine::string_hash, Engine::string_hash::equals>;
    template<class KEY, class VALUE> using string_map           = std::map<KEY, VALUE, std::less<>>;
    template<class KEY>              using string_set           = std::set<KEY, std::less<>>;

    template<class CONTAINER, class VALUE> CONTAINER create_and_resize(uint32_t size, VALUE value = VALUE{}) {
        CONTAINER result;
        result.resize(size, value);
        return result;
    }
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
std::string numToCommasImpl(std::string s, int start, int end) noexcept;

template<class T> std::string convertNumToNumWithCommas(const T& n) noexcept {
    std::string r = std::to_string(n);
    return numToCommasImpl(r, 0, r.size() - 1);
}

#endif