#ifndef ENGINE_CONSTRAINTS_H
#define ENGINE_CONSTRAINTS_H

class Object;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;
class btDiscreteDynamicsWorld;
class GLDebugDrawer;
class btCollisionShape;
class btRigidBody;
class btVector3;
class btTypedConstraint;
class btTriangleMesh;

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

enum ConstraintType{
    CONSTRAINT_TYPE_FIXED,
    CONSTRAINT_TYPE_HINGE,
};

class Constraint{
    private:
        btRigidBody* m_RigidBodyA;
        btRigidBody* m_RigidBodyB;
        ConstraintType m_ConstraintType;
        btTypedConstraint* m_Constraint;
    public:
        Constraint(btRigidBody* a, btRigidBody* b);
        ~Constraint();

        void makeHingeConstraint(glm::vec3& axisA,glm::vec3& axisB,glm::vec3& locationA = glm::vec3(0.0f),glm::vec3& locationB = glm::vec3(0.0f));
};

#endif