#include <core/engine/physics/DebugDrawer.h>

#include <core/engine/renderer/Renderer.h>

using namespace Engine;
using namespace std;

#pragma region PhysicsDebugDrawcallback

priv::PhysicsDebugDrawcallback::PhysicsDebugDrawcallback(btIDebugDraw* debugDrawer, const btTransform& worldTrans, const btVector3& color) {
    m_DebugDrawer = debugDrawer;
    m_Color = color;
    m_WorldTransform = worldTrans;
}
void priv::PhysicsDebugDrawcallback::internalProcessTriangleIndex(btVector3* triangle, int partId, int  triangleIndex) {
    processTriangle(triangle, partId, triangleIndex);
}
void priv::PhysicsDebugDrawcallback::processTriangle(btVector3* triangle, int partId, int triangleIndex) {
    (void)partId;
    (void)triangleIndex;
    auto& drawer = *m_DebugDrawer;
    btVector3 wv0, wv1, wv2;
    wv0 = m_WorldTransform * triangle[0];
    wv1 = m_WorldTransform * triangle[1];
    wv2 = m_WorldTransform * triangle[2];
    btVector3 center = (wv0 + wv1 + wv2) * btScalar(1.0 / 3.0);
    if (drawer.getDebugMode() & btIDebugDraw::DBG_DrawNormals) {
        btVector3 normal = (wv1 - wv0).cross(wv2 - wv0);
        normal.normalize();
        btVector3 normalColor(1, 1, 0);
        drawer.drawLine(center, center + normal, normalColor);
    }
    drawer.drawLine(wv0, wv1, m_Color);
    drawer.drawLine(wv1, wv2, m_Color);
    drawer.drawLine(wv2, wv0, m_Color);
}

#pragma endregion

#pragma region GLDebugDrawer


