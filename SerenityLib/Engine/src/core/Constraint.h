#ifndef ENGINE_CONSTRAINTS_H
#define ENGINE_CONSTRAINTS_H

class btRigidBody;
class btVector3;
class btTypedConstraint;

#include <glm/fwd.hpp>

struct ConstraintType final {enum Type {
    Fixed,
    Hinge,
_TOTAL};};
class Constraint{
    private:
        btRigidBody* m_RigidBodyA;
        btRigidBody* m_RigidBodyB;
        ConstraintType::Type m_ConstraintType;
        btTypedConstraint* m_Constraint;
    public:
        Constraint(btRigidBody* a, btRigidBody* b);
        ~Constraint();

        void makeHingeConstraint(glm::vec3& axisA,glm::vec3& axisB,glm::vec3 locationA = glm::vec3(0.0f),glm::vec3 locationB = glm::vec3(0.0f));
};
#endif