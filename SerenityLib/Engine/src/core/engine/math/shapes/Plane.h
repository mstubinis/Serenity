#pragma once
#ifndef ENGINE_MATH_SHAPES_PLANE_H
#define ENGINE_MATH_SHAPES_PLANE_H

namespace Engine {
    class Plane final {
        private:
            glm::vec3 m_A      = glm::vec3(0.0f);
            glm::vec3 m_B      = glm::vec3(0.0f);
            glm::vec3 m_C      = glm::vec3(0.0f);
            glm::vec3 m_Normal = glm::vec3(0.0f);
        public:
            Plane() = default;
            Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
                m_A = a;
                m_B = b;
                m_C = c;
                m_Normal = glm::normalize(glm::cross(m_B - m_A, m_C - m_A));
            }
            float CalculateDot(const glm::vec3& locationModelSpace) const {
                return glm::dot(locationModelSpace, m_Normal);
            }
    };
};

#endif