priv::GLDebugDrawer::LineVertex::LineVertex() {
    position = glm::vec3(0.0f);
    color = glm::vec3(1.0f);
}
void priv::GLDebugDrawer::init() {
    m_Mode         = btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints + btIDebugDraw::DBG_DrawConstraints + btIDebugDraw::DBG_DrawConstraintLimits;
    C_MAX_POINTS   = 262144;
    m_VAO          = 0;
    m_VertexBuffer = 0;
    registerEvent(EventType::WindowFullscreenChanged);
}
void priv::GLDebugDrawer::destruct() {
    glDeleteBuffers(1, &m_VertexBuffer);
    Engine::Renderer::deleteVAO(m_VAO);
    vector_clear(vertices);
}
void priv::GLDebugDrawer::bindDataToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(offsetof(LineVertex, color)));
}
void priv::GLDebugDrawer::renderLines() {
    if (m_VAO) {
        Engine::Renderer::bindVAO(m_VAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        Engine::Renderer::bindVAO(0);
    }else{
        bindDataToGPU();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void priv::GLDebugDrawer::buildVAO() {
    Engine::Renderer::deleteVAO(m_VAO);
    if (Engine::priv::Renderer::OPENGL_VERSION >= 30) {
        Engine::Renderer::genAndBindVAO(m_VAO);
        bindDataToGPU();
        Engine::Renderer::bindVAO(0);
    }
}
void priv::GLDebugDrawer::postRender() {
    vector_clear(vertices);
}




priv::GLDebugDrawer::GLDebugDrawer() {
    init();
}
priv::GLDebugDrawer::~GLDebugDrawer() {
    destruct();
}

void priv::GLDebugDrawer::initRenderingContext() {
    vector<LineVertex> temp1;
    temp1.resize(C_MAX_POINTS, LineVertex());

    glGenBuffers(1, &m_VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * C_MAX_POINTS, &temp1[0], GL_DYNAMIC_DRAW);

    //support vao's
    buildVAO();
}
void priv::GLDebugDrawer::drawAccumulatedLines() {
    if (vertices.size() > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LineVertex) * vertices.size(), &vertices[0]);
        renderLines();
    }
}
void priv::GLDebugDrawer::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        buildVAO();
    }
}
void priv::GLDebugDrawer::drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3&, const btVector3&, const btVector3&, const btVector3& color, btScalar alpha) {
    drawTriangle(v0, v1, v2, color, alpha);
}
void priv::GLDebugDrawer::drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar) {
    drawLine(v0, v1, color);
    drawLine(v1, v2, color);
    drawLine(v2, v0, color);
}
void priv::GLDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    if (vertices.size() >= (C_MAX_POINTS)) 
        return;
    LineVertex v1, v2;
    glm::vec3 _color = glm::vec3(color.x(), color.y(), color.z());
    v1.color = _color;
    v2.color = _color;
    v1.position = glm::vec3(from.x(), from.y(), from.z());
    v2.position = glm::vec3(to.x(), to.y(), to.z());
    vertices.push_back(v1);
    vertices.push_back(v2);
}
void priv::GLDebugDrawer::drawSphere(btScalar radius, const btTransform& transform, const btVector3& color) {
    btVector3 center = transform.getOrigin();
    btVector3 up     = transform.getBasis().getColumn(1);
    btVector3 axis   = transform.getBasis().getColumn(0);
    btScalar minTh   = -SIMD_HALF_PI;
    btScalar maxTh   = SIMD_HALF_PI;
    btScalar minPs   = -SIMD_HALF_PI;
    btScalar maxPs   = SIMD_HALF_PI;
    btScalar stepDegrees = 30.f;
    drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
    drawSpherePatch(center, up, -axis, radius, minTh, maxTh, minPs, maxPs, color, stepDegrees, false);
}
void priv::GLDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color) {
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(p);
    drawSphere(radius, tr, color);
}
void priv::GLDebugDrawer::drawArc(const btVector3& center, const btVector3& normal, const btVector3& axis, btScalar radiusA, btScalar radiusB, btScalar minAngle, btScalar maxAngle, const btVector3& color, bool drawSect, btScalar stepDegrees) {
    const btVector3& vx = axis;
    btVector3 vy = normal.cross(axis);
    btScalar step = stepDegrees * SIMD_RADS_PER_DEG;
    int nSteps = (int)btFabs((maxAngle - minAngle) / step);
    if (!nSteps) nSteps = 1;
    btVector3 prev = center + radiusA * vx * btCos(minAngle) + radiusB * vy * btSin(minAngle);
    if (drawSect) {
        drawLine(center, prev, color);
    }
    for (int i = 1; i <= nSteps; i++) {
        btScalar angle = minAngle + (maxAngle - minAngle) * btScalar(i) / btScalar(nSteps);
        btVector3 next = center + radiusA * vx * btCos(angle) + radiusB * vy * btSin(angle);
        drawLine(prev, next, color);
        prev = next;
    }
    if (drawSect) {
        drawLine(center, prev, color);
    }
}
void priv::GLDebugDrawer::drawSpherePatch(const btVector3& center, const btVector3& up, const btVector3& axis, btScalar radius, btScalar minTh, btScalar maxTh, btScalar minPs, btScalar maxPs, const btVector3& color, btScalar stepDegrees, bool drawCenter) {
    btVector3 vA[74];
    btVector3 vB[74];
    btVector3* pvA = vA, * pvB = vB, * pT;
    btVector3 npole = center + up * radius;
    btVector3 spole = center - up * radius;
    btVector3 arcStart;
    btScalar step = stepDegrees * SIMD_RADS_PER_DEG;
    const btVector3& kv = up;
    const btVector3& iv = axis;
    btVector3 jv = kv.cross(iv);
    bool drawN = false;
    bool drawS = false;
    if (minTh <= -SIMD_HALF_PI) {
        minTh = -SIMD_HALF_PI + step;
        drawN = true;
    }
    if (maxTh >= SIMD_HALF_PI) {
        maxTh = SIMD_HALF_PI - step;
        drawS = true;
    }
    if (minTh > maxTh) {
        minTh = -SIMD_HALF_PI + step;
        maxTh = SIMD_HALF_PI - step;
        drawN = drawS = true;
    }
    int n_hor = (int)((maxTh - minTh) / step) + 1;
    if (n_hor < 2) n_hor = 2;
    btScalar step_h = (maxTh - minTh) / btScalar(n_hor - 1);
    bool isClosed = false;
    if (minPs > maxPs) {
        minPs = -SIMD_PI + step;
        maxPs = SIMD_PI;
        isClosed = true;
    }else if ((maxPs - minPs) >= SIMD_PI * btScalar(2.f)) {
        isClosed = true;
    }else{
        isClosed = false;
    }
    int n_vert = (int)((maxPs - minPs) / step) + 1;
    if (n_vert < 2) n_vert = 2;
    btScalar step_v = (maxPs - minPs) / btScalar(n_vert - 1);
    for (int i = 0; i < n_hor; i++) {
        btScalar th = minTh + btScalar(i) * step_h;
        btScalar sth = radius * btSin(th);
        btScalar cth = radius * btCos(th);
        for (int j = 0; j < n_vert; j++) {
            btScalar psi = minPs + btScalar(j) * step_v;
            btScalar sps = btSin(psi);
            btScalar cps = btCos(psi);
            pvB[j] = center + cth * cps * iv + cth * sps * jv + sth * kv;
            if (i) {
                drawLine(pvA[j], pvB[j], color);
            }else if (drawS) {
                drawLine(spole, pvB[j], color);
            }
            if (j) {
                drawLine(pvB[j - 1], pvB[j], color);
            }else{
                arcStart = pvB[j];
            }
            if ((i == (n_hor - 1)) && drawN) {
                drawLine(npole, pvB[j], color);
            }
            if (drawCenter) {
                if (isClosed) {
                    if (j == (n_vert - 1)) {
                        drawLine(arcStart, pvB[j], color);
                    }
                }else{
                    if (((!i) || (i == (n_hor - 1))) && ((!j) || (j == (n_vert - 1)))) {
                        drawLine(center, pvB[j], color);
                    }
                }
            }
        }
        pT = pvA;
        pvA = pvB;
        pvB = pT;
    }
}
void priv::GLDebugDrawer::drawTransform(const btTransform& transform, btScalar orthoLen) {
    btVector3 start = transform.getOrigin();
    drawLine(start, start + transform.getBasis() * btVector3(orthoLen, 0, 0), btVector3(btScalar(1.), btScalar(0.3), btScalar(0.3)));
    drawLine(start, start + transform.getBasis() * btVector3(0, orthoLen, 0), btVector3(btScalar(0.3), btScalar(1.), btScalar(0.3)));
    drawLine(start, start + transform.getBasis() * btVector3(0, 0, orthoLen), btVector3(btScalar(0.3), btScalar(0.3), btScalar(1.)));
}
void priv::GLDebugDrawer::drawAabb(const btVector3& from, const btVector3& to, const btVector3& color) {
    btVector3 halfExtents = (to - from) * 0.5f;
    btVector3 center = (to + from) * 0.5f;
    int i, j;
    btVector3 edgecoord(1.f, 1.f, 1.f), pa, pb;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
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
void priv::GLDebugDrawer::drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color) {
    btVector3 start = transform.getOrigin();
    btVector3	offsetHeight(0, 0, 0);
    offsetHeight[upAxis] = halfHeight;
    int stepDegrees = 30;
    btVector3 capStart(0.f, 0.f, 0.f);
    capStart[upAxis] = -halfHeight;
    btVector3 capEnd(0.f, 0.f, 0.f);
    capEnd[upAxis] = halfHeight;
    for (int i = 0; i < 360; i += stepDegrees) {
        capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = btSin(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
        capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = btCos(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
        drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
    }
    // Drawing top and bottom caps of the cylinder
    btVector3 yaxis(0, 0, 0);
    yaxis[upAxis] = btScalar(1.0);
    btVector3 xaxis(0, 0, 0);
    xaxis[(upAxis + 1) % 3] = btScalar(1.0);
    drawArc(start - transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, btScalar(10.0));
    drawArc(start + transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, btScalar(10.0));
}
void priv::GLDebugDrawer::drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color) {
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
    for (int i = 0; i < 360; i += stepDegrees) {
        capEnd[(upAxis + 1) % 3] = capStart[(upAxis + 1) % 3] = btSin(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
        capEnd[(upAxis + 2) % 3] = capStart[(upAxis + 2) % 3] = btCos(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
        drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
    }
}
void priv::GLDebugDrawer::drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color) {
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
void priv::GLDebugDrawer::drawBox(const btVector3& bbMin, const btVector3& bbMax, const btTransform& trans, const btVector3& color) {
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
void priv::GLDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
}
void priv::GLDebugDrawer::drawPlane(const btVector3& planeNormal, btScalar planeConst, const btTransform& transform, const btVector3& color) {
    btVector3 planeOrigin = planeNormal * planeConst;
    btVector3 vec0, vec1;
    btPlaneSpace1(planeNormal, vec0, vec1);
    btScalar vecLen = 100.f;
    btVector3 pt0 = planeOrigin + vec0 * vecLen;
    btVector3 pt1 = planeOrigin - vec0 * vecLen;
    btVector3 pt2 = planeOrigin + vec1 * vecLen;
    btVector3 pt3 = planeOrigin - vec1 * vecLen;
    drawLine(transform * pt0, transform * pt1, color);
    drawLine(transform * pt2, transform * pt3, color);
}
void priv::GLDebugDrawer::drawCone(btScalar radius, btScalar height, int upAxis, const btTransform& transform, const btVector3& color) {
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
    for (int i = 0; i < 360; i += stepDegrees) {
        capEnd[(upAxis + 1) % 3] = btSin(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
        capEnd[(upAxis + 2) % 3] = btCos(btScalar(i) * SIMD_RADS_PER_DEG) * radius;
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
    drawArc(start - transform.getBasis() * (offsetHeight), transform.getBasis() * yaxis, transform.getBasis() * xaxis, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
}
void priv::GLDebugDrawer::reportErrorWarning(const char* errWarning) {
}
void priv::GLDebugDrawer::draw3dText(const btVector3& location, const char* text) {
}
void priv::GLDebugDrawer::setDebugMode(int _mode) {
    m_Mode = _mode;
}
int priv::GLDebugDrawer::getDebugMode() const {
    return m_Mode;
}


#pragma endregion