#pragma once
#ifndef ENGINE_PHYSICS_DEBUG_DRAWER_H
#define ENGINE_PHYSICS_DEBUG_DRAWER_H

#include <core/engine/events/Observer.h>
#include <BulletCollision/CollisionShapes/btTriangleCallback.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btIDebugDraw.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

namespace Engine::priv {
    class PhysicsManager;
    class PhysicsDebugDrawcallback final : public btTriangleCallback, public btInternalTriangleIndexCallback {
        private:
            btIDebugDraw& m_DebugDrawer;
            btVector3	  m_Color;
            btTransform   m_WorldTransform;

            PhysicsDebugDrawcallback() = delete;
        public:
            PhysicsDebugDrawcallback(btIDebugDraw* debugDrawer, const btTransform& worldTrans, const btVector3& color);
            virtual void internalProcessTriangleIndex(btVector3* triangle, int partId, int  triangleIndex);
            virtual void processTriangle(btVector3* triangle, int partId, int triangleIndex);
    };
    class GLDebugDrawer final : public btIDebugDraw, public Observer, public Engine::NonCopyable, public Engine::NonMoveable {
        friend class Engine::priv::PhysicsManager;
        private:
            GLuint m_PerFrameCount = 0U;
            GLuint m_Mode          = btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE;
            GLuint m_VAO           = 0U;
            GLuint m_VertexBuffer  = 0U;
            GLuint C_MAX_POINTS    = 2'097'152U;
            struct LineVertex final {
                glm::vec3 position = glm::vec3(0.0f);
                glm::vec3 color    = glm::vec3(1.0f);
            };
            std::vector<LineVertex> m_LineVertices;

            void init();
            void destruct();
            void bindDataToGPU();
            void render();
            void buildVAO();
            void postRender();
        public:
            GLDebugDrawer() = default;
            ~GLDebugDrawer();

            inline void setDebugMode(int mode) noexcept { m_Mode = mode; }
            inline int getDebugMode() const noexcept { return m_Mode; }

            void drawAccumulatedLines();
            void onEvent(const Event& e) override;
            void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3&, const btVector3&, const btVector3&, const btVector3& color, btScalar alpha);
            void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar);
            void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
            void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color);
            void drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
            void drawArc(const btVector3& center, const btVector3& normal, const btVector3& axis, btScalar radiusA, btScalar radiusB, btScalar minAngle, btScalar maxAngle, const btVector3& color, bool drawSect, btScalar stepDegrees = btScalar(10.f));
            void drawSpherePatch(const btVector3& center, const btVector3& up, const btVector3& axis, btScalar radius, btScalar minTh, btScalar maxTh, btScalar minPs, btScalar maxPs, const btVector3& color, btScalar stepDegrees = btScalar(10.f), bool drawCenter = true);
            void drawTransform(const btTransform& transform, btScalar orthoLen);
            void drawAabb(const btVector3& from, const btVector3& to, const btVector3& color);
            void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color);
            void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color);
            void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color);
            void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btTransform& trans, const btVector3& color);
            void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){}
            void drawPlane(const btVector3& planeNormal, btScalar planeConst, const btTransform& transform, const btVector3& color);
            void drawCone(btScalar radius, btScalar height, int upAxis, const btTransform& transform, const btVector3& color);
            void reportErrorWarning(const char* errWarning) {
                ENGINE_PRODUCTION_LOG("DebugDraw error: " << errWarning)
            }
            void draw3dText(const btVector3& location, const char* text) {}
    };
};

#endif