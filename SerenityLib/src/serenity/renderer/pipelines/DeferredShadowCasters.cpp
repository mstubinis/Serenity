#include <serenity/renderer/pipelines/DeferredShadowCasters.h>
#include <serenity/renderer/Renderer.h>

#pragma region Directional Light

Engine::priv::GLDeferredDirectionalLightShadowInfo::GLDeferredDirectionalLightShadowInfo(const Camera& camera, const DirectionalLight& directionalLight, uint32_t shadowMapWidth, uint32_t shadowMapHeight, float orthographicRadius, float orthoNear, float orthoFar)
    : m_TexelSize{ glm::vec2{1.0f / static_cast<float>(shadowMapWidth), 1.0f / static_cast<float>(shadowMapHeight)} }
{
    m_ShadowWidth  = shadowMapWidth;
    m_ShadowHeight = shadowMapHeight;
    setOrtho(orthographicRadius, orthoNear, orthoFar);

    calculateSplits(camera);

    setLookAt(directionalLight.getComponent<ComponentTransform>()->getForward());
    initGL();
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateSplits(const Camera& camera) {
    float nd     = camera.getNear();
    float fd     = camera.getFar();
    float lambda = 0.3f;
    float ratio  = fd / nd;
    m_CascadeDistances[0] = nd;
    for (int i = 1; i < (int)m_CascadeDistances.size(); i++) {
        float si     = i / (float)m_CascadeDistances.size();
        float t_near = lambda * (nd * powf(ratio, si)) + (1 - lambda) * (nd + (fd - nd) * si);
        m_CascadeDistances[i] = t_near;
    }
    m_CascadeDistances[m_CascadeDistances.size() - 1] = fd;
}
Engine::priv::GLDeferredDirectionalLightShadowInfo::~GLDeferredDirectionalLightShadowInfo() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures((GLsizei)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS, m_DepthTexture.data());
}
bool Engine::priv::GLDeferredDirectionalLightShadowInfo::initGL() {
    glGenTextures(DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS, m_DepthTexture.data());
    for (uint32_t i = 0; i < (uint32_t)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS; ++i) {
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
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsReading(int textureStartSlot, const Camera& camera) noexcept {
    for (uint32_t i = 0; i < (uint32_t)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS; ++i) {
        const glm::vec4 vView{ 0.0f, 0.0f, m_CascadeDistances[i + 1], 1.0f };
        const glm::vec4 vClip = camera.getProjection() * vView;
        m_BufferLightMatrices[i] = m_LightOrthoProjection[i] * m_LightViewMatrix;
        m_BufferVClips[i] = vClip.z;
    }
    Engine::Renderer::sendUniformMatrix4vSafe("LightMatrix[0]", m_BufferLightMatrices, (uint32_t)m_BufferLightMatrices.size());
    Engine::Renderer::sendUniform1vSafe("CascadeEndClipSpace[0]", m_BufferVClips, (uint32_t)m_BufferVClips.size());
    Engine::Renderer::sendTexturesSafe("ShadowTexture[0]", m_DepthTexture.data(), textureStartSlot, GL_TEXTURE_2D, (int)m_DepthTexture.size());

    Engine::Renderer::sendUniform1Safe("ShadowEnabled", 1);
    Engine::Renderer::sendUniform2Safe("ShadowTexelSize", m_TexelSize);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::bindUniformsWriting(int cascadeMapIndex) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture[cascadeMapIndex], 0);
    Engine::Renderer::Settings::clear(false, true, false);
    Engine::Renderer::sendUniformMatrix4Safe("LightMatrix", m_LightOrthoProjection[cascadeMapIndex] * m_LightViewMatrix);
}
void Engine::priv::GLDeferredDirectionalLightShadowInfo::calculateOrthographicProjections(const Camera& camera, const DirectionalLight& directionalLight) {
    const float angleRadians       = (camera.getAngle() / 2.0f) + 0.5f;
    const float tanHalfHFOV        = glm::tan(angleRadians);
    const float tanHalfVFOV        = glm::tan(angleRadians) / camera.getAspectRatio();
    const auto cameraViewInverse   = glm::inverse(camera.getView());
    for (uint32_t i = 0; i < (uint32_t)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS; i++) {
        const float xNear = std::max((m_CascadeDistances[i + 0] - 1.0f), 0.0f) * tanHalfHFOV;
        const float yNear = std::max((m_CascadeDistances[i + 0] - 1.0f), 0.0f) * tanHalfVFOV;
        const float xFar  = (m_CascadeDistances[i + 1] + 1.0f) * tanHalfHFOV;
        const float yFar  = (m_CascadeDistances[i + 1] + 1.0f) * tanHalfVFOV;
        const glm::vec4 frustumCornersObjectSpace[DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS] = {
            // Near face
            glm::vec4{ xNear, yNear,   -(m_CascadeDistances[i + 0] - 1.0f), 1.0 },
            glm::vec4{ -xNear, yNear,  -(m_CascadeDistances[i + 0] - 1.0f), 1.0 },
            glm::vec4{ xNear, -yNear,  -(m_CascadeDistances[i + 0] - 1.0f), 1.0 },
            glm::vec4{ -xNear, -yNear, -(m_CascadeDistances[i + 0] - 1.0f), 1.0 },

            // Far face
            glm::vec4{ xFar, yFar,     -(m_CascadeDistances[i + 1] + 1.0f), 1.0 },
            glm::vec4{ -xFar, yFar,    -(m_CascadeDistances[i + 1] + 1.0f), 1.0 },
            glm::vec4{ xFar, -yFar,    -(m_CascadeDistances[i + 1] + 1.0f), 1.0 },
            glm::vec4{ -xFar, -yFar,   -(m_CascadeDistances[i + 1] + 1.0f), 1.0 },
        };
        glm::vec4 frustumCornersLightSpace[DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS];
        float minX = std::numeric_limits<float>().max();
        float maxX = std::numeric_limits<float>().min();
        float minY = std::numeric_limits<float>().max();
        float maxY = std::numeric_limits<float>().min();
        float minZ = std::numeric_limits<float>().max();
        float maxZ = std::numeric_limits<float>().min();
        for (uint32_t j = 0; j < (uint32_t)DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS; j++) {
            const glm::vec4 worldSpace = cameraViewInverse * frustumCornersObjectSpace[j];
            frustumCornersLightSpace[j] = m_LightViewMatrix * worldSpace;
            minX = glm::min(minX, frustumCornersLightSpace[j].x);
            maxX = glm::max(maxX, frustumCornersLightSpace[j].x);
            minY = glm::min(minY, frustumCornersLightSpace[j].y);
            maxY = glm::max(maxY, frustumCornersLightSpace[j].y);
            minZ = glm::min(minZ, frustumCornersLightSpace[j].z);
            maxZ = glm::max(maxZ, frustumCornersLightSpace[j].z);
        }
        m_LightOrthoProjection[i] = glm::ortho(minX, maxX, minY, maxY, m_CascadeDistances[0], m_CascadeDistances[m_CascadeDistances.size() - 1]);
    }
}

#pragma endregion