#define BT_THREADSAFE 1
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Scene.h>

// ecs
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <BulletCollision/Gimpact/btCompoundFromGimpact.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>

//Multi-threading
#include <core/engine/threading/Engine_ThreadManager.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <BulletDynamics/Dynamics/btSimulationIslandManagerMt.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

//yes, this code is needed or stuff breaks. find out why
#include <SFML/OpenGL.hpp>
#include <GL/freeglut.h>
////////////////////////////////////////////

//bullet rendering fixes (see the custom world class below)
#include <BulletCollision/CollisionShapes/btConvexPolyhedron.h>
class DebugDrawcallback : public btTriangleCallback, public btInternalTriangleIndexCallback {
    private:
        btIDebugDraw* m_debugDrawer;
        btVector3	  m_color;
        btTransform   m_worldTrans;
    public:
        DebugDrawcallback(btIDebugDraw*	debugDrawer, const btTransform& worldTrans, const btVector3& color) {
            m_debugDrawer = debugDrawer; m_color = color; m_worldTrans = worldTrans;
        }
        virtual void internalProcessTriangleIndex(btVector3* triangle, int partId, int  triangleIndex) { processTriangle(triangle, partId, triangleIndex); }
        virtual void processTriangle(btVector3* triangle, int partId, int triangleIndex) {
            (void)partId; (void)triangleIndex;
            btVector3 wv0, wv1, wv2;
            wv0 = m_worldTrans * triangle[0]; wv1 = m_worldTrans * triangle[1]; wv2 = m_worldTrans * triangle[2];
            btVector3 center = (wv0 + wv1 + wv2)*btScalar(1. / 3.);
            if (m_debugDrawer->getDebugMode() & btIDebugDraw::DBG_DrawNormals) {
                btVector3 normal = (wv1 - wv0).cross(wv2 - wv0); normal.normalize();
                btVector3 normalColor(1, 1, 0); m_debugDrawer->drawLine(center, center + normal, normalColor);
            }
            m_debugDrawer->drawLine(wv0, wv1, m_color); m_debugDrawer->drawLine(wv1, wv2, m_color); m_debugDrawer->drawLine(wv2, wv0, m_color);
        }
};
////////////////////////////////////////////////////////////////////////////////////////



using namespace Engine;
using namespace std;

void _preTicCallback(btDynamicsWorld* world, btScalar timeStep) {
}
void _postTicCallback(btDynamicsWorld* world, btScalar timeStep) {
}

namespace Engine{
    namespace epriv{
        class GLDebugDrawer: public btIDebugDraw,public EventObserver {
            friend class ::Engine::epriv::PhysicsManager;
            private:
                GLuint m_Mode, m_VAO, m_VertexBuffer, C_MAX_POINTS;
                struct LineVertex{
                    glm::vec3 position;
                    glm::vec3 color;
                    LineVertex(){ position = glm::vec3(0.0f); color = glm::vec3(1.0f); }
                };
                vector<LineVertex> vertices;

                void init(){
                    m_Mode = btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints + btIDebugDraw::DBG_DrawConstraints + btIDebugDraw::DBG_DrawConstraintLimits;
                    C_MAX_POINTS = 262144;
                    m_VAO = m_VertexBuffer = 0;
                    registerEvent(EventType::WindowFullscreenChanged);
                }
                void destruct(){
                    glDeleteBuffers(1, &m_VertexBuffer);
                    Renderer::deleteVAO(m_VAO);
                    vector_clear(vertices);
                }
                void bindDataToGPU() {
                    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(offsetof(LineVertex, color)));
                }
                void renderLines(){
                    if(m_VAO){
                        Renderer::bindVAO(m_VAO);
                        glDrawArrays(GL_LINES, 0,vertices.size());
                        Renderer::bindVAO(0);
                    }else{
                        bindDataToGPU();
                        glDrawArrays(GL_LINES, 0,vertices.size());
                        glDisableVertexAttribArray(0);
                        glDisableVertexAttribArray(1);
                    }
                }
                void buildVAO() {
                    Renderer::deleteVAO(m_VAO);
                    if (epriv::RenderManager::OPENGL_VERSION >= 30) {
                        Renderer::genAndBindVAO(m_VAO);
                        bindDataToGPU();
                        Renderer::bindVAO(0);
                    }
                }
                void postRender(){
                    vector_clear(vertices);
                }
            public:
                void initRenderingContext(){
                    vector<LineVertex> temp1;
                    temp1.resize(C_MAX_POINTS,LineVertex());

                    glGenBuffers(1, &m_VertexBuffer);
                    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * C_MAX_POINTS, &temp1[0], GL_DYNAMIC_DRAW);

