
#include <serenity/math/MathCompression.h>

uint8_t Engine::Compression::pack2NibblesIntoChar(float val1, float val2) {
    uint8_t packedData = 0;
    const int bits    = (int)std::round(val1 / 0.066666666666f);
    const int bits1   = (int)std::round(val2 / 0.066666666666f);
    packedData |= bits & 15;
    packedData |= (bits1 << 4) & 240;
    return packedData;
}
glm::vec2 Engine::Compression::unpack2NibblesFromChar(uint8_t compressedValue) {
    return glm::vec2(
        (float)(((int)compressedValue & 15) * 0.066666666666f),
        (float)(((int)compressedValue >> 4) * 0.066666666666f)
    );
}
float Engine::Compression::pack2NibblesIntoCharBasic(float val1, float val2) {
    return (
        std::round(val1 / 0.066666666666f) + 
        (std::round(val2 / 0.066666666666f) * 16.0f)
    );
}
glm::vec2 Engine::Compression::unpack2NibblesFromCharBasic(float compressedValue) {
    float highEnd = (compressedValue / 16.0f);
    highEnd       = highEnd - std::floor(highEnd);
    float lowEnd  = float(compressedValue - (highEnd * 16.0f));
    return glm::vec2(highEnd, float(lowEnd / 255.0));
}

 uint32_t Engine::Compression::pack3NormalsInto32Int(float x, float y, float z) {
    const uint32_t xs = x < 0.0f;
    const uint32_t ys = y < 0.0f;
    const uint32_t zs = z < 0.0f;
    constexpr float w = 1.0f;
    const uint32_t ws = w < 0.0f;
    const uint32_t packedVal =
        ws << 31 | ((uint32_t)(w + (ws << 1)) & 1) << 30 |
        zs << 29 | ((uint32_t)(z * 0b01'1111'1111 + (zs << 9)) & 0b01'1111'1111) << 20 |
        ys << 19 | ((uint32_t)(y * 0b01'1111'1111 + (ys << 9)) & 0b01'1111'1111) << 10 |
        xs <<  9 | ((uint32_t)(x * 0b01'1111'1111 + (xs << 9)) & 0b01'1111'1111);
    return packedVal;
}
glm::vec3 Engine::Compression::unpack3NormalsFrom32Int(uint32_t compressedValue) {
    glm::vec3 conversions;
    glm::vec3 negatives = glm::vec3{ 1.0f };
    //X
    conversions.x = float(compressedValue & 0b11'1111'1111 << 0);
    if (conversions.x >= 512.0f) { //2^9
        conversions.x = 1023 - conversions.x; //2^10
        negatives.x *= -1.0f;
    }
    conversions.x /= 511.0f * negatives.x; //(2^9) - 1
    //Y 
    conversions.y = float(compressedValue & 0b11'1111'1111 << 10);
    if (conversions.y >= 524289.0f) { //2^19
        conversions.y = 1048575.0f - conversions.y; //2^20
        negatives.y *= -1.0f;
    }
    conversions.y /= 524288.0f * negatives.y; //(2^19) - 1
    //Z
    conversions.z = float(compressedValue & 0b11'1111'1111 << 20);
    if (conversions.z >= 536870912.0f) { //2^29
        conversions.z = 1073741824.0f - conversions.z; //2^30
        negatives.z *= -1.0f;
    }
    conversions.z /= 536870911.0f * negatives.z; //(2^29) - 1
    return conversions;
}
 uint32_t Engine::Compression::pack3NormalsInto32Int(const glm::vec3& values) {
    return Engine::Compression::pack3NormalsInto32Int(values.x, values.y, values.z);
}

 float Engine::Compression::pack3FloatsInto1Float(float val1, float val2, float val3) {
    val1 = (val1 + 1.0f) * 0.5f;
    const uint8_t _r = uint8_t(val1 * 255.0f);
    val2 = (val2 + 1.0f) * 0.5f;
    const uint8_t _g = uint8_t(val2 * 255.0f);
    val3 = (val3 + 1.0f) * 0.5f;
    const uint8_t _b = uint8_t(val3 * 255.0f);
    const uint32_t packedColor = (_r << 16) | (_g << 8) | _b;
    const float packedFloat = float(double(packedColor) / double(1 << 24));
    return packedFloat;
}
 float Engine::Compression::pack3FloatsInto1Float(const glm::vec3& values) {
    return Engine::Compression::pack3FloatsInto1Float(values.r, values.g, values.b);
}
glm::vec3 Engine::Compression::unpack3FloatsInto1Float(float compressedValue) {
    glm::vec3 ret = glm::vec3(
        (float)std::fmod(compressedValue,  1.0f),
        (float)std::fmod(compressedValue * 256.0f, 1.0f),
        (float)std::fmod(compressedValue * 65536.0f, 1.0f)
    );
    ret = (ret * 2.0f) - 1.0f; //Unpack to the -1..1 range
    return ret;
}

 float Engine::Compression::pack3FloatsInto1FloatUnsigned(float val1, float val2, float val3) {
    const uint8_t v1 = uint8_t(val1 * 255.0f);
    const uint8_t v2 = uint8_t(val2 * 255.0f);
    const uint8_t v3 = uint8_t(val3 * 255.0f);
    const uint32_t packedColor = (v1 << 16) | (v2 << 8) | v3;
    const float packedFloat = float(double(packedColor) / double(1 << 24));
    return packedFloat;
}
 float Engine::Compression::pack3FloatsInto1FloatUnsigned(const glm::vec3& values) {
    return Engine::Compression::pack3FloatsInto1Float(values.r, values.g, values.b);
}
glm::vec3 Engine::Compression::unpack3FloatsInto1FloatUnsigned(float compressedValue) {
    return glm::vec3(
        (float)std::fmod(compressedValue, 1.0f),
        (float)std::fmod(compressedValue * 256.0f, 1.0f),
        (float)std::fmod(compressedValue * 65536.0f, 1.0f)
    );
}

