#pragma once
#ifndef ENGINE_PHYSICS_WORLD_H
#define ENGINE_PHYSICS_WORLD_H

class btSequentialImpulseConstraintSolverMt;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionConfiguration;
class btConstraintSolver;
class btBroadphaseInterface;
class btDispatcher;
class btCollisionShape;
class btTransform;
class btVector3;
class btDiscreteDynamicsWorld;
namespace Engine {
    namespace epriv {
        class  GLDebugDrawer;
        class PhysicsWorld final{
            public:
                btBroadphaseInterface*                  broadphase;
                btDefaultCollisionConfiguration*        collisionConfiguration;
                btCollisionDispatcher*                  dispatcher;
                btSequentialImpulseConstraintSolver*    solver;
                btSequentialImpulseConstraintSolverMt*  solverMT;
                btDiscreteDynamicsWorld*                world;
                GLDebugDrawer*                          debugDrawer;
            public:
                PhysicsWorld();
                ~PhysicsWorld();

                PhysicsWorld(const PhysicsWorld&)                      = delete;
                PhysicsWorld& operator=(const PhysicsWorld&)           = delete;
                PhysicsWorld(PhysicsWorld&& other) noexcept            = delete;
                PhysicsWorld& operator=(PhysicsWorld&& other) noexcept = delete;
        };
    };
};
#endif