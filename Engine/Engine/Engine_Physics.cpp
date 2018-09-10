#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Math.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"

#include <bullet/BulletCollision/CollisionShapes/btShapeHull.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#include <bullet/BulletCollision/Gimpact/btCompoundFromGimpact.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

//yes, this code is needed or stuff breaks. find out why
#include <SFML/OpenGL.hpp>
#include <GL/freeglut.h>
////////////////////////////////////////////

using namespace Engine;
using namespace std;


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
                    unregisterEvent(EventType::WindowFullscreenChanged);
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
                    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LineVertex) * vertices.size(), &vertices[0]);
                    renderLines();
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
                    }
                    else if ((maxPs - minPs) >= SIMD_PI * btScalar(2.f)){
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
    };
};


void _preTicCallback(btDynamicsWorld* world, btScalar timeStep){
}
void _postTicCallback(btDynamicsWorld* world, btScalar timeStep){
}

class epriv::PhysicsManager::impl final{
    public:
        btBroadphaseInterface* m_Broadphase;
        btDefaultCollisionConfiguration* m_CollisionConfiguration;
        btCollisionDispatcher* m_Dispatcher;
        btSequentialImpulseConstraintSolver* m_Solver;
        btDiscreteDynamicsWorld* m_World;
        GLDebugDrawer* m_DebugDrawer;
        bool m_Paused;
        vector<Collision*> m_CollisionObjects;

        void _init(const char* name,uint& w,uint& h){
            m_Paused = false;
            m_Broadphase = new btDbvtBroadphase();
            m_CollisionConfiguration = new btDefaultCollisionConfiguration();
            m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
            m_Solver = new btSequentialImpulseConstraintSolver;
            m_World = new btDiscreteDynamicsWorld(m_Dispatcher,m_Broadphase,m_Solver,m_CollisionConfiguration);
            m_DebugDrawer = new GLDebugDrawer();
            m_DebugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
            m_World->setDebugDrawer(m_DebugDrawer);
            m_World->setGravity(btVector3(0.0f,0.0f,0.0f));

            btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher);

            m_World->setInternalTickCallback(_preTicCallback,(void*)m_World,true);
            m_World->setInternalTickCallback(_postTicCallback,(void*)m_World,false);
        }
        void _postInit(const char* name,uint w,uint h){
            m_DebugDrawer->initRenderingContext();
        }
        void _destruct(){
            SAFE_DELETE(m_DebugDrawer);
            SAFE_DELETE(m_World);
            SAFE_DELETE(m_Solver);
            SAFE_DELETE(m_Dispatcher);
            SAFE_DELETE(m_CollisionConfiguration);
            SAFE_DELETE(m_Broadphase);
            SAFE_DELETE_VECTOR(m_CollisionObjects);
        }
        void _update(float& dt, int& maxSteps, float& other){
            if(m_Paused) return;
            m_World->stepSimulation(dt,maxSteps,other);
            uint numManifolds = m_World->getDispatcher()->getNumManifolds();
            for (uint i = 0; i < numManifolds; ++i){
                btPersistentManifold* contactManifold =  m_World->getDispatcher()->getManifoldByIndexInternal(i);
                btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
                btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
                for (int j = 0; j < contactManifold->getNumContacts(); ++j){
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    if (pt.getDistance() < 0.0f){
                        const btVector3& ptA = pt.getPositionWorldOnA();
                        const btVector3& ptB = pt.getPositionWorldOnB();
                        const btVector3& normalOnB = pt.m_normalWorldOnB;

                        ComponentBody* a = (ComponentBody*)(obA->getUserPointer());
                        ComponentBody* b = (ComponentBody*)(obB->getUserPointer());

                        //a->collisionResponse(b);    b->collisionResponse(a);
                    }
                }
            }
        }
        void _render(){
            m_World->debugDrawWorld();

            Camera* c = Resources::getCurrentScene()->getActiveCamera();
            glm::vec3 camPos = c->getPosition();
            glm::mat4 model = glm::mat4(1.0f);
            model[3][0] -= camPos.x;
            model[3][1] -= camPos.y;
            model[3][2] -= camPos.z;
            Renderer::sendUniformMatrix4f("Model",model);
            Renderer::sendUniformMatrix4f("VP",c->getViewProjection());

            m_DebugDrawer->drawAccumulatedLines();
            m_DebugDrawer->postRender();
        }
        void _removeCollision(Collision* collisionObject){
            removeFromVector(m_CollisionObjects,collisionObject);
            SAFE_DELETE(collisionObject);
        }
};
epriv::PhysicsManager::impl* physicsManager;

