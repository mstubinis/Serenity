#ifndef ENGINE_CONSTRAINTS_H
#define ENGINE_CONSTRAINTS_H

class btRigidBody;
class btVector3;
class btTypedConstraint;

#include <glm/fwd.hpp>

enum class ConstraintType : unsigned char {
    Fixed,
    Hinge,
_TOTAL};
class Constraint{
    private:
        btRigidBody&                         m_RigidBodyA;
        btRigidBody&                         m_RigidBodyB;
        ConstraintType                       m_ConstraintType = ConstraintType::Fixed;
        std::unique_ptr<btTypedConstraint>   m_Constraint;

        Constraint()                                       = delete;
        Constraint(const Constraint& other)                = delete;
        Constraint& operator=(const Constraint& other)     = delete;
        Constraint(Constraint&& other) noexcept            = delete;
        Constraint& operator=(Constraint&& other) noexcept = delete;
    public:
        Constraint(btRigidBody& a, btRigidBody& b);
        ~Constraint();


        void makeHingeConstraint(glm::vec3& axisA,glm::vec3& axisB,glm::vec3 locationA = glm::vec3(0.0f),glm::vec3 locationB = glm::vec3(0.0f));
};
#endif