#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/math/MathCompression.h>

std::uint8_t Engine::Compression::pack2NibblesIntoChar(float val1, float val2) {
    std::uint8_t packedData = 0;
    int bits    = (int)std::round(val1 / 0.066666666666f);
    int bits1   = (int)std::round(val2 / 0.066666666666f);
    packedData |= bits & 15;
    packedData |= (bits1 << 4) & 240;
    return packedData;
}
glm::vec2 Engine::Compression::unpack2NibblesFromChar(std::uint8_t compressedValue) {
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
    float lowEnd  = (float)(compressedValue - (highEnd * 16.0f));
    return glm::vec2(highEnd, (float)(lowEnd / 255.0));
}

uint32_t Engine::Compression::pack3NormalsInto32Int(float x, float y, float z) {
    uint32_t xsign = x < 0; //if x < 0, this = 1, else this = 0
    uint32_t ysign = y < 0; //if y < 0, this = 1, else this = 0
    uint32_t zsign = z < 0; //if z < 0, this = 1, else this = 0
    float w = 0.0f;         //2 bits left for w, should i ever want to use it
    uint32_t wsign = w < 0; //if w < 0, this = 1, else this = 0
    uint32_t intW = ((uint32_t)(w + (wsign << 1)) & 1);
    uint32_t intZ = ((uint32_t)(z * 511 + (zsign << 9)) & 511);
    uint32_t intY = ((uint32_t)(y * 511 + (ysign << 9)) & 511);
    uint32_t intX = ((uint32_t)(x * 511 + (xsign << 9)) & 511);
    uint32_t data =
        (wsign << 31 | intW << 30) |
        (zsign << 29 | intZ << 20) |
        (ysign << 19 | intY << 10) |
        (xsign << 9 | intX);
    return data;
}
glm::vec3 Engine::Compression::unpack3NormalsFrom32Int(uint32_t compressedValue) {
    glm::vec3 conversions;
    glm::vec3 negatives = glm::vec3(1.0f);
    //X
    conversions.x = static_cast<float>(compressedValue & 1023 << 0);
    if (conversions.x >= 512.0f) { //2^9
        conversions.x = 1023 - conversions.x; //2^10
        negatives.x *= -1.0f;
    }
    conversions.x /= 511.0f * negatives.x; //(2^9) - 1
    //Y 
    conversions.y = static_cast<float>(compressedValue & 1023 << 10);
    if (conversions.y >= 524289.0f) { //2^19
        conversions.y = 1048575.0f - conversions.y; //2^20
        negatives.y *= -1.0f;
    }
    conversions.y /= 524288.0f * negatives.y; //(2^19) - 1
    //Z
    conversions.z = static_cast<float>(compressedValue & 1023 << 20);
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
    std::uint8_t _r = static_cast<std::uint8_t>(val1 * 255.0f);
    val2 = (val2 + 1.0f) * 0.5f;
    std::uint8_t _g = static_cast<std::uint8_t>(val2 * 255.0f);
    val3 = (val3 + 1.0f) * 0.5f;
    std::uint8_t _b = static_cast<std::uint8_t>(val3 * 255.0f);
    uint32_t packedColor = (_r << 16) | (_g << 8) | _b;
    float packedFloat = static_cast<float>((double)packedColor / (double)(1 << 24));
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
    std::uint8_t v1 = static_cast<std::uint8_t>(val1 * 255.0f);
    std::uint8_t v2 = static_cast<std::uint8_t>(val2 * 255.0f);
    std::uint8_t v3 = static_cast<std::uint8_t>(val3 * 255.0f);
    uint32_t packedColor = (v1 << 16) | (v2 << 8) | v3;
    float packedFloat = static_cast<float>((double)packedColor / (double)(1 << 24));
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
    int v1 = (int)((val1 + 1.0f) * 0.5f);
    int v2 = (int)((val2 + 1.0f) * 0.5f);
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