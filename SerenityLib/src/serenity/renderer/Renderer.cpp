
#include <serenity/system/EngineOptions.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/math/MathCompression.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/system/Engine.h>
#include <serenity/model/ModelInstance.h>

#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/DepthOfField.h>
#include <serenity/renderer/postprocess/Bloom.h>
#include <serenity/renderer/postprocess/FXAA.h>
#include <serenity/renderer/postprocess/SMAA.h>
#include <serenity/renderer/postprocess/GodRays.h>
#include <serenity/renderer/postprocess/Fog.h>

#include <serenity/renderer/pipelines/DeferredPipeline.h>

using namespace Engine::priv;

Engine::view_ptr<RenderModule> RenderModule::RENDER_MODULE = nullptr;

RenderModule::RenderModule(const EngineOptions& options)
    : m_GI_Pack{ Engine::Compression::pack3FloatsInto1FloatUnsigned(m_GI_Diffuse, m_GI_Specular, m_GI_Global) }
{
    m_Pipeline    = std::make_unique<Engine::priv::DeferredPipeline>(*this);
    RENDER_MODULE = this;
}
void RenderModule::_init(){
    m_Pipeline->init();
}
void RenderModule::render(RenderModule& renderModule, Viewport& viewport, bool mainFunc){
    renderModule.m_Pipeline->render(renderModule, viewport, mainFunc);
}
void RenderModule::_resize(uint32_t w, uint32_t h){
    m_Pipeline->onResize(w, h);
}
void RenderModule::_onFullscreen(uint32_t width, uint32_t height) {
    m_Pipeline->onFullscreen();
}
void RenderModule::_onOpenGLContextCreation(uint32_t windowWidth, uint32_t windowHeight){
    m_Pipeline->onOpenGLContextCreation(windowWidth, windowHeight);
}
void RenderModule::_clear2DAPICommands() {
    m_Pipeline->clear2DAPI();
}
void RenderModule::_sort2DAPICommands() {
    m_Pipeline->sort2DAPI();
}

bool RenderModule::setShadowCaster(SunLight& sunLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(sunLight);
    }
    m_Pipeline->toggleShadowCaster(sunLight, isShadowCaster);
    return res;
}
bool RenderModule::setShadowCaster(PointLight& pointLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(pointLight);
    }
    m_Pipeline->toggleShadowCaster(pointLight, isShadowCaster);
    return res;
}
bool RenderModule::setShadowCaster(DirectionalLight& directionalLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(directionalLight);
    }
    m_Pipeline->toggleShadowCaster(directionalLight, isShadowCaster);
    return res;
}
bool RenderModule::setShadowCaster(SpotLight& spotLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(spotLight);
    }
    m_Pipeline->toggleShadowCaster(spotLight, isShadowCaster);
    return res;
}
bool RenderModule::setShadowCaster(RodLight& rodLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(rodLight);
    }
    m_Pipeline->toggleShadowCaster(rodLight, isShadowCaster);
    return res;
}
bool RenderModule::setShadowCaster(ProjectionLight& projectionLight, bool isShadowCaster) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(projectionLight);
    }
    m_Pipeline->toggleShadowCaster(projectionLight, isShadowCaster);
    return res;
}

bool RenderModule::bind(ModelInstance* modelInstance) const {
    bool res = m_Pipeline->bind(modelInstance);
    if (res) {
        modelInstance->m_CustomBindFunctor(modelInstance, this);
    }
    return res;
}
bool RenderModule::unbind(ModelInstance* modelInstance) const {
    modelInstance->m_CustomUnbindFunctor(modelInstance, this);
    return m_Pipeline->unbind(modelInstance);
}
bool RenderModule::bind(ShaderProgram* program) const {
    bool res = m_Pipeline->bind(program);
    if (res) {
        program->m_CustomBindFunctor(program);
    }
    return res;
}
bool RenderModule::unbind(ShaderProgram* program) const {
    return m_Pipeline->unbind(program);
}