float Engine::Compression::pack2FloatsInto1Float(float val1, float val2) {
    const int v1 = int((val1 + 1.0f) * 0.5f);
    const int v2 = int((val2 + 1.0f) * 0.5f);
    return std::floor(v1 * 1000.0f) + v2;
}
float Engine::Compression::pack2FloatsInto1Float(const glm::vec2& values) {
    return Engine::Compression::pack2FloatsInto1Float(values.x, values.y);
}
glm::vec2 Engine::Compression::unpack2FloatsInto1Float(float compressedValue) {
    glm::vec2 res;
    res.y = compressedValue - std::floor(compressedValue);
    res.x = (compressedValue - res.y) / 1000.0f;
    res.x = (res.x - 0.5f) * 2.0f;
    res.y = (res.y - 0.5f) * 2.0f;
    return res;
}
 uint16_t Engine::Compression::pack4ColorsInto16Int(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    const int r_ = int((float(r) * 16.0f) / 256.0f);
    const int g_ = int((float(g) * 16.0f) / 256.0f);
    const int b_ = int((float(b) * 16.0f) / 256.0f);
    const int a_ = int((float(a) * 16.0f) / 256.0f);
    return (r_ << 12) | (g_ << 8) | (b_ << 4) | a_;
}
 glm::u8vec4 Engine::Compression::unpackFour16IntColorInto4Chars(uint16_t i) noexcept {
    const float xx = float((i >> 12) & 0b1111);
    const float yy = float((i >> 8) & 0b1111);
    const float zz = float((i >> 4) & 0b1111);
    const float ww = float(i & 0b1111);
    return glm::u8vec4{
        uint8_t(xx * 16.0f),
        uint8_t(yy * 16.0f),
        uint8_t(zz * 16.0f),
        uint8_t(ww * 16.0f)
    };
}
 glm::vec4 Engine::Compression::unpackFour16IntColorInto4Floats(uint16_t i) noexcept {
    constexpr float one_over_15 = 0.0666666666666f;
    const float x = float((i >> 12) & 0b1111);
    const float y = float((i >> 8) & 0b1111);
    const float z = float((i >> 4) & 0b1111);
    const float w = float(i & 0b1111);
    return glm::vec4{
        x * one_over_15,
        y * one_over_15,
        z * one_over_15,
        w * one_over_15
    };
}