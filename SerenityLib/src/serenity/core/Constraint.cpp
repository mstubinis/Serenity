
#include <serenity/core/Constraint.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <memory>

Constraint::Constraint(btRigidBody& a, btRigidBody& b) 
    : m_RigidBodyA{ a }
    , m_RigidBodyB{ b }
{
    btTransform& a_transform = m_RigidBodyA.getWorldTransform();
    btTransform& b_transform = m_RigidBodyA.getWorldTransform();
    m_Constraint             = std::make_unique<btFixedConstraint>(m_RigidBodyA, m_RigidBodyB, a_transform, b_transform);
    m_RigidBodyA.addConstraintRef(m_Constraint.get());
    m_RigidBodyB.addConstraintRef(m_Constraint.get());
}
Constraint::~Constraint() {
}
void Constraint::makeHingeConstraint(const glm::vec3& axisA, const glm::vec3& axisB, const glm::vec3& locationA, const glm::vec3& locationB){
    m_RigidBodyA.removeConstraintRef(m_Constraint.get());
    m_RigidBodyB.removeConstraintRef(m_Constraint.get());
    m_ConstraintType = ConstraintType::Hinge;

    btVector3 la(locationA.x,locationA.y,locationA.z);
    btVector3 lb(locationB.x,locationB.y,locationB.z);

    btVector3 a(axisA.x,axisA.y,axisA.z);
    btVector3 b(axisB.x,axisB.y,axisB.z);

    m_Constraint.reset(new btHingeConstraint(m_RigidBodyA, m_RigidBodyB, la, lb, a, b));

    m_RigidBodyA.addConstraintRef(m_Constraint.get());
    m_RigidBodyB.addConstraintRef(m_Constraint.get());
}