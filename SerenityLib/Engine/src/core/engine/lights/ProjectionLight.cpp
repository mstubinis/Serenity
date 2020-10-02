#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/ProjectionLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Scene.h>

using namespace Engine;

ProjectionLight::ProjectionLight(Texture* texture, const glm::vec3& direction, Scene* scene) 
    : SunLight{ glm::vec3(0.0f), LightType::Projection, scene }
{
    getComponent<ComponentBody>()->alignTo(direction);
    setTexture(texture);
    if (m_Type == LightType::Projection) {
        auto& projLights = priv::InternalScenePublicInterface::GetProjectionLights(*scene);
        projLights.emplace_back(this);
    }
    recalc_frustum_indices();
}
ProjectionLight::ProjectionLight(Handle textureHandle, const glm::vec3& direction, Scene* scene) 
    : ProjectionLight{ textureHandle.get<Texture>(), direction, scene }
{}
void ProjectionLight::recalc_frustum_points() noexcept {
    //0-3 : near plane
    //4-7 : far plane
    //both in order: top left, top right, btm left, btm right

    glm::vec2 texture_dimension_ratio = (m_Texture) ? m_Texture->sizeAsRatio() : glm::vec2(1.0f);
    std::array<glm::vec2, 4> offsets = {
        glm::vec2(-1.0f,  1.0f),
        glm::vec2( 1.0f,  1.0f),
        glm::vec2(-1.0f, -1.0f),
        glm::vec2( 1.0f, -1.0f),
    };
    for (int i = 0; i <= 3; ++i) {
        m_FrustumPoints[i].x = m_NearScale.x * offsets[i].x;
        m_FrustumPoints[i].y = m_NearScale.y * offsets[i].y;
        m_FrustumPoints[i].z = -m_Near;
    }
    for (int i = 4; i <= 7; ++i) {
        m_FrustumPoints[i].x = m_FarScale.x * offsets[i - 4].x;
        m_FrustumPoints[i].y = m_FarScale.y * offsets[i - 4].y;
        m_FrustumPoints[i].z = -m_Far;
    }
}
void ProjectionLight::recalc_frustum_indices() noexcept {
    //12 triangles * 3 indices per triangle = 36 total indices
    //clockwise
    m_FrustumIndices[0]  = 0;
    m_FrustumIndices[1]  = 1;
    m_FrustumIndices[2]  = 2;

    m_FrustumIndices[3]  = 1;
    m_FrustumIndices[4]  = 3;
    m_FrustumIndices[5]  = 2;

    m_FrustumIndices[6]  = 4;
    m_FrustumIndices[7]  = 5;
    m_FrustumIndices[8]  = 6;

    m_FrustumIndices[9]  = 5;
    m_FrustumIndices[10] = 7;
    m_FrustumIndices[11] = 6;

    m_FrustumIndices[12] = 4;
    m_FrustumIndices[13] = 0;
    m_FrustumIndices[14] = 6;

    m_FrustumIndices[15] = 0;
    m_FrustumIndices[16] = 2;
    m_FrustumIndices[17] = 6;

    m_FrustumIndices[18] = 1;
    m_FrustumIndices[19] = 5;
    m_FrustumIndices[20] = 3;

    m_FrustumIndices[21] = 5;
    m_FrustumIndices[22] = 7;
    m_FrustumIndices[23] = 3;

    m_FrustumIndices[24] = 4;
    m_FrustumIndices[25] = 5;
    m_FrustumIndices[26] = 0;

    m_FrustumIndices[27] = 5;
    m_FrustumIndices[28] = 1;
    m_FrustumIndices[29] = 0;

    m_FrustumIndices[30] = 7;
    m_FrustumIndices[31] = 6;
    m_FrustumIndices[32] = 3;

    m_FrustumIndices[33] = 6;
    m_FrustumIndices[34] = 2;
    m_FrustumIndices[35] = 3;
}
void ProjectionLight::setTexture(Handle textureHandle) noexcept {
    m_Texture = textureHandle.get<Texture>();
}
void ProjectionLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetProjectionLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}