                    //support vao's
                    buildVAO();
                }
                void drawAccumulatedLines() {
                    if (vertices.size() > 0) {
                        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LineVertex) * vertices.size(), &vertices[0]);
                        renderLines();
                    }
                }
                void onEvent(const Event& e) {
                    if (e.type == EventType::WindowFullscreenChanged) {
                        buildVAO();
                    }
                }
                void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3&, const btVector3&, const btVector3&, const btVector3& color, btScalar alpha){
                    drawTriangle(v0, v1, v2, color, alpha);
                }
                void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar){
                    drawLine(v0, v1, color);
                    drawLine(v1, v2, color);
                    drawLine(v2, v0, color);
                }
                void drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
                    if(vertices.size() >= (C_MAX_POINTS)) return;
                    LineVertex v1, v2;
                    glm::vec3 _color = glm::vec3(color.x(),color.y(),color.z());
                    v1.color = _color;  v2.color = _color;
                    v1.position = glm::vec3(from.x(),from.y(),from.z());  v2.position = glm::vec3(to.x(),to.y(),to.z());
                    vertices.push_back(v1);  vertices.push_back(v2);
                }
                void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color){
                    btVector3 center = transform.getOrigin();
                    btVector3 up = transform.getBasis().getColumn(1);
                    btVector3 axis = transform.getBasis().getColumn(0);
                    btScalar minTh = -SIMD_HALF_PI;
                    btScalar maxTh = SIMD_HALF_PI;
                    btScalar minPs = -SIMD_HALF_PI;
                    btScalar maxPs = SIMD_HALF_PI;
                    btScalar stepDegrees = 30.f;
                    drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
                    drawSpherePatch(center, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
                }
                void drawSphere(const btVector3& p, btScalar radius, const btVector3& color){
                    btTransform tr;
                    tr.setIdentity();
                    tr.setOrigin(p);
                    drawSphere(radius, tr, color);
                }
                void drawArc(const btVector3& center, const btVector3& normal, const btVector3& axis, btScalar radiusA, btScalar radiusB, btScalar minAngle, btScalar maxAngle,const btVector3& color, bool drawSect, btScalar stepDegrees = btScalar(10.f)){
                    const btVector3& vx = axis;
                    btVector3 vy = normal.cross(axis);
                    btScalar step = stepDegrees * SIMD_RADS_PER_DEG;
                    int nSteps = (int)btFabs((maxAngle - minAngle) / step);
                    if (!nSteps) nSteps = 1;
                    btVector3 prev = center + radiusA * vx * btCos(minAngle) + radiusB * vy * btSin(minAngle);
                    if (drawSect){
                        drawLine(center, prev, color);
                    }
                    for (int i = 1; i <= nSteps; i++){
                        btScalar angle = minAngle + (maxAngle - minAngle) * btScalar(i) / btScalar(nSteps);
                        btVector3 next = center + radiusA * vx * btCos(angle) + radiusB * vy * btSin(angle);
                        drawLine(prev, next, color);
                        prev = next;
                    }
                    if (drawSect){
                        drawLine(center, prev, color);
                    }
                }
                void drawSpherePatch(const btVector3& center, const btVector3& up, const btVector3& axis, btScalar radius,btScalar minTh, btScalar maxTh, btScalar minPs, btScalar maxPs, const btVector3& color, btScalar stepDegrees = btScalar(10.f), bool drawCenter = true){
                    btVector3 vA[74];
                    btVector3 vB[74];
                    btVector3 *pvA = vA, *pvB = vB, *pT;
                    btVector3 npole = center + up * radius;
                    btVector3 spole = center - up * radius;
                    btVector3 arcStart;
                    btScalar step = stepDegrees * SIMD_RADS_PER_DEG;
                    const btVector3& kv = up;
                    const btVector3& iv = axis;
                    btVector3 jv = kv.cross(iv);
                    bool drawN = false;
                    bool drawS = false;
                    if (minTh <= -SIMD_HALF_PI){
                        minTh = -SIMD_HALF_PI + step;
                        drawN = true;
                    }
                    if (maxTh >= SIMD_HALF_PI){
                        maxTh = SIMD_HALF_PI - step;
                        drawS = true;
                    }
                    if (minTh > maxTh){
                        minTh = -SIMD_HALF_PI + step;
                        maxTh = SIMD_HALF_PI - step;
                        drawN = drawS = true;
                    }
                    int n_hor = (int)((maxTh - minTh) / step) + 1;
                    if (n_hor < 2) n_hor = 2;
                    btScalar step_h = (maxTh - minTh) / btScalar(n_hor - 1);
                    bool isClosed = false;
                    if (minPs > maxPs){
                        minPs = -SIMD_PI + step;
                        maxPs = SIMD_PI;
                        isClosed = true;
                    }else if ((maxPs - minPs) >= SIMD_PI * btScalar(2.f)){
                        isClosed = true;
                    }else{
                        isClosed = false;
                    }
                    int n_vert = (int)((maxPs - minPs) / step) + 1;
                    if (n_vert < 2) n_vert = 2;
                    btScalar step_v = (maxPs - minPs) / btScalar(n_vert - 1);
                    for (int i = 0; i < n_hor; i++){
                        btScalar th = minTh + btScalar(i) * step_h;
                        btScalar sth = radius * btSin(th);
                        btScalar cth = radius * btCos(th);
                        for (int j = 0; j < n_vert; j++){
                            btScalar psi = minPs + btScalar(j) * step_v;
                            btScalar sps = btSin(psi);
                            btScalar cps = btCos(psi);
                            pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
                            if (i){
                                drawLine(pvA[j], pvB[j], color);
                            }else if (drawS){
                                drawLine(spole, pvB[j], color);
                            }
                            if (j){
                                drawLine(pvB[j - 1], pvB[j], color);
                            }else{
                                arcStart = pvB[j];
                            }
                            if ((i == (n_hor - 1)) && drawN){
                                drawLine(npole, pvB[j], color);
                            }
                            if (drawCenter){
                                if (isClosed){
                                    if (j == (n_vert - 1)){
                                        drawLine(arcStart, pvB[j], color);
                                    }
                                }else{
                                    if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1)))){
                                        drawLine(center, pvB[j], color);
                                    }
                                }
                            }
                        }
                        pT = pvA; pvA = pvB; pvB = pT;
                    }
                }
                void drawTransform(const btTransform& transform, btScalar orthoLen){
                    btVector3 start = transform.getOrigin();
                    drawLine(start, start + transform.getBasis() * btVector3(orthoLen, 0, 0), btVector3(btScalar(1.), btScalar(0.3), btScalar(0.3)));
                    drawLine(start, start + transform.getBasis() * btVector3(0, orthoLen, 0), btVector3(btScalar(0.3), btScalar(1.), btScalar(0.3)));
                    drawLine(start, start + transform.getBasis() * btVector3(0, 0, orthoLen), btVector3(btScalar(0.3), btScalar(0.3), btScalar(1.)));
                }
                void drawAabb(const btVector3 &from, const btVector3 &to, const btVector3 &color){
                    btVector3 halfExtents = (to - from)* 0.5f;
                    btVector3 center = (to + from) *0.5f;
                    int i, j;
                    btVector3 edgecoord(1.f, 1.f, 1.f), pa, pb;
                    for (i = 0; i < 4; i++){
                        for (j = 0; j < 3; j++){
                            pa = btVector3(edgecoord[0] * halfExtents[0], edgecoord[1] * halfExtents[1], edgecoord[2] * halfExtents[2]);
                            pa += center;
                            int othercoord = j % 3;
                            edgecoord[othercoord] *= -1.f;
                            pb = btVector3(edgecoord[0] * halfExtents[0], edgecoord[1] * halfExtents[1], edgecoord[2] * halfExtents[2]);
                            pb += center;
                            drawLine(pa, pb, color);
                        }
                        edgecoord = btVector3(-1.f, -1.f, -1.f);
                        if (i < 3)
                            edgecoord[i] *= -1.f;
                    }
                }
                void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color){
                    btVector3 start = transform.getOrigin();
                    btVector3	offsetHeight(0, 0, 0);
                    offsetHeight[upAxis] = halfHeight;
                    int stepDegrees = 30;
                    btVector3 capStart(0.f, 0.f, 0.f);
                    capStart[upAxis] = -halfHeight;
                    btVector3 capEnd(0.f, 0.f, 0.f);
                    capEnd[upAxis] = halfHeight;
                    for (int i = 0; i < 360; i += stepDegrees){
                        capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = btSin(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = btCos(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
                    }
                    // Drawing top and bottom caps of the cylinder
                    btVector3 yaxis(0, 0, 0);
                    yaxis[upAxis] = btScalar(1.0);
                    btVector3 xaxis(0, 0, 0);
                    xaxis[(upAxis + 1) % 3] = btScalar(1.0);
                    drawArc(start - transform.getBasis()*(offsetHeight), transform.getBasis()*yaxis, transform.getBasis()*xaxis, radius, radius, 0, SIMD_2_PI, color, false, btScalar(10.0));
                    drawArc(start + transform.getBasis()*(offsetHeight), transform.getBasis()*yaxis, transform.getBasis()*xaxis, radius, radius, 0, SIMD_2_PI, color, false, btScalar(10.0));
                }
                void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color){
                    int stepDegrees = 30;
                    btVector3 capStart(0.f, 0.f, 0.f);
                    capStart[upAxis] = -halfHeight;

                    btVector3 capEnd(0.f, 0.f, 0.f);
                    capEnd[upAxis] = halfHeight;

                    // Draw the ends		
                    btTransform childTransform = transform;
                    childTransform.getOrigin() = transform * capStart;	
                    btVector3 center = childTransform.getOrigin();
                    btVector3 up = childTransform.getBasis().getColumn((upAxis + 1) % 3);
                    btVector3 axis = -childTransform.getBasis().getColumn(upAxis);
                    btScalar minTh = -SIMD_HALF_PI;
                    btScalar maxTh = SIMD_HALF_PI;
                    btScalar minPs = -SIMD_HALF_PI;
                    btScalar maxPs = SIMD_HALF_PI;
                    drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, btScalar(stepDegrees), false);
                                
                    childTransform = transform;
                    childTransform.getOrigin() = transform * capEnd;
                    center = childTransform.getOrigin();
                    up = childTransform.getBasis().getColumn((upAxis + 1) % 3);
                    axis = childTransform.getBasis().getColumn(upAxis);
                    drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, btScalar(stepDegrees), false);

                    // Draw some additional lines
                    btVector3 start = transform.getOrigin();
                    for (int i = 0; i < 360; i += stepDegrees){
                        capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = btSin(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = btCos(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
                    }
                }
                void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color){
                    drawLine(btVector3(bbMin[0], bbMin[1], bbMin[2]), btVector3(bbMax[0], bbMin[1], bbMin[2]), color);
                    drawLine(btVector3(bbMax[0], bbMin[1], bbMin[2]), btVector3(bbMax[0], bbMax[1], bbMin[2]), color);
                    drawLine(btVector3(bbMax[0], bbMax[1], bbMin[2]), btVector3(bbMin[0], bbMax[1], bbMin[2]), color);
                    drawLine(btVector3(bbMin[0], bbMax[1], bbMin[2]), btVector3(bbMin[0], bbMin[1], bbMin[2]), color);
                    drawLine(btVector3(bbMin[0], bbMin[1], bbMin[2]), btVector3(bbMin[0], bbMin[1], bbMax[2]), color);
                    drawLine(btVector3(bbMax[0], bbMin[1], bbMin[2]), btVector3(bbMax[0], bbMin[1], bbMax[2]), color);
                    drawLine(btVector3(bbMax[0], bbMax[1], bbMin[2]), btVector3(bbMax[0], bbMax[1], bbMax[2]), color);
                    drawLine(btVector3(bbMin[0], bbMax[1], bbMin[2]), btVector3(bbMin[0], bbMax[1], bbMax[2]), color);
                    drawLine(btVector3(bbMin[0], bbMin[1], bbMax[2]), btVector3(bbMax[0], bbMin[1], bbMax[2]), color);
                    drawLine(btVector3(bbMax[0], bbMin[1], bbMax[2]), btVector3(bbMax[0], bbMax[1], bbMax[2]), color);
                    drawLine(btVector3(bbMax[0], bbMax[1], bbMax[2]), btVector3(bbMin[0], bbMax[1], bbMax[2]), color);
                    drawLine(btVector3(bbMin[0], bbMax[1], bbMax[2]), btVector3(bbMin[0], bbMin[1], bbMax[2]), color);
                }
                void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btTransform& trans, const btVector3& color) {
                    drawLine(trans * btVector3(bbMin[0], bbMin[1], bbMin[2]), trans * btVector3(bbMax[0], bbMin[1], bbMin[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMin[1], bbMin[2]), trans * btVector3(bbMax[0], bbMax[1], bbMin[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMax[1], bbMin[2]), trans * btVector3(bbMin[0], bbMax[1], bbMin[2]), color);
                    drawLine(trans * btVector3(bbMin[0], bbMax[1], bbMin[2]), trans * btVector3(bbMin[0], bbMin[1], bbMin[2]), color);
                    drawLine(trans * btVector3(bbMin[0], bbMin[1], bbMin[2]), trans * btVector3(bbMin[0], bbMin[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMin[1], bbMin[2]), trans * btVector3(bbMax[0], bbMin[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMax[1], bbMin[2]), trans * btVector3(bbMax[0], bbMax[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMin[0], bbMax[1], bbMin[2]), trans * btVector3(bbMin[0], bbMax[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMin[0], bbMin[1], bbMax[2]), trans * btVector3(bbMax[0], bbMin[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMin[1], bbMax[2]), trans * btVector3(bbMax[0], bbMax[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMax[0], bbMax[1], bbMax[2]), trans * btVector3(bbMin[0], bbMax[1], bbMax[2]), color);
                    drawLine(trans * btVector3(bbMin[0], bbMax[1], bbMax[2]), trans * btVector3(bbMin[0], bbMin[1], bbMax[2]), color);
                }
                void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,btScalar distance,int lifeTime, const btVector3& color){
                }
                void drawPlane(const btVector3& planeNormal, btScalar planeConst, const btTransform& transform, const btVector3& color){
                    btVector3 planeOrigin = planeNormal * planeConst;
                    btVector3 vec0, vec1;
                    btPlaneSpace1(planeNormal, vec0, vec1);
                    btScalar vecLen = 100.f;
                    btVector3 pt0 = planeOrigin + vec0 * vecLen;
                    btVector3 pt1 = planeOrigin - vec0 * vecLen;
                    btVector3 pt2 = planeOrigin + vec1 * vecLen;
                    btVector3 pt3 = planeOrigin - vec1 * vecLen;
                    drawLine(transform*pt0, transform*pt1, color);
                    drawLine(transform*pt2, transform*pt3, color);
                }
                void drawCone(btScalar radius, btScalar height, int upAxis, const btTransform& transform, const btVector3& color){
                    int stepDegrees = 30;
                    btVector3 start = transform.getOrigin();
                    btVector3	offsetHeight(0, 0, 0);
                    btScalar halfHeight = height * btScalar(0.5);
                    offsetHeight[upAxis] = halfHeight;
                    btVector3	offsetRadius(0, 0, 0);
                    offsetRadius[(upAxis + 1) % 3] = radius;
                    btVector3	offset2Radius(0, 0, 0);
                    offset2Radius[(upAxis + 2) % 3] = radius;
                    btVector3 capEnd(0.f, 0.f, 0.f);
                    capEnd[upAxis] = -halfHeight;
                    for (int i = 0; i < 360; i += stepDegrees){
                        capEnd[(upAxis + 1) % 3] = btSin(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        capEnd[(upAxis + 2) % 3] = btCos(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
                        drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * capEnd, color);
                    }
                    drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight + offsetRadius), color);
                    drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight - offsetRadius), color);
                    drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight + offset2Radius), color);
                    drawLine(start + transform.getBasis() * (offsetHeight), start + transform.getBasis() * (-offsetHeight - offset2Radius), color);

                    // Drawing the base of the cone
                    btVector3 yaxis(0, 0, 0);
                    yaxis[upAxis] = btScalar(1.0);
                    btVector3 xaxis(0, 0, 0);
                    xaxis[(upAxis + 1) % 3] = btScalar(1.0);
                    drawArc(start - transform.getBasis()*(offsetHeight), transform.getBasis()*yaxis, transform.getBasis()*xaxis, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
                }
                void reportErrorWarning(const char* errWarning){
                }
                void draw3dText(const btVector3& location, const char* text){
                }
                void setDebugMode(int _mode){ m_Mode = _mode; }
                int getDebugMode() const { return m_Mode; }
                GLDebugDrawer(){ init(); }
                ~GLDebugDrawer(){ destruct(); }
        };
        
        //This is derived so we can render btUniformScalingShapes, the current Bullet build has this bugged.
        class EnginePhyiscsDynamicWorld : public btDiscreteDynamicsWorld {
            private:
                btScalar m_scale;
            public:
                EnginePhyiscsDynamicWorld(btDispatcher* dp, btBroadphaseInterface* pc, btConstraintSolver* cs, btCollisionConfiguration* cc):btDiscreteDynamicsWorld(dp,pc,cs,cc){
                    m_scale = btScalar(1.0);
                }
                virtual ~EnginePhyiscsDynamicWorld() {
                }
                void debugDrawObject(const btTransform& worldTransform, const btCollisionShape* shape, const btVector3& color) {
                    // Draw a small simplex at the center of the object
                    if (getDebugDrawer() && getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_DrawFrames) {
                        getDebugDrawer()->drawTransform(worldTransform, btScalar(0.3));
                    }
                    if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
                        const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(shape);
                        for (int i = compoundShape->getNumChildShapes() - 1; i >= 0; i--) {
                            btTransform childTrans = compoundShape->getChildTransform(i);
                            const btCollisionShape* colShape = compoundShape->getChildShape(i);
                            debugDrawObject(worldTransform*childTrans, colShape, color);
                        }
                    }else if (shape->getShapeType() == UNIFORM_SCALING_SHAPE_PROXYTYPE) {
                        const btUniformScalingShape* scalingShape = static_cast<const btUniformScalingShape*>(shape);
                        m_scale = scalingShape->getUniformScalingFactor();
                        debugDrawObject(worldTransform, scalingShape->getChildShape(), color);
                    }else{
                        switch (shape->getShapeType()) {
                            case BOX_SHAPE_PROXYTYPE: {
                                const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
                                btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
                                getDebugDrawer()->drawBox(-halfExtents * m_scale, halfExtents * m_scale, worldTransform, color);
                                break;
                            }
                            case SPHERE_SHAPE_PROXYTYPE: {
                                const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
                                btScalar radius = sphereShape->getMargin();//radius doesn't include the margin, so draw with margin
                                getDebugDrawer()->drawSphere(radius * m_scale, worldTransform, color);
                                break;
                            }
                            case MULTI_SPHERE_SHAPE_PROXYTYPE: {
                                const btMultiSphereShape* multiSphereShape = static_cast<const btMultiSphereShape*>(shape);
                                btTransform childTransform;
                                childTransform.setIdentity();
                                for (int i = multiSphereShape->getSphereCount() - 1; i >= 0; i--) {
                                    childTransform.setOrigin(multiSphereShape->getSpherePosition(i));
                                    getDebugDrawer()->drawSphere(multiSphereShape->getSphereRadius(i) * m_scale, worldTransform*childTransform, color);
                                }
                                break;
                            }
                            case CAPSULE_SHAPE_PROXYTYPE: {
                                const btCapsuleShape* capsuleShape = static_cast<const btCapsuleShape*>(shape);
                                btScalar radius = capsuleShape->getRadius();
                                btScalar halfHeight = capsuleShape->getHalfHeight();
                                int upAxis = capsuleShape->getUpAxis();
                                getDebugDrawer()->drawCapsule(radius * m_scale, halfHeight * m_scale, upAxis, worldTransform, color);
                                break;
                            }
                            case CONE_SHAPE_PROXYTYPE: {
                                const btConeShape* coneShape = static_cast<const btConeShape*>(shape);
                                btScalar radius = coneShape->getRadius();
                                btScalar height = coneShape->getHeight();
                                int upAxis = coneShape->getConeUpIndex();
                                getDebugDrawer()->drawCone(radius * m_scale, height * m_scale, upAxis, worldTransform, color);
                                break;
                            }
                            case CYLINDER_SHAPE_PROXYTYPE: {
                                const btCylinderShape* cylinder = static_cast<const btCylinderShape*>(shape);
                                int upAxis = cylinder->getUpAxis();
                                btScalar radius = cylinder->getRadius();
                                btScalar halfHeight = cylinder->getHalfExtentsWithMargin()[upAxis];
                                getDebugDrawer()->drawCylinder(radius * m_scale, halfHeight * m_scale, upAxis, worldTransform, color);
                                break;
                            }
                            case STATIC_PLANE_PROXYTYPE: {
                                const btStaticPlaneShape* staticPlaneShape = static_cast<const btStaticPlaneShape*>(shape);
                                btScalar planeConst = staticPlaneShape->getPlaneConstant();
                                const btVector3& planeNormal = staticPlaneShape->getPlaneNormal();
                                getDebugDrawer()->drawPlane(planeNormal, planeConst, worldTransform, color);
                                break;
                            }
                            default: {
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
                                                        worldTransform*(poly->m_vertices[lastV]* m_scale),
                                                        worldTransform*(poly->m_vertices[curVert]* m_scale),
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
                                                    worldTransform*(centroid*m_scale),
                                                    worldTransform*((centroid + faceNormal)*m_scale),
                                                    normalColor
                                                );
                                            }
                                        }
                                    }else {
                                        for (i = 0; i < polyshape->getNumEdges(); i++) {
                                            btVector3 a, b;
                                            polyshape->getEdge(i, a, b);
                                            btVector3 wa = worldTransform * (a *m_scale);
                                            btVector3 wb = worldTransform * (b *m_scale);
                                            getDebugDrawer()->drawLine(wa, wb, color);
                                        }
                                    }
                                }
                                if (shape->isConcave()) {
                                    btConcaveShape* concaveMesh = (btConcaveShape*)shape;
                                    btVector3 aabbMax(btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT));
                                    btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT));
                                    DebugDrawcallback drawCallback(getDebugDrawer(), worldTransform, color);
                                    concaveMesh->processAllTriangles(&drawCallback, aabbMin, aabbMax);
                                }
                                if (shape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE) {
                                    btConvexTriangleMeshShape* convexMesh = (btConvexTriangleMeshShape*)shape;		
                                    btVector3 aabbMax(btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT), btScalar(BT_LARGE_FLOAT));
                                    btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT), btScalar(-BT_LARGE_FLOAT));
                                    DebugDrawcallback drawCallback(getDebugDrawer(), worldTransform, color);
                                    convexMesh->getMeshInterface()->InternalProcessAllTriangles(&drawCallback, aabbMin, aabbMax);
                                }
                            }
                        }
                    }
                    m_scale = btScalar(1.0);
                }
        };

        struct PhysicsWorld {
            btBroadphaseInterface* broadphase;
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btSequentialImpulseConstraintSolver* solver;
            btSequentialImpulseConstraintSolverMt* solverMT;
            //btDiscreteDynamicsWorld* world;
            EnginePhyiscsDynamicWorld* world;
            GLDebugDrawer* debugDrawer;
            PhysicsWorld(uint numCores) {
                broadphase = new btDbvtBroadphase();
                collisionConfiguration = new btDefaultCollisionConfiguration();
                dispatcher = new btCollisionDispatcher(collisionConfiguration);
                //if (numCores <= 1) {
                    solver = new btSequentialImpulseConstraintSolver();
                    solverMT = nullptr;
                    //world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
                    world = new EnginePhyiscsDynamicWorld(dispatcher, broadphase, solver, collisionConfiguration);
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
            ~PhysicsWorld() {
                SAFE_DELETE(debugDrawer);
                SAFE_DELETE(world);
                SAFE_DELETE(solver);
                SAFE_DELETE(solverMT);
                SAFE_DELETE(dispatcher);
                SAFE_DELETE(collisionConfiguration);
                SAFE_DELETE(broadphase);
            }
        };
    };
};
class epriv::PhysicsManager::impl final{
    public:
        epriv::PhysicsWorld* data;
        bool m_Paused;

