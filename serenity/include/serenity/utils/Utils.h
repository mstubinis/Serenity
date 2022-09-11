#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <cassert>
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
        size_t operator()(std::string_view txt) const { return std::hash<std::string_view>{}(txt); }
        size_t operator()(const std::string& txt) const { return std::hash<std::string_view>{}(txt); }
        size_t operator()(const char* txt) const { return std::hash<std::string_view>{}(txt); }
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

    template<class Container>
    void insertion_sort(Container& container) {
        using signed_size_t = std::make_signed_t<std::size_t>;
        signed_size_t i = 1;
        signed_size_t sz = static_cast<signed_size_t>(container.size());
        while (i < sz) {
            auto key = container[i]; //copy
            signed_size_t j = i - 1;
            while (j >= 0 && container[j] > key) {
                container[j + 1] = container[j];
                j -= 1;
            }
            container[j + 1] = key;
            i += 1;
        }
    }
    template<class CONTAINER, class VALUE>
    [[nodiscard]] size_t binary_search(const CONTAINER& container, const VALUE& entityID) noexcept {
        if (container.size() > 0) {
            if (container.size() == 1) {
                return container[0] == entityID ? 0 : std::numeric_limits<size_t>().max();
            }
            //binary search
            using signed_size_t = std::make_signed_t<size_t>;
            signed_size_t left  = 0;
            signed_size_t right = signed_size_t(container.size()) - signed_size_t(1);
            assert(right >= 0 && right < container.size());
            while (left <= right) {
                const signed_size_t mid = left + (right - left) / 2;
                assert(left >= 0 && left < container.size());
                assert(right >= 0 && right < container.size());
                assert(mid >= 0 && mid < container.size());
                const auto itrID = container[mid];
                if (itrID == entityID) {
                    return mid;
                } else if (itrID > entityID) {
                    right = mid - 1; //this is why we need signed integers
                } else {
                    left  = mid + 1;
                }
            }
        }
        return std::numeric_limits<size_t>().max();
    }

    template<class CONTAINER, class FUNC, class THEN, class ... ARGS>
    bool swap_and_pop_single_then(CONTAINER& container, FUNC&& func, THEN&& then, ARGS&&... args) {
        size_t i = 0;
        while (i != container.size()) {
            if (func(container[i], std::forward<ARGS>(args)...)) {
                then(container[i], std::forward<ARGS>(args)...);
                if (container.size() >= 2) {
                    size_t last = container.size() - 1;
                    assert(last >= 0 && last < container.size());
                    if (i != last) {
                        std::swap(container[i], container[last]);
                    }
                }
                if (container.size() > 1) {
                    container.pop_back();
                } else {
                    container.clear();
                }
                return true;
            }
            ++i;
        }
        return false;
    }
    template<class CONTAINER, class FUNC, class ... ARGS>
    bool swap_and_pop_single(CONTAINER& container, FUNC&& func, ARGS&&... args) {
        return swap_and_pop_single_then(container, func, FUNC{}, std::forward<ARGS>(args)...); //blank then function. FUNC uses same signature as THEN
    }
    template<class CONTAINER, class FUNC, class THEN, class ... ARGS>
    size_t swap_and_pop_then(CONTAINER& container, FUNC&& func, THEN&& then, ARGS&&... args) {
        size_t i = 0;
        size_t totalRemoved = 0;
        while (i != container.size()) {
            if (func(container[i], std::forward<ARGS>(args)...)) {
                then(container[i], std::forward<ARGS>(args)...);
                if (container.size() >= 2) {
                    size_t last = container.size() - 1;
                    if (i != last) {
                        std::swap(container[i], container[last]);
                        --i;
                    }
                }
                if (container.size() > 1) {
                    container.pop_back();
                } else {
                    container.clear();
                }
                ++totalRemoved;
            }
            ++i;
        }
        return totalRemoved;
    }
    template<class CONTAINER, class FUNC, class ... ARGS>
    size_t swap_and_pop(CONTAINER& container, FUNC&& func, ARGS&&... args) {
        return swap_and_pop_then(container, func, FUNC{}, std::forward<ARGS>(args)...);  //blank then function. FUNC uses same signature as THEN
    }
    template<class CONTAINER>
    bool swap_and_pop(CONTAINER& container, size_t index) {
        if (index < container.size()) {
            if (container.size() >= 2) {
                size_t last = container.size() - 1;
                assert(last >= 0 && last < container.size());
                if (index != last) {
                    std::swap(container[index], container[last]);
                }
            }
            if (container.size() > 1) {
                container.pop_back();
            } else {
                container.clear();
            }
            return true;
        }
        return false;
    }

    template<class KEY, class VALUE> using unordered_string_map = std::unordered_map<KEY, VALUE, Engine::string_hash, Engine::string_hash::equals>;
    template<class KEY>              using unordered_string_set = std::unordered_set<KEY, Engine::string_hash, Engine::string_hash::equals>;
    template<class KEY, class VALUE> using string_map           = std::map<KEY, VALUE, std::less<>>;
    template<class KEY>              using string_set           = std::set<KEY, std::less<>>;

    template<class CONTAINER, class VALUE, class SIZE> CONTAINER create_and_resize(SIZE&& size, VALUE value = VALUE{}) {
        CONTAINER result;
        result.resize(static_cast<size_t>(std::forward<SIZE>(size)), value);
        return result;
    }
    template<class CONTAINER, class SIZE> CONTAINER create_and_reserve(SIZE&& size) {
        CONTAINER result;
        result.reserve(static_cast<size_t>(std::forward<SIZE>(size)));
        return result;
    }
    template<class DERIVED, class BASE> inline DERIVED* type_cast(BASE* inPtr) noexcept {
        return (inPtr && typeid(DERIVED) == typeid(*inPtr)) ? static_cast<DERIVED*>(inPtr) : nullptr;
    }
    template<class DERIVED, class BASE> inline bool type_compare(BASE* inPtr) noexcept {
        return inPtr && typeid(DERIVED) == typeid(*inPtr);
    }

    //returns true if the iterator was found and erase was successful
    template<class CONTAINER, class ITERATOR> bool erase(CONTAINER& container, const ITERATOR& itr) {
        if (itr != container.end()) {
            container.erase(itr);
            return true;
        }
        return false;
    }


    template <class OUTTYPE, class DATA> void readBigEndian(OUTTYPE& out, const DATA& dataBuffer, const uint32_t inBufferSizeInBytes, uint32_t& offset) noexcept {
        assert(inBufferSizeInBytes > 0);
        out = OUTTYPE(dataBuffer[offset]) << 8 * (inBufferSizeInBytes - 1);
        for (uint32_t i = 1; i != inBufferSizeInBytes; ++i) {
            out |= OUTTYPE(dataBuffer[offset + i]) << 8 * ((inBufferSizeInBytes - i) - 1);
        }
        offset += inBufferSizeInBytes;
    }
    template <class OUTTYPE, class STREAM> void readBigEndian(STREAM& inStream, OUTTYPE& out, const uint32_t inBufferSizeInBytes) noexcept {
        assert(inBufferSizeInBytes > 0);
        std::vector<uint8_t> buffer(inBufferSizeInBytes, 0);
        inStream.read(reinterpret_cast<char*>(buffer.data()), inBufferSizeInBytes);
        out = OUTTYPE(buffer[0]) << 8 * (inBufferSizeInBytes - 1);
        for (uint32_t i = 1; i != inBufferSizeInBytes; ++i) {
            out |= OUTTYPE(buffer[i]) << 8 * ((inBufferSizeInBytes - i) - 1);
        }
    }
    template <class OUTTYPE, class STREAM> void readBigEndian(STREAM& inStream, OUTTYPE& out) noexcept {
        readBigEndian(inStream, out, sizeof(out));
    }
    template <class INTYPE, class STREAM> void writeBigEndian(STREAM& inStream, const INTYPE& in, const uint32_t inBufferSizeInBytes) noexcept {
        assert(inBufferSizeInBytes > 0);
        std::vector<uint8_t> buffer(inBufferSizeInBytes, 0);
        uint64_t offset = 255;
        for (uint32_t i = inBufferSizeInBytes; i-- > 0;) {
            uint64_t shift = (8 * ((inBufferSizeInBytes - 1) - i));
            buffer[i] = uint8_t(uint64_t(in & INTYPE(offset)) >> shift);
            offset <<= 8;
        }
        inStream.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    }
    template <class INTYPE, class STREAM> void writeBigEndian(STREAM& inStream, const INTYPE& in) noexcept {
        writeBigEndian(inStream, in, sizeof(in));
    }

    template<class INTEGRAL>
    requires std::is_integral_v<INTEGRAL>
    std::string convertNumToNumWithCommas(const INTEGRAL& number) noexcept {
        std::string res = std::to_string(number);
        res = res.substr(0, int(res.size()) - 1);
        int p = int(res.size()) - 3;
        while (p > 0) {
            res.insert(p, ",");
            p -= 3;
        }
        return res;
    }
}

#endif