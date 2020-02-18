#include <core/engine/physics/World.h>
#include <core/engine/utils/Utils.h>

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMT.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMT.h>

using namespace std;
using namespace Engine;

void _preTicCallback(btDynamicsWorld* world, btScalar timeStep) {}
void _postTicCallback(btDynamicsWorld* world, btScalar timeStep) {}

priv::PhysicsWorld::PhysicsWorld() {
    m_Broadphase             = new btDbvtBroadphase();
    m_CollisionConfiguration = new btDefaultCollisionConfiguration();
    m_Dispatcher             = new btCollisionDispatcher(m_CollisionConfiguration);
    //if (numCores <= 1) {
    m_Solver                 = new btSequentialImpulseConstraintSolver();
    m_SolverMT               = nullptr;
    m_World                  = new btDiscreteDynamicsWorld(m_Dispatcher, m_Broadphase, m_Solver, m_CollisionConfiguration);
    //}else{
    //    m_Solver           = new btSequentialImpulseConstraintSolver();
    //    m_SolverMT         = new btSequentialImpulseConstraintSolverMt();
    //    m_World            = new btDiscreteDynamicsWorldMt(m_Dispatcher,m_Broadphase,(btConstraintSolverPoolMt*)m_SolverMT, m_Solver, m_CollisionConfiguration);
    //}
    m_DebugDrawer.setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    m_World->setDebugDrawer(&m_DebugDrawer);
    m_World->setGravity(btVector3(static_cast<btScalar>(0.0), static_cast<btScalar>(0.0), static_cast<btScalar>(0.0)));
    btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher);
    m_World->setInternalTickCallback(_preTicCallback, (void*)m_World, true);
    m_World->setInternalTickCallback(_postTicCallback, (void*)m_World, false);
}
priv::PhysicsWorld::~PhysicsWorld() {
    SAFE_DELETE(m_World);
    SAFE_DELETE(m_Solver);
    SAFE_DELETE(m_SolverMT);
    SAFE_DELETE(m_Dispatcher);
    SAFE_DELETE(m_CollisionConfiguration);
    SAFE_DELETE(m_Broadphase);
}