bool RenderModule::bind(Mesh* mesh) const {
    bool res = m_Pipeline->bind(mesh);
    if (res) {
        if (mesh->isLoaded()) {
            mesh->m_CustomBindFunctor(mesh, this);
        }else{
            Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().get<Mesh>()->m_CPUData.m_VertexData->bind();
        }
    }
    return res;
}
bool RenderModule::unbind(Mesh* mesh) const {
    bool res = m_Pipeline->unbind(mesh);
    if (mesh->isLoaded()) {
        mesh->m_CustomUnbindFunctor(mesh, this);
    }else{
        Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().get<Mesh>()->m_CPUData.m_VertexData->unbind();
    }
    return true;
}
bool RenderModule::bind(Material* material) const {
    return m_Pipeline->bind(material->isLoaded() ? material : Material::Checkers.get<Material>());
}
bool RenderModule::unbind(Material* material) const {
    return m_Pipeline->unbind(material);
}
void RenderModule::_genPBREnvMapData(TextureCubemap& cubemap, Handle convolutionTexture, Handle preEnvTexture, uint32_t size1, uint32_t size2){
    return m_Pipeline->generatePBRData(cubemap, convolutionTexture, preEnvTexture, size1, size2);
}
void Engine::Renderer::restoreDefaultOpenGLState() {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->restoreDefaultState();
}
void Engine::Renderer::restoreCurrentOpenGLState() {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->restoreCurrentState();
}
void Engine::Renderer::Settings::Lighting::enable(bool lighting){
    Engine::priv::RenderModule::RENDER_MODULE->m_Lighting = lighting;
}
void Engine::Renderer::Settings::Lighting::disable(){
    Engine::priv::RenderModule::RENDER_MODULE->m_Lighting = false;
}
float Engine::Renderer::Settings::Lighting::getGIContributionGlobal(){
    return Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global;
}
void Engine::Renderer::Settings::Lighting::setGIContributionGlobal(float gi){
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global = glm::clamp(gi,0.001f,0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse;
}
void Engine::Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse = glm::clamp(gi, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionSpecular(){
    return Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular;
}
void Engine::Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular = glm::clamp(gi, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global
    );
}
void Engine::Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global = glm::clamp(g, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse = glm::clamp(d, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular = glm::clamp(s, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Specular,
        Engine::priv::RenderModule::RENDER_MODULE->m_GI_Global
    );
}
LightingAlgorithm Engine::Renderer::Settings::Lighting::getLightingAlgorithm() {
    return Engine::priv::RenderModule::RENDER_MODULE->m_LightingAlgorithm;
}
bool Engine::Renderer::Settings::Lighting::setLightingAlgorithm(LightingAlgorithm algorithm) {
    if (Engine::priv::RenderModule::RENDER_MODULE->m_LightingAlgorithm != algorithm) {
        Engine::priv::RenderModule::RENDER_MODULE->m_LightingAlgorithm = algorithm;
        return true;
    }
    return false;
}
void Engine::Renderer::Settings::enableSkybox(bool enabled) {
    Engine::priv::RenderModule::RENDER_MODULE->m_DrawSkybox = enabled;
}
void Engine::Renderer::Settings::disableSkybox() {
    Engine::priv::RenderModule::RENDER_MODULE->m_DrawSkybox = false;
}
bool Engine::Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm algorithm){
    switch (algorithm) {
        case AntiAliasingAlgorithm::None: {
            break;
        }case AntiAliasingAlgorithm::FXAA: {
            break;
        }case AntiAliasingAlgorithm::SMAA_LOW: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Low);
            break;
        }case AntiAliasingAlgorithm::SMAA_MED: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Medium);
            break;
        }case AntiAliasingAlgorithm::SMAA_HIGH: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::High);
            break;
        }case AntiAliasingAlgorithm::SMAA_ULTRA: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Ultra);
            break;
        }
    }
    if(Engine::priv::RenderModule::RENDER_MODULE->m_AA_algorithm != algorithm){
        Engine::priv::RenderModule::RENDER_MODULE->m_AA_algorithm = algorithm;
        return true;
    }
    return false;
}
bool Engine::Renderer::stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->stencilOperation(sfail, dpfail, dppass);
}
bool Engine::Renderer::stencilMask(GLuint mask) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->stencilMask(mask);
}
bool Engine::Renderer::cullFace(GLenum state){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->cullFace(state);
}
void Engine::Renderer::Settings::clear(bool color, bool depth, bool stencil){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->clear(color, depth, stencil);
}
void Engine::Renderer::Settings::applyGlobalAnisotropicFiltering(float filtering) {
    const auto textures = Engine::priv::Core::m_Engine->m_ResourceManager.GetAllResourcesOfType<Texture>(); //std::list
    std::for_each(std::cbegin(textures), std::cend(textures), [filtering](const auto& texture) {
        texture->setAnisotropicFiltering(filtering);
    });
}
void Engine::Renderer::Settings::enableDrawPhysicsInfo(bool enableDrawPhysics){
    Engine::priv::RenderModule::RENDER_MODULE->m_DrawPhysicsDebug = enableDrawPhysics;
}
void Engine::Renderer::Settings::disableDrawPhysicsInfo(){
    Engine::priv::RenderModule::RENDER_MODULE->m_DrawPhysicsDebug = false;
}
void Engine::Renderer::Settings::setGamma(float gamma){
    Engine::priv::RenderModule::RENDER_MODULE->m_Gamma = gamma;
}
const float Engine::Renderer::Settings::getGamma(){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Gamma;
}
bool Engine::Renderer::setDepthFunc(GLenum func){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->setDepthFunction(func);
}
bool Engine::Renderer::setViewport(float x, float y, float w, float h){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->setViewport(x, y, w, h);
}
bool Engine::Renderer::stencilFunc(GLenum func, GLint ref, GLuint mask) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->stencilFunction(func, ref, mask);
}
bool Engine::Renderer::colorMask(bool r, bool g, bool b, bool a) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->colorMask(r, g, b, a);
}
bool Engine::Renderer::clearColor(float r, float g, float b, float a) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->clearColor(r, g, b, a);
}
uint32_t Engine::Renderer::getCurrentlyBoundTextureOfType(uint32_t textureType) noexcept {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->getCurrentBoundTextureOfType(textureType);
}
bool Engine::Renderer::bindTextureForModification(TextureType textureType, GLuint textureObject) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->bindTextureForModification(textureType, textureObject);
}

