#pragma once
#ifndef ENGINE_MATH_SHAPES_PYRAMID_H
#define ENGINE_MATH_SHAPES_PYRAMID_H

#include <serenity/core/engine/math/shapes/Plane.h>
#include <array>
#include <algorithm>

namespace Engine {
    /// A--------B
    /// | \    / |
    /// |    E   |
    /// | /    \ |
    /// C--------D
    class Pyramid final {
        private:
            std::array<Engine::Plane, 6> m_Planes;
        public:
            Pyramid() = default;
            Pyramid(const glm::vec3 & A, const glm::vec3 & B, const glm::vec3 & C, const glm::vec3 & D, const glm::vec3 & E) {
                Engine::Plane left{ E, C, A };
                Engine::Plane right{ E, B, D };
                Engine::Plane top{ E, A, B };
                Engine::Plane bottom{ E, D, C };
                Engine::Plane frontTopLeft{ A, B, D };
                Engine::Plane frontBottomRight{ D, C, A };

                m_Planes[0] = left;
                m_Planes[1] = right;
                m_Planes[2] = top;
                m_Planes[3] = bottom;
                m_Planes[4] = frontTopLeft;
                m_Planes[5] = frontBottomRight;
            }
            ~Pyramid() = default;

            inline bool isPointInside(const glm::vec3& locationModelSpace) const noexcept {
                return std::all_of(std::cbegin(m_Planes), std::cend(m_Planes), [&locationModelSpace](const auto& plane) {
                    return plane.CalculateDot(locationModelSpace) <= 0.0f;
                });
            }
    };
};

#endif