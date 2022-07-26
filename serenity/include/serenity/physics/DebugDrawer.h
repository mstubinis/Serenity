#pragma once
#ifndef ENGINE_PHYSICS_DEBUG_DRAWER_H
#define ENGINE_PHYSICS_DEBUG_DRAWER_H

namespace Engine::priv {
    class  PhysicsPipeline;
    class  PhysicsModule;
};

#include <serenity/dependencies/glm.h>
#include <serenity/system/Macros.h>
#include <serenity/events/Observer.h>
#include <serenity/resources/mesh/gl/VertexArrayObject.h>
#include <serenity/resources/mesh/gl/VertexBufferObject.h>
#include <BulletCollision/CollisionShapes/btTriangleCallback.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btIDebugDraw.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>

namespace Engine::priv {
    class PhysicsDebugDrawcallback final : public btTriangleCallback, public btInternalTriangleIndexCallback {
        private:
            btIDebugDraw& m_DebugDrawer;
            btVector3	  m_Color;
            btTransform   m_WorldTransform;

            PhysicsDebugDrawcallback() = delete;
        public:
            PhysicsDebugDrawcallback(btIDebugDraw*, const btTransform& worldTrans, const btVector3& color);
            void internalProcessTriangleIndex(btVector3* triangle, int partId, int  triangleIndex) override;
            void processTriangle(btVector3* triangle, int partId, int triangleIndex) override;
    };
    class GLDebugDrawer final : public btIDebugDraw, public Observer {
        friend class Engine::priv::PhysicsPipeline;
        friend class Engine::priv::PhysicsModule;
        private:
            GLuint               m_PerFrameCount = 0;
            GLuint               m_Mode          = btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE;
            VertexArrayObject    m_VAO;
            VertexBufferObject   m_VBO;
            struct LineVertex final {
                glm::vec3 position = glm::vec3{ 0.0f };
                glm::vec3 color    = glm::vec3{ 1.0f };
            };
            std::vector<LineVertex> m_LineVertices;

            void init();
            void bindDataToGPU();
            void render();
            void buildVAO();
            void postRender();
        public:
            inline void setDebugMode(int mode) noexcept override { m_Mode = mode; }
            [[nodiscard]] inline int getDebugMode() const noexcept override { return m_Mode; }

            void drawAccumulatedLines();
            void onEvent(const Event&) override;
            void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3&, const btVector3&, const btVector3&, const btVector3& color, btScalar alpha) override;
            void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar) override;
            void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
            void drawSphere(btScalar radius, const btTransform&, const btVector3& color) override;
            void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;
            void drawArc(const btVector3& center, const btVector3& normal, const btVector3& axis, btScalar radiusA, btScalar radiusB, btScalar minAngle, btScalar maxAngle, const btVector3& color, bool drawSect, btScalar stepDegrees = btScalar(10.f)) override;
            void drawSpherePatch(const btVector3& center, const btVector3& up, const btVector3& axis, btScalar radius, btScalar minTh, btScalar maxTh, btScalar minPs, btScalar maxPs, const btVector3& color, btScalar stepDegrees = btScalar(10.f), bool drawCenter = true) override;
            void drawTransform(const btTransform&, btScalar orthoLen) override;
            void drawAabb(const btVector3& from, const btVector3& to, const btVector3& color) override;
            void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform&, const btVector3& color) override;
            void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform&, const btVector3& color) override;
            void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color) override;
            void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btTransform&, const btVector3& color) override;
            void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {}
            void drawPlane(const btVector3& planeNormal, btScalar planeConst, const btTransform&, const btVector3& color) override;
            void drawCone(btScalar radius, btScalar height, int upAxis, const btTransform&, const btVector3& color) override;
            void reportErrorWarning(const char* errWarning) override {
                ENGINE_PRODUCTION_LOG("btDebugDraw error: " << errWarning)
            }
            void draw3dText(const btVector3& location, const char* text) override {}
    };
};

#endif