bool Engine::Renderer::bindVAO(GLuint vaoObject){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->bindVAO(vaoObject);
}
bool Engine::Renderer::deleteVAO(GLuint& vaoObject) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->deleteVAO(vaoObject);
}
void Engine::Renderer::genAndBindTexture(TextureType textureType, GLuint& textureObject){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->generateAndBindTexture(textureType, textureObject);
}
void Engine::Renderer::genAndBindVAO(GLuint& vaoObject){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->generateAndBindVAO(vaoObject);
}
bool Engine::Renderer::GLEnable(GLenum apiEnum) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->enableAPI(apiEnum);
}
bool Engine::Renderer::GLDisable(GLenum apiEnum) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->disableAPI(apiEnum);
}
bool Engine::Renderer::GLEnablei(GLenum apiEnum, GLuint index) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->enableAPI_i(apiEnum, index);
}
bool Engine::Renderer::GLDisablei(GLenum apiEnum, GLuint index) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->disableAPI_i(apiEnum, index);
}
void Engine::Renderer::clearTexture(int unit, GLuint glTextureType) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->clearTexture(unit, glTextureType);
}
void Engine::Renderer::sendTexture(const char* location, Texture& texture, int unit){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexture(location, texture, unit);
}
void Engine::Renderer::sendTexture(const char* location, TextureCubemap& cubemap, int unit) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexture(location, cubemap, unit);
}
void Engine::Renderer::sendTexture(const char* location, GLuint textureObject, int unit, GLuint textureTarget){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexture(location, textureObject, unit, textureTarget);
}
void Engine::Renderer::sendTextureSafe(const char* location, Texture& texture, int unit){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextureSafe(location, texture, unit);
}
void Engine::Renderer::sendTextureSafe(const char* location, TextureCubemap& cubemap, int unit) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextureSafe(location, cubemap, unit);
}
void Engine::Renderer::sendTextureSafe(const char* location, GLuint textureObject, int unit, GLuint textureTarget){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextureSafe(location, textureObject, unit, textureTarget);
}
void Engine::Renderer::sendTextures(const char* location, const Texture** data, int slot, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, arrSize);
}
void Engine::Renderer::sendTextures(const char* location, const TextureCubemap** data, int slot, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, arrSize);
}
void Engine::Renderer::sendTextures(const char* location, const GLuint* data, int slot, GLuint glTextureType, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, glTextureType, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const Texture** data, int slot, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const TextureCubemap** data, int slot, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const GLuint* data, int slot, GLuint glTextureType, const int arrSize) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, glTextureType, arrSize);
}

