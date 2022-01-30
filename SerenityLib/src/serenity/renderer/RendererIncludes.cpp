#include <serenity/renderer/RendererIncludes.h>
#include <array>

namespace {
#ifndef ENGINE_PRODUCTION
    constexpr const std::array<const char*, RenderStage::_TOTAL> ENUM_TO_NAMES{ {
        "GeometryOpaque",
        "GeometryOpaque_2",
        "GeometryOpaque_3",
        "GeometryOpaque_4",
        "GeometryTransparent",
        "GeometryTransparent_2",
        "GeometryTransparent_3",
        "GeometryTransparent_4",
        "GeometryTransparentTrianglesSorted",
        "GeometryTransparentTrianglesSorted_2",
        "GeometryTransparentTrianglesSorted_3",
        "GeometryTransparentTrianglesSorted_4",
        "ForwardOpaque",
        "ForwardOpaque_2",
        "ForwardOpaque_3",
        "ForwardOpaque_4",
        "ForwardTransparent",
        "ForwardTransparent_2",
        "ForwardTransparent_3",
        "ForwardTransparent_4",
        "ForwardTransparentTrianglesSorted",
        "ForwardTransparentTrianglesSorted_2",
        "ForwardTransparentTrianglesSorted_3",
        "ForwardTransparentTrianglesSorted_4",
        "ForwardParticles",
        "ForwardParticles_2",
        "ForwardParticles_3",
        "ForwardParticles_4",
        "Decals",
        "Decals_2",
        "Decals_3",
        "Decals_4",
    } };
#endif
}

const char* RenderStage::toString() const noexcept { 
#ifndef ENGINE_PRODUCTION
    return ENUM_TO_NAMES[m_Type]; 
#else
    return "";
#endif
}