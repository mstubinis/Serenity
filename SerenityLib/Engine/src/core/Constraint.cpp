#include "core/engine/utils/PrecompiledHeader.h"
#include <core/Constraint.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

Constraint::Constraint(btRigidBody* a, btRigidBody* b){
    m_RigidBodyA = a;
    m_RigidBodyB = b;
    m_ConstraintType = ConstraintType::Fixed;

    btTransform& a_transform = m_RigidBodyA->getWorldTransform();
    btTransform& b_transform = m_RigidBodyA->getWorldTransform();
    m_Constraint = new btFixedConstraint((*m_RigidBodyA),(*m_RigidBodyB),a_transform,b_transform);
    m_RigidBodyA->addConstraintRef(m_Constraint);
    m_RigidBodyB->addConstraintRef(m_Constraint);
}
Constraint::~Constraint() {
    SAFE_DELETE(m_Constraint);
}
void Constraint::makeHingeConstraint(glm::vec3& axisA, glm::vec3& axisB,glm::vec3 locationA, glm::vec3 locationB){
    m_RigidBodyA->removeConstraintRef(m_Constraint);
    m_RigidBodyB->removeConstraintRef(m_Constraint);
    SAFE_DELETE(m_Constraint);
    m_ConstraintType = ConstraintType::Hinge;

    btVector3 la(locationA.x,locationA.y,locationA.z);
    btVector3 lb(locationB.x,locationB.y,locationB.z);

    btVector3 a(axisA.x,axisA.y,axisA.z);
    btVector3 b(axisB.x,axisB.y,axisB.z);
    m_Constraint = new btHingeConstraint((*m_RigidBodyA),(*m_RigidBodyB),la,lb,a,b);
    m_RigidBodyA->addConstraintRef(m_Constraint);
    m_RigidBodyB->addConstraintRef(m_Constraint);
}