        void _init(const char* name, const uint& w, const uint& h){
            m_Paused = false;      
        }
        void _postInit(const char* name, const uint& w, const uint& h, const uint& numCores){
            data = new epriv::PhysicsWorld(numCores);
            data->debugDrawer->initRenderingContext();
        }
        void _destructWorldObjectsOnly() {
            auto& world = *data->world;
            int collisionObjCount = world.getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btCollisionObject* obj = world.getCollisionObjectArray()[i];
                if (obj) {
                    //btRigidBody* body = btRigidBody::upcast(obj);
                    //if (body) {
                        //auto* motionState = body->getMotionState();
                        //SAFE_DELETE(motionState);
                    //}
                    world.removeCollisionObject(obj);
                    SAFE_DELETE(obj);
                }
            }
        }
        void _destruct(){
            _destructWorldObjectsOnly();
            SAFE_DELETE(data);
        }
        void _update(const double& dt, int& maxSteps, float& other){
            if(m_Paused) 
                return;
            data->world->stepSimulation(static_cast<btScalar>(dt),maxSteps,other);

            uint numManifolds = data->dispatcher->getNumManifolds();
            for (uint i = 0; i < numManifolds; ++i){
                btPersistentManifold* contactManifold = data->dispatcher->getManifoldByIndexInternal(i);
                btCollisionObject* collisionObjectA = const_cast<btCollisionObject*>(contactManifold->getBody0());
                btCollisionObject* collisionObjectB = const_cast<btCollisionObject*>(contactManifold->getBody1());
                for (int j = 0; j < contactManifold->getNumContacts(); ++j){
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    if (pt.getDistance() < 0.0f){
                        glm::vec3 ptA = Math::btVectorToGLM(pt.getPositionWorldOnA());
                        glm::vec3 ptB = Math::btVectorToGLM(pt.getPositionWorldOnB());
                        glm::vec3 normalOnB = Math::btVectorToGLM(pt.m_normalWorldOnB);

                        auto aPtr = collisionObjectA->getUserPointer();
                        auto bPtr = collisionObjectB->getUserPointer();

                        ComponentBody* _a = static_cast<ComponentBody*>(aPtr);
                        ComponentBody* _b = static_cast<ComponentBody*>(bPtr);
                        if (_a && _b) {
                            ComponentBody& a = *_a;
                            ComponentBody& b = *_b;

                            a.collisionResponse(a, ptA, b, ptB, normalOnB);
                            b.collisionResponse(b, ptB, a, ptA, normalOnB);
                        }
                    }
                }
            }
        }
        void _render(Camera& camera){
            data->world->debugDrawWorld();
            const glm::vec3 camPos = camera.getPosition();
            const glm::mat4 model = glm::mat4(1.0f);
            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
            data->debugDrawer->drawAccumulatedLines();
            data->debugDrawer->postRender();
        }
        void _addRigidBody(btRigidBody* _rigidBody) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        return;
                    }
                }
            }
            data->world->addRigidBody(_rigidBody);
        }
        void _addRigidBody(btRigidBody* _rigidBody, short group, short mask) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        return;
                    }
                }
            }
            data->world->addRigidBody(_rigidBody, group, mask);
        }
        void _removeRigidBody(btRigidBody* _rigidBody) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        data->world->removeRigidBody(_rigidBody);
                        return;
                    }
                }
            }
        }
        void _updateRigidBody(btRigidBody* rigidBody) {
            data->world->updateSingleAabb(rigidBody);
        }
};
epriv::PhysicsManager::impl* physicsManager;

