#include <core/engine/physics/World.h>
#include <core/engine/physics/DebugDrawer.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <BulletCollision/Gimpact/btCompoundFromGimpact.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btConvexPolyhedron.h>

using namespace Engine;

void _preTicCallback(btDynamicsWorld* world, btScalar timeStep) {
}
void _postTicCallback(btDynamicsWorld* world, btScalar timeStep) {
}

epriv::PhyiscsDynamicWorld::PhyiscsDynamicWorld(btDispatcher* dp, btBroadphaseInterface* pc, btConstraintSolver* cs, btCollisionConfiguration* cc) : btDiscreteDynamicsWorld(dp, pc, cs, cc) {
    m_scale = btScalar(1.0);
}
epriv::PhyiscsDynamicWorld::~PhyiscsDynamicWorld() {
}
void epriv::PhyiscsDynamicWorld::debugDrawObject(const btTransform& worldTransform, const btCollisionShape* shape, const btVector3& color) {
    // Draw a small simplex at the center of the object
    if (getDebugDrawer() && getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_DrawFrames) {
        getDebugDrawer()->drawTransform(worldTransform, btScalar(0.3));
    }
    if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
        const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(shape);
        for (int i = compoundShape->getNumChildShapes() - 1; i >= 0; i--) {
            btTransform childTrans = compoundShape->getChildTransform(i);
            const btCollisionShape* colShape = compoundShape->getChildShape(i);
            debugDrawObject(worldTransform * childTrans, colShape, color);
        }
    }else if (shape->getShapeType() == UNIFORM_SCALING_SHAPE_PROXYTYPE) {
        const btUniformScalingShape* scalingShape = static_cast<const btUniformScalingShape*>(shape);
        m_scale = scalingShape->getUniformScalingFactor();
        debugDrawObject(worldTransform, scalingShape->getChildShape(), color);
    }else {
        switch (shape->getShapeType()) {
            case BOX_SHAPE_PROXYTYPE: {
                const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
                btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
                getDebugDrawer()->drawBox(-halfExtents * m_scale, halfExtents * m_scale, worldTransform, color);
                break;
            }case SPHERE_SHAPE_PROXYTYPE: {
                const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
                btScalar radius = sphereShape->getMargin();//radius doesn't include the margin, so draw with margin
                getDebugDrawer()->drawSphere(radius * m_scale, worldTransform, color);
                break;
            }case MULTI_SPHERE_SHAPE_PROXYTYPE: {
                const btMultiSphereShape* multiSphereShape = static_cast<const btMultiSphereShape*>(shape);
                btTransform childTransform;
                childTransform.setIdentity();
                for (int i = multiSphereShape->getSphereCount() - 1; i >= 0; i--) {
                    childTransform.setOrigin(multiSphereShape->getSpherePosition(i));
                    getDebugDrawer()->drawSphere(multiSphereShape->getSphereRadius(i) * m_scale, worldTransform * childTransform, color);
                }
                break;
            }case CAPSULE_SHAPE_PROXYTYPE: {
                const btCapsuleShape* capsuleShape = static_cast<const btCapsuleShape*>(shape);
                btScalar radius = capsuleShape->getRadius();
                btScalar halfHeight = capsuleShape->getHalfHeight();
                int upAxis = capsuleShape->getUpAxis();
                getDebugDrawer()->drawCapsule(radius * m_scale, halfHeight * m_scale, upAxis, worldTransform, color);
                break;
            }case CONE_SHAPE_PROXYTYPE: {
                const btConeShape* coneShape = static_cast<const btConeShape*>(shape);
                btScalar radius = coneShape->getRadius();
                btScalar height = coneShape->getHeight();
                int upAxis = coneShape->getConeUpIndex();
                getDebugDrawer()->drawCone(radius * m_scale, height * m_scale, upAxis, worldTransform, color);
                break;
            }case CYLINDER_SHAPE_PROXYTYPE: {
                const btCylinderShape* cylinder = static_cast<const btCylinderShape*>(shape);
                int upAxis = cylinder->getUpAxis();
                btScalar radius = cylinder->getRadius();
                btScalar halfHeight = cylinder->getHalfExtentsWithMargin()[upAxis];
                getDebugDrawer()->drawCylinder(radius * m_scale, halfHeight * m_scale, upAxis, worldTransform, color);
                break;
            }case STATIC_PLANE_PROXYTYPE: {
                const btStaticPlaneShape* staticPlaneShape = static_cast<const btStaticPlaneShape*>(shape);
                btScalar planeConst = staticPlaneShape->getPlaneConstant();
                const btVector3& planeNormal = staticPlaneShape->getPlaneNormal();
                getDebugDrawer()->drawPlane(planeNormal, planeConst, worldTransform, color);
                break;
            }default:{
                /// for polyhedral shapes
                if (shape->isPolyhedral()) {
                    btPolyhedralConvexShape* polyshape = (btPolyhedralConvexShape*)shape;
                    int i;
                    if (polyshape->getConvexPolyhedron()) {
                        const btConvexPolyhedron* poly = polyshape->getConvexPolyhedron();
                        for (i = 0; i < poly->m_faces.size(); i++) {
                            btVector3 centroid(0, 0, 0);
                            int numVerts = poly->m_faces[i].m_indices.size();
                            if (numVerts) {
                                int lastV = poly->m_faces[i].m_indices[numVerts - 1];
                                for (int v = 0; v < poly->m_faces[i].m_indices.size(); v++) {
                                    int curVert = poly->m_faces[i].m_indices[v];
                                    centroid += poly->m_vertices[curVert];
                                    getDebugDrawer()->drawLine(
                                        worldTransform * (poly->m_vertices[lastV] * m_scale),
                                        worldTransform * (poly->m_vertices[curVert] * m_scale),
                                        color
                                    );
                                    lastV = curVert;
                                }
                            }
                            centroid *= btScalar(1.f) / btScalar(numVerts);
                            if (getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_DrawNormals) {
                                btVector3 normalColor(1, 1, 0);
                                btVector3 faceNormal(poly->m_faces[i].m_plane[0], poly->m_faces[i].m_plane[1], poly->m_faces[i].m_plane[2]);
                                getDebugDrawer()->drawLine(
                                    worldTransform * (centroid * m_scale),
                                    worldTransform * ((centroid + faceNormal) * m_scale),
                                    normalColor
                                );
                            }
                        }
                    }else{
                        for (i = 0; i < polyshape->getNumEdges(); i++) {
                            btVector3 a, b;
                            polyshape->getEdge(i, a, b);
                            btVector3 wa = worldTransform * (a * m_scale);
                            btVector3 wb = worldTransform * (b * m_scale);
                            getDebugDrawer()->drawLine(wa, wb, color);
                        }
                    }
                }
                if (shape->isConcave()) {
                    btConcaveShape* concaveMesh = (btConcaveShape*)shape;
                    btVector3 aabbMax(btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT));
                    btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT));
                    PhysicsDebugDrawcallback drawCallback(getDebugDrawer(), worldTransform, color);
                    concaveMesh->processAllTriangles(&drawCallback, aabbMin, aabbMax);
                }
                if (shape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE) {
                    btConvexTriangleMeshShape* convexMesh = (btConvexTriangleMeshShape*)shape;
                    btVector3 aabbMax(btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT));
                    btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT));
                    PhysicsDebugDrawcallback drawCallback(getDebugDrawer(), worldTransform, color);
                    convexMesh->getMeshInterface()->InternalProcessAllTriangles(&drawCallback, aabbMin, aabbMax);
                }
            }
        }
    }
    m_scale = btScalar(1.0);
}
epriv::PhysicsWorld::PhysicsWorld(const unsigned int numCores) {
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    //if (numCores <= 1) {
    solver = new btSequentialImpulseConstraintSolver();
    solverMT = nullptr;
    //world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    world = new PhyiscsDynamicWorld(dispatcher, broadphase, solver, collisionConfiguration);
    //}else{
    //    solver = new btSequentialImpulseConstraintSolver();
    //    solverMT = new btSequentialImpulseConstraintSolverMt();
    //    world = new btDiscreteDynamicsWorldMt(dispatcher,broadphase,(btConstraintSolverPoolMt*)solverMT, solver, collisionConfiguration);
    //}
    debugDrawer = new GLDebugDrawer();
    debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    world->setDebugDrawer(debugDrawer);
    world->setGravity(btVector3(0.0f, 0.0f, 0.0f));
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
