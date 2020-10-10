#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/math/MathCompression.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/system/Engine.h>
#include <core/engine/model/ModelInstance.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

#include <core/engine/renderer/pipelines/DeferredPipeline.h>

using namespace Engine::priv;

Engine::view_ptr<RenderModule> RenderModule::RENDERER = nullptr;

unsigned int RenderModule::GLSL_VERSION;
unsigned int RenderModule::OPENGL_VERSION;

RenderModule::RenderModule(const EngineOptions& options)
    : m_GI_Pack{ Engine::Compression::pack3FloatsInto1FloatUnsigned(m_GI_Diffuse, m_GI_Specular, m_GI_Global) }
{
    m_Pipeline  = std::make_unique<Engine::priv::DeferredPipeline>(*this);
    RENDERER    = this;
}
void RenderModule::_init(){
    m_Pipeline->init();
}
void RenderModule::_render(Viewport& viewport, bool mainFunc){
    m_Pipeline->render(*this, viewport, mainFunc);
}
void RenderModule::_resize(uint w,uint h){
    m_Pipeline->onResize(w, h);
}
void RenderModule::_onFullscreen(unsigned int width, unsigned int height) {
    m_Pipeline->onFullscreen();
}
void RenderModule::_onOpenGLContextCreation(uint windowWidth,uint windowHeight,uint _glslVersion,uint _openglVersion){
    m_Pipeline->onOpenGLContextCreation(windowWidth, windowHeight, _glslVersion, _openglVersion);
}
void RenderModule::_clear2DAPICommands() {
    m_Pipeline->clear2DAPI();
}
void RenderModule::_sort2DAPICommands() {
    m_Pipeline->sort2DAPI();
}
float RenderModule::_getGIPackedData() {
    return m_GI_Pack;
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
    bool res = m_Pipeline->bind(material);
    if (res) {
        if (material->isLoaded()) {
            material->m_CustomBindFunctor(material);
        }else{
            auto* material = Material::Checkers.get<Material>();
            material->m_CustomBindFunctor(material);
        }
    }
    return res;
}
bool RenderModule::unbind(Material* material) const {
    return m_Pipeline->unbind(material);
}
void RenderModule::_genPBREnvMapData(Texture& texture, Handle convolutionTexture, Handle preEnvTexture, uint size1, uint size2){
    return m_Pipeline->generatePBRData(texture, convolutionTexture, preEnvTexture, size1, size2);
}
void Engine::Renderer::restoreDefaultOpenGLState() {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->restoreDefaultState();
}
void Engine::Renderer::restoreCurrentOpenGLState() {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->restoreCurrentState();
}
void Engine::Renderer::Settings::Lighting::enable(bool lighting){
    Engine::priv::RenderModule::RENDERER->m_Lighting = lighting;
}
void Engine::Renderer::Settings::Lighting::disable(){
    Engine::priv::RenderModule::RENDERER->m_Lighting = false;
}
float Engine::Renderer::Settings::Lighting::getGIContributionGlobal(){
    return Engine::priv::RenderModule::RENDERER->m_GI_Global;
}
void Engine::Renderer::Settings::Lighting::setGIContributionGlobal(float gi){
    Engine::priv::RenderModule::RENDERER->m_GI_Global = glm::clamp(gi,0.001f,0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDERER->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDERER->m_GI_Specular,
        Engine::priv::RenderModule::RENDERER->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return Engine::priv::RenderModule::RENDERER->m_GI_Diffuse;
}
void Engine::Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){
    Engine::priv::RenderModule::RENDERER->m_GI_Diffuse = glm::clamp(gi, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDERER->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDERER->m_GI_Specular,
        Engine::priv::RenderModule::RENDERER->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionSpecular(){
    return Engine::priv::RenderModule::RENDERER->m_GI_Specular;
}
void Engine::Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    Engine::priv::RenderModule::RENDERER->m_GI_Specular = glm::clamp(gi, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDERER->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDERER->m_GI_Specular,
        Engine::priv::RenderModule::RENDERER->m_GI_Global
    );
}
void Engine::Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    Engine::priv::RenderModule::RENDERER->m_GI_Global = glm::clamp(g, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Diffuse = glm::clamp(d, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Specular = glm::clamp(s, 0.001f, 0.999f);
    Engine::priv::RenderModule::RENDERER->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        Engine::priv::RenderModule::RENDERER->m_GI_Diffuse,
        Engine::priv::RenderModule::RENDERER->m_GI_Specular,
        Engine::priv::RenderModule::RENDERER->m_GI_Global
    );
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
    if(Engine::priv::RenderModule::RENDERER->m_AA_algorithm != algorithm){
        Engine::priv::RenderModule::RENDERER->m_AA_algorithm = algorithm;
        return true;
    }
    return false;
}
bool Engine::Renderer::stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->stencilOperation(sfail, dpfail, dppass);
}
bool Engine::Renderer::stencilMask(GLuint mask) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->stencilMask(mask);
}
bool Engine::Renderer::cullFace(GLenum state){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->cullFace(state);
}
void Engine::Renderer::Settings::clear(bool color, bool depth, bool stencil){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->clear(color, depth, stencil);
}
void Engine::Renderer::Settings::applyGlobalAnisotropicFiltering(float filtering) {
    const auto textures = Engine::priv::Core::m_Engine->m_ResourceManager.GetAllResourcesOfType<Texture>();
    for (auto& texture : textures) {
        texture->setAnisotropicFiltering(filtering);
    }
}
void Engine::Renderer::Settings::enableDrawPhysicsInfo(bool enableDrawPhysics){
    Engine::priv::RenderModule::RENDERER->m_DrawPhysicsDebug = enableDrawPhysics;
}
void Engine::Renderer::Settings::disableDrawPhysicsInfo(){
    Engine::priv::RenderModule::RENDERER->m_DrawPhysicsDebug = false;
}
void Engine::Renderer::Settings::setGamma(float gamma){
    Engine::priv::RenderModule::RENDERER->m_Gamma = gamma;
}
const float Engine::Renderer::Settings::getGamma(){
    return Engine::priv::RenderModule::RENDERER->m_Gamma;
}
bool Engine::Renderer::setDepthFunc(GLenum func){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->setDepthFunction(func);
}
bool Engine::Renderer::setViewport(float x, float y, float w, float h){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->setViewport(x, y, w, h);
}
bool Engine::Renderer::stencilFunc(GLenum func, GLint ref, GLuint mask) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->stencilFunction(func, ref, mask);
}
bool Engine::Renderer::colorMask(bool r, bool g, bool b, bool a) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->colorMask(r, g, b, a);
}
bool Engine::Renderer::clearColor(float r, float g, float b, float a) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->clearColor(r, g, b, a);
}
unsigned int Engine::Renderer::getCurrentlyBoundTextureOfType(unsigned int textureType) noexcept {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->getCurrentBoundTextureOfType(textureType);
}
bool Engine::Renderer::bindTextureForModification(TextureType textureType, GLuint textureObject) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->bindTextureForModification(textureType, textureObject);
}

