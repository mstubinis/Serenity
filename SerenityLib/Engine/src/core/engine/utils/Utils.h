#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

template <class OutType, class Data> void readBigEndian(OutType& out, Data& dataBuffer, unsigned int inBufferSizeInBytes, unsigned int& offset) noexcept {
    out = (uint32_t)dataBuffer[offset + 0U] << (8U * (inBufferSizeInBytes - 1U));
    for (auto i = 1U; i < inBufferSizeInBytes; ++i) {
        out |= (uint32_t)dataBuffer[offset + i] << (8U * ((inBufferSizeInBytes - i) - 1U));
    }
    offset += inBufferSizeInBytes;
}
template <class OutType, class Stream> void readBigEndian(Stream& inStream, OutType& out, unsigned int inBufferSizeInBytes) noexcept {
    std::vector<std::uint8_t> buffer(inBufferSizeInBytes, 0);
    inStream.read((char*)buffer.data(), inBufferSizeInBytes);

    out = (uint32_t)buffer[0] << (8U * (inBufferSizeInBytes - 1U));
    for (auto i = 1U; i < inBufferSizeInBytes; ++i) {
        out |= (uint32_t)buffer[i] << (8U * ((inBufferSizeInBytes - i) - 1U));
    }
}
template <class OutType, class Stream> void readBigEndian(Stream& inStream, OutType& out) noexcept {
    readBigEndian(inStream, out, sizeof(out));
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType& in, unsigned int inBufferSizeInBytes) noexcept {
    std::vector<std::uint8_t> buffer(inBufferSizeInBytes, 0);
    unsigned long long offset = 255U;
    for (int i = int(inBufferSizeInBytes) - 1; i >= 0; --i) {
        unsigned int shift = (8U * ((inBufferSizeInBytes - 1U) - i));
        buffer[i] = (in & (InType)offset) >> shift;
        offset = (offset * 255U) + offset;
    }
    inStream.write((char*)buffer.data(), buffer.size());
}
template <class InType, class Stream> void writeBigEndian(Stream& inStream, InType&& in, unsigned int inBufferSizeInBytes) noexcept {
    std::vector<std::uint8_t> buffer(inBufferSizeInBytes, 0);
    unsigned long long offset = 255U;
    for (int i = int(inBufferSizeInBytes) - 1; i >= 0; --i) {
        unsigned int shift = (8U * ((inBufferSizeInBytes - 1U) - i));
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
template<typename E, typename B> bool isInVector(std::vector<B*>& v, E* e) noexcept {
    for (auto& item : v) {
        if (item == e)
            return true;
    }
    return false;
}

//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B*>& v, const E* e) noexcept {
    v.erase(std::remove(v.begin(), v.end(), e), v.end());
}
//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B>& v,const E& e) noexcept {
    v.erase(std::remove(v.begin(), v.end(), e), v.end());
}

//clears a vector, reset its size to zero, and removes the elements from memory. does NOT delete pointer elements
template <typename E> void vector_clear(E& t) noexcept {
    t.clear();
    E().swap(t);
    t.shrink_to_fit(); 
}

//formats a number to have commas to represent thousandth places
template<typename T> std::string convertNumToNumWithCommas(const T& n) noexcept {
    std::string r = std::to_string(n);
    int p         = (int)r.length() - 3;
    while (p > 0) {
        r.insert(p, ",");
        p -= 3;
    }
    return r;
}


#endif