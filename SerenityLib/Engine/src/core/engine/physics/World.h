#pragma once
#ifndef ENGINE_PHYSICS_WORLD_H
#define ENGINE_PHYSICS_WORLD_H

#include <btBulletDynamicsCommon.h>
#include <memory>

class btSequentialImpulseConstraintSolverMt;
namespace Engine {
    namespace epriv {
        class  GLDebugDrawer;
        class  PhyiscsDynamicWorld;

        struct PhysicsWorld {
            btBroadphaseInterface*                  broadphase;
            btDefaultCollisionConfiguration*        collisionConfiguration;
            btCollisionDispatcher*                  dispatcher;
            btSequentialImpulseConstraintSolver*    solver;
            btSequentialImpulseConstraintSolverMt*  solverMT;
            PhyiscsDynamicWorld*                    world;
            GLDebugDrawer*                          debugDrawer;
            PhysicsWorld(const unsigned int numCores);
            ~PhysicsWorld();
        };

        //This is derived so we can render btUniformScalingShapes, the current Bullet build has this bugged.
        class PhyiscsDynamicWorld : public btDiscreteDynamicsWorld {
            private:
                btScalar m_scale;
            public:
                PhyiscsDynamicWorld(btDispatcher* dp, btBroadphaseInterface* pc, btConstraintSolver* cs, btCollisionConfiguration* cc);
                virtual ~PhyiscsDynamicWorld();
                void debugDrawObject(const btTransform& worldTransform, const btCollisionShape* shape, const btVector3& color);
        };
    };
};


#endif