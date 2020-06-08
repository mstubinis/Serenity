#include <core/engine/utils/ForcedInclude.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;

Engine::color_vector_4::color_vector_4(const float r_, const float g_, const float b_, const float a_) {
    color.r = static_cast<unsigned char>(r_ * 255.0f);
    color.g = static_cast<unsigned char>(g_ * 255.0f);
    color.b = static_cast<unsigned char>(b_ * 255.0f);
    color.a = static_cast<unsigned char>(a_ * 255.0f);
}
Engine::color_vector_4::color_vector_4(const float color_) {
    color.r = static_cast<unsigned char>(color_ * 255.0f);
    color.g = static_cast<unsigned char>(color_ * 255.0f);
    color.b = static_cast<unsigned char>(color_ * 255.0f);
    color.a = static_cast<unsigned char>(color_ * 255.0f);
}
Engine::color_vector_4::color_vector_4(const glm::vec4& color_) {
    color.r = static_cast<unsigned char>(color_.r * 255.0f);
    color.g = static_cast<unsigned char>(color_.g * 255.0f);
    color.b = static_cast<unsigned char>(color_.b * 255.0f);
    color.a = static_cast<unsigned char>(color_.a * 255.0f);
}
Engine::color_vector_4::color_vector_4(const unsigned char r_, const unsigned char g_, const unsigned char b_, const unsigned char a_) {
    color.r = r_;
    color.g = g_;
    color.b = b_;
    color.a = a_;
}
Engine::color_vector_4::color_vector_4(const unsigned char color_) {
    color.r = color_;
    color.g = color_;
    color.b = color_;
    color.a = color_;
}

constexpr float one_over_255 = 1.0f / 255.0f;

const uint32_t Engine::color_vector_4::toPackedInt() const {
    auto res  = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
    return res;
}
glm::vec4 Engine::color_vector_4::unpackInt(uint32_t i) const {
    float xx = static_cast<float>((i >> 24) & 255);
    float yy = static_cast<float>((i >> 16) & 255);
    float zz = static_cast<float>((i >> 8) & 255);
    float ww = static_cast<float>(i & 255);
    return glm::vec4(xx * one_over_255,   yy * one_over_255,   zz * one_over_255,   ww * one_over_255);
}