#include <core/engine/physics/World.h>
#include <core/engine/physics/DebugDrawer.h>
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
epriv::PhysicsWorld::PhysicsWorld(const unsigned int numCores) {
    broadphase             = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher             = new btCollisionDispatcher(collisionConfiguration);
    //if (numCores <= 1) {
    solver                 = new btSequentialImpulseConstraintSolver();
    solverMT               = nullptr;
    world                  = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    //}else{
    //    solver           = new btSequentialImpulseConstraintSolver();
    //    solverMT         = new btSequentialImpulseConstraintSolverMt();
    //    world            = new btDiscreteDynamicsWorldMt(dispatcher,broadphase,(btConstraintSolverPoolMt*)solverMT, solver, collisionConfiguration);
    //}
    debugDrawer = NEW GLDebugDrawer();
    debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    world->setDebugDrawer(debugDrawer);
    world->setGravity(btVector3(static_cast<btScalar>(0.0), static_cast<btScalar>(0.0), static_cast<btScalar>(0.0)));
    btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
    world->setInternalTickCallback(_preTicCallback, (void*)world, true);
    world->setInternalTickCallback(_postTicCallback, (void*)world, false);
}
epriv::PhysicsWorld::~PhysicsWorld() {
    SAFE_DELETE(debugDrawer);
    SAFE_DELETE(world);
    SAFE_DELETE(solver);
    SAFE_DELETE(solverMT);
    SAFE_DELETE(dispatcher);
    SAFE_DELETE(collisionConfiguration);
    SAFE_DELETE(broadphase);
}