bool Engine::Renderer::bindReadFBO(const GLuint fbo){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->bindReadFBO(fbo);
}
bool Engine::Renderer::bindDrawFBO(const GLuint fbo) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->bindDrawFBO(fbo);
}
void Engine::Renderer::bindFBO(const priv::FramebufferObject& fbo){
    Engine::Renderer::bindFBO(fbo.address());
}
bool Engine::Renderer::bindRBO(priv::RenderbufferObject& rbo){
    return Engine::Renderer::bindRBO(rbo.address());
}
void Engine::Renderer::bindFBO(const GLuint fbo){
    Engine::Renderer::bindReadFBO(fbo);
    Engine::Renderer::bindDrawFBO(fbo);
}
bool Engine::Renderer::bindRBO(const GLuint rbo){
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->bindRBO(rbo);
}
void Engine::Renderer::unbindFBO(){
    Engine::Renderer::bindFBO(GLuint(0));
}
void Engine::Renderer::unbindRBO(){
    Engine::Renderer::bindRBO(GLuint(0));
}
void Engine::Renderer::unbindReadFBO(){
    Engine::Renderer::bindReadFBO(0);
}
void Engine::Renderer::unbindDrawFBO(){
    Engine::Renderer::bindDrawFBO(0);
}
uint32_t Engine::Renderer::getUniformLoc(const char* location) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->getUniformLocation(location);
}
uint32_t Engine::Renderer::getUniformLocUnsafe(const char* location) {
    return Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->getUniformLocationUnsafe(location);
}

void Engine::Renderer::alignmentOffset(const Alignment align, float& x, float& y, const float width, const float height) {
    switch (align) {
        case Alignment::TopLeft: {
            x += width / 2;
            y -= height / 2;
            break;
        }case Alignment::TopCenter: {
            y -= height / 2;
            break;
        }case Alignment::TopRight: {
            x -= width / 2;
            y -= height / 2;
            break;
        }case Alignment::Left: {
            x += width / 2;
            break;
        }case Alignment::Center: {
            break;
        }case Alignment::Right: {
            x -= width / 2;
            break;
        }case Alignment::BottomLeft: {
            x += width / 2;
            y += height / 2;
            break;
        }case Alignment::BottomCenter: {
            y += height / 2;
            break;
        }case Alignment::BottomRight: {
            x -= width / 2;
            y += height / 2;
            break;
        }
    }
}
void Engine::Renderer::renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Engine::Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Engine::Renderer::renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor){
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderTexture(texture, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderTexture(uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderTexture(textureAddress, textureWidth, textureHeight, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderText(t, font, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}


void Engine::Renderer::renderBackgroundTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBackgroundTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Engine::Renderer::renderBackgroundRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBackgroundRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Engine::Renderer::renderBackgroundTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBackgroundTexture(texture, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBackgroundText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBackgroundText(t, font, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBackgroundBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderBackgroundBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}




void Engine::Renderer::renderFullscreenQuad() {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderFullscreenQuad();
}
void Engine::Renderer::renderFullscreenTriangle() {
    Engine::priv::RenderModule::RENDER_MODULE->m_Pipeline->renderFullscreenTriangle();
}
