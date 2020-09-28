#pragma once
#ifndef ENGINE_MATH_SHAPES_PYRAMID_H
#define ENGINE_MATH_SHAPES_PYRAMID_H

#include <core/engine/math/shapes/Plane.h>

namespace Engine {
    /// <summary>
    /// A--------B
    /// | \    / |
    /// |    E   |
    /// | /    \ |
    /// C--------D
    /// </summary>
    class Pyramid final {
        private:
            std::array<Engine::Plane, 6> m_Planes;
        public:
            Pyramid() = default;
            Pyramid(const glm::vec3 & A, const glm::vec3 & B, const glm::vec3 & C, const glm::vec3 & D, const glm::vec3 & E) {
                Engine::Plane left(E, C, A);//E,C,A
                Engine::Plane right(E, B, D);//E,B,D
                Engine::Plane top(E, A, B);//E,A,B
                Engine::Plane bottom(E, D, C);//E,D,C
                Engine::Plane frontTopLeft(A, B, D);//A,B,C
                Engine::Plane frontBottomRight(D, C, A);//B,D,C

                m_Planes[0] = left;
                m_Planes[1] = right;
                m_Planes[2] = top;
                m_Planes[3] = bottom;
                m_Planes[4] = frontTopLeft;
                m_Planes[5] = frontBottomRight;
            }
            ~Pyramid() = default;

            bool isPointInside(const glm::vec3& locationModelSpace) const noexcept {
                for (const auto& plane : m_Planes) {
                    float plane_dot = plane.CalculateDot(locationModelSpace);
                    if (plane_dot > 0.0f) {
                        return false;
                    }
                }
                return true;
            }
    };
};

#endif