epriv::PhysicsManager::PhysicsManager(const char* name, const uint& w, const uint& h):m_i(new impl){ 
    m_i->_init(name,w,h); physicsManager = m_i.get(); 
}
epriv::PhysicsManager::~PhysicsManager(){ 
    m_i->_destruct(); 
}
void epriv::PhysicsManager::_init(const char* name, const uint& w, const uint& h, const uint& numCores){ 
    m_i->_postInit(name,w,h,numCores); 
}
void epriv::PhysicsManager::_update(const double& dt,int maxsteps,float other){ 
    m_i->_update(dt,maxsteps,other); 
}
void epriv::PhysicsManager::_render(Camera& camera){
    m_i->_render(camera);
}

void Physics::pause(bool b){ 
    physicsManager->m_Paused = b; 
}
void Physics::unpause(){ 
    physicsManager->m_Paused = false; 
}
void Physics::setGravity(const float x, const float y, const float z){ 
    physicsManager->data->world->setGravity(btVector3(x,y,z)); 
}
void Physics::setGravity(const glm::vec3& gravity){ 
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ 
    physicsManager->_addRigidBody(rigidBody,group,mask); 
}
void Physics::addRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_addRigidBody(rigidBody); 
}
void Physics::removeRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_removeRigidBody(rigidBody); 
}
void Physics::updateRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_updateRigidBody(rigidBody); 
}
vector<glm::vec3> _rayCastInternal(const btVector3& start, const btVector3& end) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    physicsManager->data->world->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if (RayCallback.hasHit()) {
        result.reserve(2); //is this needed performance wise?
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(), RayCallback.m_hitPointWorld.y(), RayCallback.m_hitPointWorld.z());
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(), RayCallback.m_hitNormalWorld.y(), RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored){
        physicsManager->data->world->removeRigidBody(ignored);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    if(ignored){
        physicsManager->data->world->addRigidBody(ignored);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto& object:ignored){
        physicsManager->data->world->removeRigidBody(object);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    for(auto& object:ignored){
        physicsManager->data->world->addRigidBody(object);
    }
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e, Entity* ignored){
    btVector3 _s = Math::btVectorFromGLM(s);
    btVector3 _e = Math::btVectorFromGLM(e);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body) {
			const auto& rigid = body->getBody();
            return Physics::rayCast(_s, _e, &const_cast<btRigidBody&>(rigid));
        }
    }
    return Physics::rayCast(_s, _e, nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Entity>& ignored){
    btVector3 _s = Math::btVectorFromGLM(s);
    btVector3 _e = Math::btVectorFromGLM(e);
    vector<btRigidBody*> objs;
    for(auto& o : ignored){
        ComponentBody* body = o.getComponent<ComponentBody>();
        if(body){
			const auto& rigid = body->getBody();
            objs.push_back(&const_cast<btRigidBody&>(rigid));
        }
    }
    return Physics::rayCast(_s, _e, objs);
}