epriv::PhysicsManager::PhysicsManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); physicsManager = m_i.get(); }
epriv::PhysicsManager::~PhysicsManager(){ m_i->_destruct(); }
void epriv::PhysicsManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::PhysicsManager::_update(float dt,int maxsteps,float other){ m_i->_update(dt,maxsteps,other); }
void epriv::PhysicsManager::_render(){ m_i->_render(); }
void epriv::PhysicsManager::_removeCollision(Collision* collisionObject){ m_i->_removeCollision(collisionObject); }

void Physics::pause(bool b){ physicsManager->m_Paused = b; }
void Physics::unpause(){ physicsManager->m_Paused = false; }
void Physics::setGravity(float x,float y,float z){ physicsManager->m_World->setGravity(btVector3(x,y,z)); }
void Physics::setGravity(glm::vec3& gravity){ Physics::setGravity(gravity.x,gravity.y,gravity.z); }
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ physicsManager->m_World->addRigidBody(rigidBody,group,mask); }
void Physics::addRigidBody(btRigidBody* rigidBody){ physicsManager->m_World->addRigidBody(rigidBody); }
void Physics::removeRigidBody(btRigidBody* rigidBody){ physicsManager->m_World->removeRigidBody(rigidBody); }
void Physics::updateRigidBody(btRigidBody* rigidBody){ physicsManager->m_World->updateSingleAabb(rigidBody); }
vector<glm::vec3> _rayCastInternal(const btVector3& start, const btVector3& end) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    physicsManager->m_World->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if (RayCallback.hasHit()) {
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(), RayCallback.m_hitPointWorld.y(), RayCallback.m_hitPointWorld.z());
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(), RayCallback.m_hitNormalWorld.y(), RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored){
        physicsManager->m_World->removeRigidBody(ignored);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    if(ignored){
        physicsManager->m_World->addRigidBody(ignored);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto object:ignored){
        physicsManager->m_World->removeRigidBody(object);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    for(auto object:ignored){
        physicsManager->m_World->addRigidBody(object);
    }
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,Entity* ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    
    ComponentBody* body = ignored->getComponent<ComponentBody>();

    if(body){
        return Physics::rayCast(_s,_e,const_cast<btRigidBody*>(body->getBody()));
    }
    return Physics::rayCast(_s,_e,nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Entity*>& ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    vector<btRigidBody*> objs;
    for(auto o:ignored){
        ComponentBody* body = o->getComponent<ComponentBody>();
        if(body){
            objs.push_back(const_cast<btRigidBody*>(body->getBody()));
        }
    }
    return Engine::Physics::rayCast(_s,_e,objs);
}




class Collision::impl final {
    public:
        btTriangleMesh* m_InternalMeshData;
        btVector3 m_Inertia;
        uint m_Type;
        btCollisionShape* m_Shape;

        void _init(Collision* super,float mass) {
            m_Inertia = btVector3(0.0f, 0.0f, 0.0f);
            _setMass(mass);
            physicsManager->m_CollisionObjects.push_back(super);
        }
        void _destruct() {
            SAFE_DELETE(m_InternalMeshData);
            btCompoundShape* compoundCast = dynamic_cast<btCompoundShape*>(m_Shape);
            if (compoundCast) {
                for (int i = 0; i < compoundCast->getNumChildShapes(); ++i) {
                    btCollisionShape* shape = compoundCast->getChildShape(i);
                    SAFE_DELETE(shape);
                }
            }
            SAFE_DELETE(m_Shape);
        }
        void _load(Engine::epriv::ImportedMeshData& data, CollisionType::Type collisionType, glm::vec3 _scale) {
            _destruct();

            m_InternalMeshData = nullptr;
            btCollisionShape* shape = nullptr;
            m_Type = collisionType;
            switch (collisionType) {
                case CollisionType::ConvexHull: {
                    shape = new btConvexHullShape();
                    btConvexHullShape& convexCast = *(btConvexHullShape*)shape;
                    for (auto vertex : data.points) {
                        convexCast.addPoint(btVector3(vertex.x * _scale.x, vertex.y * _scale.y, vertex.z * _scale.z));
                    }
                    btShapeHull hull = btShapeHull(&convexCast);
                    hull.buildHull(convexCast.getMargin());
                    SAFE_DELETE(shape);
                    const btVector3* ptsArray = hull.getVertexPointer();
                    shape = new btConvexHullShape();
                    btConvexHullShape& convexShape = *(btConvexHullShape*)shape;
                    for (int i = 0; i < hull.numVertices(); ++i) {
                        convexShape.addPoint(btVector3(ptsArray[i].x(), ptsArray[i].y(), ptsArray[i].z()));
                    }
                    convexShape.setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
                    convexShape.setMargin(0.001f);
                    convexShape.recalcLocalAabb();
                    break;
                }
                case CollisionType::TriangleShape: {
                    m_InternalMeshData = new btTriangleMesh();
                    for (auto triangle : data.file_triangles) {
                        btVector3 v1 = Math::btVectorFromGLM(triangle.v1.position * _scale);
                        btVector3 v2 = Math::btVectorFromGLM(triangle.v2.position * _scale);
                        btVector3 v3 = Math::btVectorFromGLM(triangle.v3.position * _scale);
                        m_InternalMeshData->addTriangle(v1, v2, v3, true);
                    }
                    shape = new btGImpactMeshShape(m_InternalMeshData);
                    btGImpactMeshShape& giShape = *((btGImpactMeshShape*)shape);
                    giShape.setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
                    giShape.setMargin(0.001f);
                    giShape.updateBound();
                    break;
                }
                case CollisionType::TriangleShapeStatic: {
                    m_InternalMeshData = new btTriangleMesh();
                    for (auto triangle : data.file_triangles) {
                        btVector3 v1 = Math::btVectorFromGLM(triangle.v1.position * _scale);
                        btVector3 v2 = Math::btVectorFromGLM(triangle.v2.position * _scale);
                        btVector3 v3 = Math::btVectorFromGLM(triangle.v3.position * _scale);
                        m_InternalMeshData->addTriangle(v1, v2, v3, true);
                    }
                    shape = new btBvhTriangleMeshShape(m_InternalMeshData, true);
                    btBvhTriangleMeshShape& triShape = *((btBvhTriangleMeshShape*)shape);
                    triShape.setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
                    triShape.setMargin(0.001f);
                    triShape.recalcLocalAabb();
                    break;
                }
                case CollisionType::Sphere: {
                    float radius = 0;
                    for (auto vertex : data.points) {
                        float length = glm::length(vertex);
                        if (length > radius) {
                            radius = length;
                        }
                    }
                    shape = new btSphereShape(radius * _scale.x);
                    btSphereShape& sphere = *((btSphereShape*)shape);
                    sphere.setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
                    sphere.setMargin(0.001f);
                    break;
                }
                case CollisionType::Box: {
                    glm::vec3 max = glm::vec3(0.0f);
                    for (auto vertex : data.file_points) {
                        float x = abs(vertex.x);
                        float y = abs(vertex.y);
                        float z = abs(vertex.z);
                        if (x > max.x) 
                            max.x = x;
                        if (y > max.y)
                            max.y = y;
                        if (z > max.z)
                            max.z = z;
                    }
                    shape = new btBoxShape(btVector3(max.x * _scale.x, max.y * _scale.y, max.z * _scale.z));
                    btBoxShape& boxShape = *(btBoxShape*)shape;
                    boxShape.setLocalScaling(btVector3(1.0f, 1.0f, 1.0f));
                    boxShape.setMargin(0.001f);
                    break;
                }
            }
            m_Shape = shape;
        }
        void _setMass(float _mass) {
            if (!m_Shape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None) return;
            m_Shape->calculateLocalInertia(_mass, m_Inertia);
        }
};


Collision::Collision(btCollisionShape* shape,CollisionType::Type type, float mass):m_i(new impl){
    m_i->m_Shape = shape;
    m_i->m_Type = type;
    m_i->_init(this,mass);
}
Collision::Collision(epriv::ImportedMeshData& data,CollisionType::Type type, float mass,glm::vec3 scale):m_i(new impl){ 
    m_i->_load(data,type,scale);
    m_i->_init(this,mass);
}
Collision::~Collision(){ m_i->_destruct(); }
void Collision::setMass(float mass){ m_i->_setMass(mass); }
const btVector3& Collision::getInertia() const { return m_i->m_Inertia; }
btCollisionShape* Collision::getShape() const { return m_i->m_Shape; }
const uint Collision::getType() const { return m_i->m_Type; }