#include <core/engine/system/EngineOptions.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/system/Engine.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

#include <core/engine/renderer/pipelines/DeferredPipeline.h>

using namespace Engine;
using namespace std;

priv::Renderer* renderManager = nullptr;

unsigned int priv::Renderer::GLSL_VERSION;
unsigned int priv::Renderer::OPENGL_VERSION;

priv::Renderer::Renderer(const EngineOptions& options){
    m_GI_Pack          = Engine::Math::pack3FloatsInto1FloatUnsigned(m_GI_Diffuse, m_GI_Specular, m_GI_Global);
    m_Pipeline         = NEW Engine::priv::DeferredPipeline(*this);
    renderManager      = this;
}
priv::Renderer::~Renderer(){
    cleanup();
}
void priv::Renderer::cleanup() {
    SAFE_DELETE(m_Pipeline);
}
void priv::Renderer::_init(){
    m_Pipeline->init();
}
void priv::Renderer::_render(Viewport& viewport,const bool mainFunc){
    m_Pipeline->render(*this, viewport, mainFunc);
}
void priv::Renderer::_resize(uint w,uint h){
    m_Pipeline->onResize(w, h);
}
void priv::Renderer::_onFullscreen(const unsigned int& width, const unsigned int& height) {
    m_Pipeline->onFullscreen();
}
void priv::Renderer::_onOpenGLContextCreation(uint windowWidth,uint windowHeight,uint _glslVersion,uint _openglVersion){
    m_Pipeline->onOpenGLContextCreation(windowWidth, windowHeight, _glslVersion, _openglVersion);
}
void priv::Renderer::_clear2DAPICommands() {
    m_Pipeline->clear2DAPI();
}
void priv::Renderer::_sort2DAPICommands() {
    m_Pipeline->sort2DAPI();
}
const float priv::Renderer::_getGIPackedData() {
    return m_GI_Pack;
}
const bool priv::Renderer::_bindShaderProgram(ShaderProgram* program){
    return m_Pipeline->bindShaderProgram(program);
}
const bool priv::Renderer::_unbindShaderProgram() {
    return m_Pipeline->unbindShaderProgram();
}
const bool priv::Renderer::_bindMaterial(Material* material){
    return m_Pipeline->bindMaterial(material);
}
const bool priv::Renderer::_unbindMaterial(){
    return m_Pipeline->unbindMaterial();
}
void priv::Renderer::_genPBREnvMapData(Texture& texture, uint size1, uint size2){
    return m_Pipeline->generatePBRData(texture, size1, size2);
}
void Renderer::restoreDefaultOpenGLState() {
    renderManager->m_Pipeline->restoreDefaultState();
}
void Renderer::restoreCurrentOpenGLState() {
    renderManager->m_Pipeline->restoreCurrentState();
}
void Renderer::Settings::Lighting::enable(const bool lighting){
    renderManager->m_Lighting = lighting;
}
void Renderer::Settings::Lighting::disable(){ 
    renderManager->m_Lighting = false;
}
const float Renderer::Settings::Lighting::getGIContributionGlobal(){
    return renderManager->m_GI_Global;
}
void Renderer::Settings::Lighting::setGIContributionGlobal(const float gi){
    renderManager->m_GI_Global = glm::clamp(gi,0.001f,0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
const float Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return renderManager->m_GI_Diffuse;
}
void Renderer::Settings::Lighting::setGIContributionDiffuse(const float gi){
    renderManager->m_GI_Diffuse = glm::clamp(gi, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
const float Renderer::Settings::Lighting::getGIContributionSpecular(){
    return renderManager->m_GI_Specular;
}
void Renderer::Settings::Lighting::setGIContributionSpecular(const float gi){
    renderManager->m_GI_Specular = glm::clamp(gi, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
void Renderer::Settings::Lighting::setGIContribution(const float g, const float d, const float s){
    renderManager->m_GI_Global = glm::clamp(g, 0.001f, 0.999f);
    renderManager->m_GI_Diffuse = glm::clamp(d, 0.001f, 0.999f);
    renderManager->m_GI_Specular = glm::clamp(s, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}

const bool Renderer::Settings::setAntiAliasingAlgorithm(const AntiAliasingAlgorithm::Algorithm& algorithm){
    switch (algorithm) {
        case AntiAliasingAlgorithm::None: {
            break;
        }case AntiAliasingAlgorithm::FXAA: {
            break;
        }case AntiAliasingAlgorithm::SMAA_LOW: {
            Renderer::smaa::setQuality(SMAAQualityLevel::Low);
            break;
        }case AntiAliasingAlgorithm::SMAA_MED: {
            Renderer::smaa::setQuality(SMAAQualityLevel::Medium);
            break;
        }case AntiAliasingAlgorithm::SMAA_HIGH: {
            Renderer::smaa::setQuality(SMAAQualityLevel::High);
            break;
        }case AntiAliasingAlgorithm::SMAA_ULTRA: {
            Renderer::smaa::setQuality(SMAAQualityLevel::Ultra);
            break;
        }default: {
            break;
        }
    }
    if(renderManager->m_AA_algorithm != algorithm){
        renderManager->m_AA_algorithm = algorithm;
        return true;
    }
    return false;
}
const bool Renderer::stencilOp(const GLenum& sfail, const GLenum& dpfail, const GLenum& dppass) {
    return renderManager->m_Pipeline->stencilOperation(sfail, dpfail, dppass);
}
const bool Renderer::stencilMask(const GLuint& mask) {
    return renderManager->m_Pipeline->stencilMask(mask);
}
const bool Renderer::cullFace(const GLenum& state){
    return renderManager->m_Pipeline->cullFace(state);
}
void Renderer::Settings::clear(const bool color, const bool depth, const bool stencil){
    return renderManager->m_Pipeline->clear(color, depth, stencil);
}
void Renderer::Settings::applyGlobalAnisotropicFiltering(const float filtering) {
    const auto textures = Engine::priv::Core::m_Engine->m_ResourceManager.GetAllResourcesOfType<Texture>();
    for (auto& texture : textures) {
        texture->setAnisotropicFiltering(filtering);
    }
}
void Renderer::Settings::enableDrawPhysicsInfo(const bool b){
    renderManager->m_DrawPhysicsDebug = b;
}
void Renderer::Settings::disableDrawPhysicsInfo(){ 
    renderManager->m_DrawPhysicsDebug = false;
}
void Renderer::Settings::setGamma(const float g){
    renderManager->m_Gamma = g;
}
const float Renderer::Settings::getGamma(){
    return renderManager->m_Gamma;
}
const bool Renderer::setDepthFunc(const GLenum& func){
    return renderManager->m_Pipeline->setDepthFunction(func);
}
const bool Renderer::setViewport(const float& x, const float& y, const float& w, const float& h){
    return renderManager->m_Pipeline->setViewport(x, y, w, h);
}
const bool Renderer::stencilFunc(const GLenum& func, const GLint& ref, const GLuint& mask) {
    return renderManager->m_Pipeline->stencilFunction(func, ref, mask);
}
const bool Renderer::colorMask(const bool& r, const bool& g, const bool& b, const bool& a) {
    return renderManager->m_Pipeline->colorMask(r, g, b, a);
}
const bool Renderer::clearColor(const float& r, const float& g, const float& b, const float& a) {
    return renderManager->m_Pipeline->clearColor(r, g, b, a);
}
const bool Renderer::bindTextureForModification(const GLuint textureType, const GLuint textureObject) {
    return renderManager->m_Pipeline->bindTextureForModification(textureType, textureObject);
}

const bool Renderer::bindVAO(const GLuint vaoObject){
    return renderManager->m_Pipeline->bindVAO(vaoObject);
}
const bool Renderer::deleteVAO(GLuint& vaoObject) {
    return renderManager->m_Pipeline->deleteVAO(vaoObject);
}
void Renderer::genAndBindTexture(const GLuint textureType, GLuint& textureObject){
    renderManager->m_Pipeline->generateAndBindTexture(textureType, textureObject);
}
void Renderer::genAndBindVAO(GLuint& vaoObject){
    renderManager->m_Pipeline->generateAndBindVAO(vaoObject);
}
const bool Renderer::GLEnable(const GLenum& apiEnum) {
    return renderManager->m_Pipeline->enableAPI(apiEnum);
}
const bool Renderer::GLDisable(const GLenum& apiEnum) {
    return renderManager->m_Pipeline->disableAPI(apiEnum);
}
const bool Renderer::GLEnablei(const GLenum& apiEnum, const GLuint& index) {
    return renderManager->m_Pipeline->enableAPI_i(apiEnum, index);
}
const bool Renderer::GLDisablei(const GLenum& apiEnum, const GLuint& index) {
    return renderManager->m_Pipeline->disableAPI_i(apiEnum, index);
}
void Renderer::sendTexture(const char* location, const Texture& texture,const int& slot){
    renderManager->m_Pipeline->sendTexture(location, texture, slot);
}
void Renderer::sendTexture(const char* location,const GLuint textureObject,const int& slot,const GLuint& textureTarget){
    renderManager->m_Pipeline->sendTexture(location, textureObject, slot, textureTarget);
}
void Renderer::sendTextureSafe(const char* location, const Texture& texture,const int& slot){
    renderManager->m_Pipeline->sendTextureSafe(location, texture, slot);
}
void Renderer::sendTextureSafe(const char* location,const GLuint textureObject,const int& slot,const GLuint& textureTarget){
    renderManager->m_Pipeline->sendTextureSafe(location, textureObject, slot, textureTarget);
}
const bool Renderer::bindReadFBO(const GLuint& fbo){
    return renderManager->m_Pipeline->bindReadFBO(fbo);
}
const bool Renderer::bindDrawFBO(const GLuint& fbo) {
    return renderManager->m_Pipeline->bindDrawFBO(fbo);
}
void Renderer::bindFBO(priv::FramebufferObject& fbo){ 
    Renderer::bindFBO(fbo.address()); 
}
const bool Renderer::bindRBO(priv::RenderbufferObject& rbo){
    return Renderer::bindRBO(rbo.address()); 
}
void Renderer::bindFBO(const GLuint& fbo){
    Renderer::bindReadFBO(fbo);
    Renderer::bindDrawFBO(fbo);
}
const bool Renderer::bindRBO(const GLuint& rbo){
    return renderManager->m_Pipeline->bindRBO(rbo);
}
void Renderer::unbindFBO(){ 
    Renderer::bindFBO(GLuint(0)); 
}
void Renderer::unbindRBO(){ 
    Renderer::bindRBO(GLuint(0)); 
}
void Renderer::unbindReadFBO(){ 
    Renderer::bindReadFBO(0); 
}
void Renderer::unbindDrawFBO(){ 
    Renderer::bindDrawFBO(0); 
}
const unsigned int Renderer::getUniformLoc(const char* location) {
    return renderManager->m_Pipeline->getUniformLocation(location);
}
const unsigned int Renderer::getUniformLocUnsafe(const char* location) {
    return renderManager->m_Pipeline->getUniformLocationUnsafe(location);
}

void Renderer::alignmentOffset(const Alignment::Type& align, float& x, float& y, const float& width, const float& height) {
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
        }default: {
            break;
        }
    }
}
void Renderer::renderTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, const float width, const float height, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Renderer::renderTexture(const Texture& tex, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const Alignment::Type& align, const glm::vec4& scissor){
    renderManager->m_Pipeline->renderTexture(tex, p, c, a, s, d, align, scissor);
}
void Renderer::renderText(const string& t, const Font& fnt, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type& align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderText(t, fnt, p, c, a, s, d, align, scissor);
}
void Renderer::renderBorder(const float borderSize, const glm::vec2& pos, const glm::vec4& col, const float w, const float h, const float angle, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}

void Renderer::renderFullscreenQuad() {
    renderManager->m_Pipeline->renderFullscreenQuad();
}

void Renderer::renderFullscreenTriangle() {
    renderManager->m_Pipeline->renderFullscreenTriangle();
}
