#include <serenity/lights/ProjectionLight.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/utils/Utils.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/scene/Scene.h>

ProjectionLight::ProjectionLight(Scene* scene, Handle textureHandle, const glm::vec3& direction)
    : SunLight{ scene, glm::vec3(0.0f), LightType::Projection }
{
    getComponent<ComponentBody>()->alignTo(direction);
    setTexture(textureHandle);
    recalc_frustum_indices();
}
ProjectionLight::~ProjectionLight() {
}
void ProjectionLight::recalc_frustum_points() noexcept {
    //0-3 : near plane
    //4-7 : far plane
    //both in order: top left, top right, btm left, btm right

    Texture* texture = m_Texture.get<Texture>();
    glm::vec2 texture_dimension_ratio = (texture) ? texture->sizeAsRatio() : glm::vec2(1.0f);
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