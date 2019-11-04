#include <core/engine/renderer/pipelines/DeferredPipeline.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/mesh/Mesh.h>

using namespace std;
using namespace Engine::epriv;

DeferredPipeline::DeferredPipeline() {

}
DeferredPipeline::~DeferredPipeline() {

}

void DeferredPipeline::renderDirectionalLight(Camera& c, DirectionalLight& d) {
    if (!d.isActive())
        return;
    auto& body = *d.getComponent<ComponentBody>();
    const auto _forward = glm::vec3(body.forward());
    const auto& col = d.color();
    Renderer::sendUniform4("light.DataA", d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), _forward.x);
    Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, 0.0f, 0.0f);
    Renderer::sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(d.type()));
    Renderer::sendUniform1Safe("Type", 0.0f);
    Renderer::renderFullscreenTriangle();
}
void DeferredPipeline::renderSunLight(Camera& c, SunLight& s) {
    if (!s.isActive())
        return;
    auto& body = *s.getComponent<ComponentBody>();
    const auto pos = glm::vec3(body.position());
    const auto& col = s.color();
    Renderer::sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
    Renderer::sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
    Renderer::sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    Renderer::sendUniform1Safe("Type", 0.0f);

    Renderer::renderFullscreenTriangle();
}
void DeferredPipeline::renderPointLight(Camera& c, PointLight& p) {
    if (!p.isActive())
        return;
    auto& body = *p.getComponent<ComponentBody>();
    const auto pos = glm::vec3(body.position());
    const auto cull = p.getCullingRadius();
    const auto factor = 1100.0f * cull;
    const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
    if ((!c.sphereIntersectTest(pos, cull)) || (distSq > factor * factor))
        return;

    const auto& col = p.color();
    Renderer::sendUniform4("light.DataA", p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
    Renderer::sendUniform4("light.DataB", 0.0f, 0.0f, p.getConstant(), p.getLinear());
    Renderer::sendUniform4("light.DataC", p.getExponent(), pos.x, pos.y, pos.z);
    Renderer::sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(p.type()));
    Renderer::sendUniform4Safe("light.DataE", 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
    Renderer::sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    Renderer::sendUniformMatrix4("Model", model);
    //Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume
        Renderer::cullFace(GL_FRONT);
    }else{
        Renderer::cullFace(GL_BACK);
    }
    auto& pointLightMesh = *epriv::InternalMeshes::PointLightBounds;

    pointLightMesh.bind();
    pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    pointLightMesh.unbind();
    Renderer::cullFace(GL_BACK);
}
void DeferredPipeline::renderSpotLight(Camera& c, SpotLight& s) {
    return;
    auto& body = *s.entity().getComponent<ComponentBody>();
    auto pos = glm::vec3(body.position());
    auto _forward = glm::vec3(body.forward());
    const auto cull = s.getCullingRadius();
    const auto factor = 1100.0f * cull;
    const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cull) || (distSq > factor * factor))
        return;
    const auto& col = s.color();
    Renderer::sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), _forward.x);
    Renderer::sendUniform4("light.DataB", _forward.y, _forward.z, s.getConstant(), s.getLinear());
    Renderer::sendUniform4("light.DataC", s.getExponent(), pos.x, pos.y, pos.z);
    Renderer::sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    Renderer::sendUniform4Safe("light.DataE", s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
    Renderer::sendUniform2Safe("VertexShaderData", s.getCutoffOuter(), cull);
    Renderer::sendUniform1Safe("Type", 2.0f);

    const auto model = body.modelMatrix();

    Renderer::sendUniformMatrix4("Model", model);
    //Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume                                                 
        Renderer::cullFace(GL_FRONT);
    }else{
        Renderer::cullFace(GL_BACK);
    }
    auto& spotLightMesh = *epriv::InternalMeshes::SpotLightBounds;

    spotLightMesh.bind();
    spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    spotLightMesh.unbind();
    Renderer::cullFace(GL_BACK);

    Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
}
void DeferredPipeline::renderRodLight(Camera& c, RodLight& r) {
    if (!r.isActive())
        return;
    auto& body = *r.entity().getComponent<ComponentBody>();
    const auto pos = glm::vec3(body.position());
    auto cullingDistance = r.rodLength() + (r.getCullingRadius() * 2.0f);
    const auto factor = 1100.0f * cullingDistance;
    const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cullingDistance) || (distSq > factor * factor))
        return;
    const auto& col = r.color();
    const float half = r.rodLength() / 2.0f;
    const auto firstEndPt = pos + (glm::vec3(body.forward()) * half);
    const auto secndEndPt = pos - (glm::vec3(body.forward()) * half);
    Renderer::sendUniform4("light.DataA", r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
    Renderer::sendUniform4("light.DataB", firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
    Renderer::sendUniform4("light.DataC", r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
    Renderer::sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(r.type()));
    Renderer::sendUniform4Safe("light.DataE", r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
    Renderer::sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    Renderer::sendUniformMatrix4("Model", model);
    //Renderer::sendUniformMatrix4("VP", m_UBOCameraData.ViewProj);

    if (distSq <= (cullingDistance * cullingDistance)) {
        Renderer::cullFace(GL_FRONT);
    }else{
        Renderer::cullFace(GL_BACK);
    }
    auto& rodLightMesh = *epriv::InternalMeshes::RodLightBounds;

    rodLightMesh.bind();
    rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    rodLightMesh.unbind();
    Renderer::cullFace(GL_BACK);

    Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
}

void DeferredPipeline::update(const double& dt) {

}
void DeferredPipeline::render() {

}