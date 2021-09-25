#include <serenity/renderer/pipelines/DeferredShadowCasters.h>
#include <serenity/renderer/Renderer.h>

#pragma region Directional Light

Engine::priv::GLDeferredDirectionalLightShadowInfo::GLDeferredDirectionalLightShadowInfo(const Camera& camera, const DirectionalLight& directionalLight, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    setShadowInfo(shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
    setLookAt(directionalLight.getComponent<ComponentTransform>()->getForward(), glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
    calculateSplits(camera);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateSplits(const Camera& camera) {
    float nd;
    float fd;
    if (m_FrustumType == LightShadowFrustumType::CameraBased) {
        nd = camera.getNear() * m_NearFactor;
        fd = camera.getFar()  * m_FarFactor;
    } else {
        nd = m_NearFactor;
        fd = m_FarFactor;
    }
    if (nd != m_NearCache || fd != m_FarCache) {
        const float lambda         = 0.3f;
        const float ratio          = fd / nd;
        m_CascadeDistances.front() = nd;
        for (int i = 1; i < int(m_CascadeDistances.size()); i++) {
            const float si         = i / float(m_CascadeDistances.size());
            const float t_near     = lambda * (nd * powf(ratio, si)) + (1 - lambda) * (nd + (fd - nd) * si);
            m_CascadeDistances[i]  = t_near;
        }
        m_CascadeDistances.back()  = fd;
        m_NearCache                = nd;
        m_FarCache                 = fd;
    }
}
Engine::priv::GLDeferredDirectionalLightShadowInfo::~GLDeferredDirectionalLightShadowInfo() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(GLsizei(m_DepthTexture.size()), m_DepthTexture.data());
}
bool Engine::priv::GLDeferredDirectionalLightShadowInfo::initGL() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(GLsizei(m_DepthTexture.size()), m_DepthTexture.data());

    glGenTextures(m_DepthTexture.size(), m_DepthTexture.data());
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
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsReading(int textureStartSlot, const Camera& camera) noexcept {
    for (uint32_t i = 0; i < m_LightOrthoProjection.size(); ++i) {
        const glm::vec4 vClipSpace = camera.getProjection() * glm::vec4{ 0.0f, 0.0f, m_CascadeDistances[i + 1], 1.0f };
        m_BufferLightMatrices[i]   = m_LightOrthoProjection[i] * m_LightViewMatrix;
        m_BufferVClips[i]          = vClipSpace.z;
    }
    Engine::Renderer::sendUniformMatrix4vSafe("uLightMatrix[0]", m_BufferLightMatrices, uint32_t(m_BufferLightMatrices.size()));
    Engine::Renderer::sendUniform1vSafe("uCascadeEndClipSpace[0]", m_BufferVClips, uint32_t(m_BufferVClips.size()));
    Engine::Renderer::sendUniform1Safe("uShadowEnabled", 1);
    Engine::Renderer::sendUniform2Safe("uShadowTexelSize", 1.0f / float(m_ShadowWidth), 1.0f / float(m_ShadowHeight));

    Engine::Renderer::sendTexturesSafe("uShadowTexture[0]", m_DepthTexture.data(), textureStartSlot, GL_TEXTURE_2D, int(m_DepthTexture.size()));
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsWriting(int cascadeMapIndex) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture[cascadeMapIndex], 0);
    Engine::Renderer::Settings::clear(false, true, false);
    Engine::Renderer::sendUniformMatrix4Safe("uLightMatrix", m_LightOrthoProjection[cascadeMapIndex] * m_LightViewMatrix);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateOrthographicProjections(const Camera& camera, const DirectionalLight& directionalLight) {
    calculateSplits(camera);
    const float angleRadians       = (camera.getAngle() / 2.0f) + 0.5f;
    const float tanHalfHFOV        = glm::tan(angleRadians);
    const float tanHalfVFOV        = glm::tan(angleRadians) / camera.getAspectRatio();
    const auto cameraViewInverse   = glm::inverse(camera.getView());
    for (uint32_t i = 0; i < m_CascadeDistances.size() - 1; i++) {
        const float xNear = std::max(m_CascadeDistances[i + 0], 0.0f) * tanHalfHFOV;
        const float yNear = std::max(m_CascadeDistances[i + 0], 0.0f) * tanHalfVFOV;
        const float xFar  = m_CascadeDistances[i + 1] * tanHalfHFOV;
        const float yFar  = m_CascadeDistances[i + 1] * tanHalfVFOV;

        const std::array<glm::vec4, DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS> frustumCornersObjectSpace = {
            // Near face
            glm::vec4{ xNear, yNear,   -m_CascadeDistances[i + 0], 1.0 },
            glm::vec4{ -xNear, yNear,  -m_CascadeDistances[i + 0], 1.0 },
            glm::vec4{ xNear, -yNear,  -m_CascadeDistances[i + 0], 1.0 },
            glm::vec4{ -xNear, -yNear, -m_CascadeDistances[i + 0], 1.0 },

            // Far face
            glm::vec4{ xFar, yFar,     -m_CascadeDistances[i + 1], 1.0 },
            glm::vec4{ -xFar, yFar,    -m_CascadeDistances[i + 1], 1.0 },
            glm::vec4{ xFar, -yFar,    -m_CascadeDistances[i + 1], 1.0 },
            glm::vec4{ -xFar, -yFar,   -m_CascadeDistances[i + 1], 1.0 },
        };

        std::array<glm::vec4, DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS> frustumCornersLightSpace;
        float minX = std::numeric_limits<float>().max();
        float maxX = std::numeric_limits<float>().min();
        float minY = std::numeric_limits<float>().max();
        float maxY = std::numeric_limits<float>().min();
        //float minZ = std::numeric_limits<float>().max();
        //float maxZ = std::numeric_limits<float>().min();
        for (uint32_t j = 0; j < frustumCornersLightSpace.size(); j++) {
            const glm::vec4 worldSpace = cameraViewInverse * frustumCornersObjectSpace[j];
            frustumCornersLightSpace[j] = m_LightViewMatrix * worldSpace;
            minX = glm::min(minX, frustumCornersLightSpace[j].x);
            maxX = glm::max(maxX, frustumCornersLightSpace[j].x);
            minY = glm::min(minY, frustumCornersLightSpace[j].y);
            maxY = glm::max(maxY, frustumCornersLightSpace[j].y);
            //minZ = glm::min(minZ, frustumCornersLightSpace[j].z);
            //maxZ = glm::max(maxZ, frustumCornersLightSpace[j].z);
        }
        m_LightOrthoProjection[i] = glm::ortho(minX, maxX, minY, maxY, m_CascadeDistances.front(), m_CascadeDistances.back() * 2.0f);
    }
}

#pragma endregion