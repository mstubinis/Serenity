#include <serenity/renderer/pipelines/DeferredShadowCasters.h>
#include <serenity/renderer/Renderer.h>

namespace {
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
        const auto inv = glm::inverse(proj * view);
        std::vector<glm::vec4> frustumCorners;
        for (uint32_t x = 0; x < 2; ++x) {
            for (uint32_t y = 0; y < 2; ++y) {
                for (uint32_t z = 0; z < 2; ++z) {
                    const glm::vec4 pt = inv * glm::vec4{ 2.0f * float(x) - 1.0f, 2.0f * float(y) - 1.0f, 2.0f * float(z) - 1.0f, 1.0f };
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }
        return frustumCorners;
    }
    glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::vec3& direction, const Camera& camera) {
        const auto proj = glm::perspective(camera.getAngle(), camera.getAspectRatio(), nearPlane, farPlane);
        const auto corners = getFrustumCornersWorldSpace(proj, camera.getView());
        glm::vec3 center = glm::vec3{ 0.0f, 0.0f, 0.0f };
        for (const auto& corner : corners) {
            center += glm::vec3(corner);
        }
        center /= corners.size();
        const auto lightView = glm::lookAt(center + direction, center, glm::vec3{ 0.0f, 1.0f, 0.0f });
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::min();
        for (const auto& corner : corners) {
            const glm::vec4 trf = lightView * corner;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }
        // Tune this parameter according to the scene
        constexpr float zMult = 10.0f;
        if (minZ < 0) {
            minZ *= zMult;
        } else {
            minZ /= zMult;
        }
        if (maxZ < 0) {
            maxZ /= zMult;
        } else {
            maxZ *= zMult;
        }
        auto lightOrtho = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
        return lightOrtho * lightView;
    }
}

#pragma region Directional Light

Engine::priv::GLDeferredDirectionalLightShadowInfo::GLDeferredDirectionalLightShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    setShadowInfo(shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateSplits(const Camera& camera) {
    m_CascadeDistances[0] = camera.getFar() * 0.02f;
    m_CascadeDistances[1] = camera.getFar() * 0.04f;
    m_CascadeDistances[2] = camera.getFar() * 0.1f;
    m_CascadeDistances[3] = camera.getFar() * 0.5f;
}
Engine::priv::GLDeferredDirectionalLightShadowInfo::~GLDeferredDirectionalLightShadowInfo() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(GLsizei(m_DepthTexture.size()), m_DepthTexture.data());
}
bool Engine::priv::GLDeferredDirectionalLightShadowInfo::initGL() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(GLsizei(m_DepthTexture.size()), m_DepthTexture.data());

    glGenTextures(GLsizei(m_DepthTexture.size()), m_DepthTexture.data());
    for (uint32_t i = 0; i < m_DepthTexture.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, m_DepthTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ShadowWidth, m_ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture[0], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    #if defined(_DEBUG)
        GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): FBO Error is: " << framebufferStatus);
            return false;
        }
    #endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::setShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    m_ShadowWidth  = shadowMapWidth;
    m_ShadowHeight = shadowMapHeight;
    m_FrustumType  = frustumType;
    m_NearFactor   = nearFactor;
    m_FarFactor    = farFactor;
    initGL();
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsReading(int textureStartSlot) noexcept {
    Engine::Renderer::sendUniformMatrix4vSafe("uLightMatrix[0]", m_LightSpaceMatrices);
    Engine::Renderer::sendUniform1vSafe("uCascadeEndClipSpace[0]", m_CascadeDistances.data(), uint32_t(m_CascadeDistances.size()));
    Engine::Renderer::sendUniform1Safe("uShadowEnabled", 1);
    Engine::Renderer::sendUniform2Safe("uShadowTexelSize", 1.0f / float(m_ShadowWidth), 1.0f / float(m_ShadowHeight));

    Engine::Renderer::sendTexturesSafe("uShadowTexture[0]", m_DepthTexture.data(), textureStartSlot, GL_TEXTURE_2D, int(m_DepthTexture.size()));
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsWriting(int cascadeMapIndex) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture[cascadeMapIndex], 0);
    Engine::Renderer::Settings::clear(false, true, false);
    Engine::Renderer::sendUniformMatrix4Safe("uLightMatrix", m_LightSpaceMatrices[cascadeMapIndex]);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateOrthographicProjections(const Camera& camera, const glm::vec3& direction) {
    calculateSplits(camera);
    //the small += 0.5f's are used to help expand for the shadow fading. They are needed
    for (size_t i = 0; i < m_CascadeDistances.size() /* + 1*/; ++i) {
        if (i == 0) {
            m_LightSpaceMatrices[i] = getLightSpaceMatrix(camera.getNear(), m_CascadeDistances[i] + 0.5f, direction, camera);
        } else if (i < m_CascadeDistances.size()) {
            m_LightSpaceMatrices[i] = getLightSpaceMatrix(m_CascadeDistances[i - 1] - 0.5f, m_CascadeDistances[i] + 0.5f, direction, camera);
        } else {
            m_LightSpaceMatrices[i] = getLightSpaceMatrix(m_CascadeDistances[i - 1] - 0.5f, camera.getFar(), direction, camera);
        }
    }
}

#pragma endregion

#pragma region Sun Light

Engine::priv::GLDeferredSunLightShadowInfo::GLDeferredSunLightShadowInfo(uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor)
    : Engine::priv::GLDeferredDirectionalLightShadowInfo{ shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor }
{
}

#pragma endregion