bool Engine::Renderer::bindVAO(GLuint vaoObject){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->bindVAO(vaoObject);
}
bool Engine::Renderer::deleteVAO(GLuint& vaoObject) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->deleteVAO(vaoObject);
}
void Engine::Renderer::genAndBindTexture(TextureType textureType, GLuint& textureObject){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->generateAndBindTexture(textureType, textureObject);
}
void Engine::Renderer::genAndBindVAO(GLuint& vaoObject){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->generateAndBindVAO(vaoObject);
}
bool Engine::Renderer::GLEnable(GLenum apiEnum) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->enableAPI(apiEnum);
}
bool Engine::Renderer::GLDisable(GLenum apiEnum) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->disableAPI(apiEnum);
}
bool Engine::Renderer::GLEnablei(GLenum apiEnum, GLuint index) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->enableAPI_i(apiEnum, index);
}
bool Engine::Renderer::GLDisablei(GLenum apiEnum, GLuint index) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->disableAPI_i(apiEnum, index);
}
void Engine::Renderer::sendTexture(const char* location, Texture& texture, int slot){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->sendTexture(location, texture, slot);
}
void Engine::Renderer::sendTexture(const char* location, GLuint textureObject, int slot, GLuint textureTarget){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->sendTexture(location, textureObject, slot, textureTarget);
}
void Engine::Renderer::sendTextureSafe(const char* location, Texture& texture, int slot){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->sendTextureSafe(location, texture, slot);
}
void Engine::Renderer::sendTextureSafe(const char* location, GLuint textureObject, int slot, GLuint textureTarget){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->sendTextureSafe(location, textureObject, slot, textureTarget);
}
bool Engine::Renderer::bindReadFBO(const GLuint fbo){
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->bindReadFBO(fbo);
}
bool Engine::Renderer::bindDrawFBO(const GLuint fbo) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->bindDrawFBO(fbo);
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
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->bindRBO(rbo);
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
unsigned int Engine::Renderer::getUniformLoc(const char* location) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->getUniformLocation(location);
}
unsigned int Engine::Renderer::getUniformLocUnsafe(const char* location) {
    return Engine::priv::RenderModule::RENDERER->m_Pipeline->getUniformLocationUnsafe(location);
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
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Engine::Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Engine::Renderer::renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor){
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderTexture(texture, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderText(t, font, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}
void Engine::Renderer::renderFullscreenQuad() {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderFullscreenQuad();
}
void Engine::Renderer::renderFullscreenTriangle() {
    Engine::priv::RenderModule::RENDERER->m_Pipeline->renderFullscreenTriangle();
}
