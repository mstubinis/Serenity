#include <core/engine/system/EngineOptions.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/shaders/ShaderProgram.h>
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
void priv::Renderer::_render(Viewport& viewport, bool mainFunc){
    m_Pipeline->render(*this, viewport, mainFunc);
}
void priv::Renderer::_resize(uint w,uint h){
    m_Pipeline->onResize(w, h);
}
void priv::Renderer::_onFullscreen(unsigned int width, unsigned int height) {
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
float priv::Renderer::_getGIPackedData() {
    return m_GI_Pack;
}
bool priv::Renderer::bind(ModelInstance* modelInstance) const {
    bool res = m_Pipeline->bind(modelInstance);
    if (res) {
        modelInstance->m_CustomBindFunctor(modelInstance, this);
    }
    return res;
}
bool priv::Renderer::unbind(ModelInstance* modelInstance) const {
    modelInstance->m_CustomUnbindFunctor(modelInstance, this);
    return m_Pipeline->unbind(modelInstance);
}
bool priv::Renderer::bind(ShaderProgram* program) const {
    bool res = m_Pipeline->bind(program);
    if (res) {
        program->m_CustomBindFunctor(program);
    }
    return res;
}
bool priv::Renderer::unbind(ShaderProgram* program) const {
    return m_Pipeline->unbind(program);
}

bool priv::Renderer::bind(Mesh* mesh) const {
    bool res = m_Pipeline->bind(mesh);
    if (res) {
        if (mesh->isLoaded()) {
            mesh->m_CustomBindFunctor(mesh, this);
        }else{
            Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().m_VertexData->bind();
        }
    }
    return res;
}
bool priv::Renderer::unbind(Mesh* mesh) const {
    bool res = m_Pipeline->unbind(mesh);
    if (mesh->isLoaded()) {
        mesh->m_CustomUnbindFunctor(mesh, this);
    }else{
        Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().m_VertexData->unbind();
    }
    return true;
}
bool priv::Renderer::bind(Material* material) const {
    bool res = m_Pipeline->bind(material);
    if (res) {
        if (material->isLoaded()) {
            material->m_CustomBindFunctor(material);
        }else{
            Material::Checkers->m_CustomBindFunctor(Material::Checkers);
        }
    }
    return res;
}
bool priv::Renderer::unbind(Material* material) const {
    return m_Pipeline->unbind(material);
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
void Renderer::Settings::Lighting::enable(bool lighting){
    renderManager->m_Lighting = lighting;
}
void Renderer::Settings::Lighting::disable(){ 
    renderManager->m_Lighting = false;
}
float Renderer::Settings::Lighting::getGIContributionGlobal(){
    return renderManager->m_GI_Global;
}
void Renderer::Settings::Lighting::setGIContributionGlobal(float gi){
    renderManager->m_GI_Global = glm::clamp(gi,0.001f,0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
float Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return renderManager->m_GI_Diffuse;
}
void Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){
    renderManager->m_GI_Diffuse = glm::clamp(gi, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
float Renderer::Settings::Lighting::getGIContributionSpecular(){
    return renderManager->m_GI_Specular;
}
void Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    renderManager->m_GI_Specular = glm::clamp(gi, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}
void Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    renderManager->m_GI_Global = glm::clamp(g, 0.001f, 0.999f);
    renderManager->m_GI_Diffuse = glm::clamp(d, 0.001f, 0.999f);
    renderManager->m_GI_Specular = glm::clamp(s, 0.001f, 0.999f);
    renderManager->m_GI_Pack = Math::pack3FloatsInto1FloatUnsigned(renderManager->m_GI_Diffuse, renderManager->m_GI_Specular, renderManager->m_GI_Global);
}

bool Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm algorithm){
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
bool Renderer::stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    return renderManager->m_Pipeline->stencilOperation(sfail, dpfail, dppass);
}
bool Renderer::stencilMask(GLuint mask) {
    return renderManager->m_Pipeline->stencilMask(mask);
}
bool Renderer::cullFace(GLenum state){
    return renderManager->m_Pipeline->cullFace(state);
}
void Renderer::Settings::clear(bool color, bool depth, bool stencil){
    return renderManager->m_Pipeline->clear(color, depth, stencil);
}
void Renderer::Settings::applyGlobalAnisotropicFiltering(float filtering) {
    const auto textures = Engine::priv::Core::m_Engine->m_ResourceManager.GetAllResourcesOfType<Texture>();
    for (auto& texture : textures) {
        texture->setAnisotropicFiltering(filtering);
    }
}
void Renderer::Settings::enableDrawPhysicsInfo(bool enableDrawPhysics){
    renderManager->m_DrawPhysicsDebug = enableDrawPhysics;
}
void Renderer::Settings::disableDrawPhysicsInfo(){ 
    renderManager->m_DrawPhysicsDebug = false;
}
void Renderer::Settings::setGamma(float gamma){
    renderManager->m_Gamma = gamma;
}
const float Renderer::Settings::getGamma(){
    return renderManager->m_Gamma;
}
bool Renderer::setDepthFunc(GLenum func){
    return renderManager->m_Pipeline->setDepthFunction(func);
}
bool Renderer::setViewport(float x, float y, float w, float h){
    return renderManager->m_Pipeline->setViewport(x, y, w, h);
}
bool Renderer::stencilFunc(GLenum func, GLint ref, GLuint mask) {
    return renderManager->m_Pipeline->stencilFunction(func, ref, mask);
}
bool Renderer::colorMask(bool r, bool g, bool b, bool a) {
    return renderManager->m_Pipeline->colorMask(r, g, b, a);
}
bool Renderer::clearColor(float r, float g, float b, float a) {
    return renderManager->m_Pipeline->clearColor(r, g, b, a);
}
bool Renderer::bindTextureForModification(GLuint textureType, GLuint textureObject) {
    return renderManager->m_Pipeline->bindTextureForModification(textureType, textureObject);
}

bool Renderer::bindVAO(GLuint vaoObject){
    return renderManager->m_Pipeline->bindVAO(vaoObject);
}
bool Renderer::deleteVAO(GLuint& vaoObject) {
    return renderManager->m_Pipeline->deleteVAO(vaoObject);
}
void Renderer::genAndBindTexture(GLuint textureType, GLuint& textureObject){
    renderManager->m_Pipeline->generateAndBindTexture(textureType, textureObject);
}
void Renderer::genAndBindVAO(GLuint& vaoObject){
    renderManager->m_Pipeline->generateAndBindVAO(vaoObject);
}
bool Renderer::GLEnable(GLenum apiEnum) {
    return renderManager->m_Pipeline->enableAPI(apiEnum);
}
bool Renderer::GLDisable(GLenum apiEnum) {
    return renderManager->m_Pipeline->disableAPI(apiEnum);
}
bool Renderer::GLEnablei(GLenum apiEnum, GLuint index) {
    return renderManager->m_Pipeline->enableAPI_i(apiEnum, index);
}
bool Renderer::GLDisablei(GLenum apiEnum, GLuint index) {
    return renderManager->m_Pipeline->disableAPI_i(apiEnum, index);
}
void Renderer::sendTexture(const char* location, Texture& texture, int slot){
    renderManager->m_Pipeline->sendTexture(location, texture, slot);
}
void Renderer::sendTexture(const char* location, GLuint textureObject, int slot, GLuint textureTarget){
    renderManager->m_Pipeline->sendTexture(location, textureObject, slot, textureTarget);
}
void Renderer::sendTextureSafe(const char* location, Texture& texture, int slot){
    renderManager->m_Pipeline->sendTextureSafe(location, texture, slot);
}
void Renderer::sendTextureSafe(const char* location, GLuint textureObject, int slot, GLuint textureTarget){
    renderManager->m_Pipeline->sendTextureSafe(location, textureObject, slot, textureTarget);
}
bool Renderer::bindReadFBO(const GLuint fbo){
    return renderManager->m_Pipeline->bindReadFBO(fbo);
}
bool Renderer::bindDrawFBO(const GLuint fbo) {
    return renderManager->m_Pipeline->bindDrawFBO(fbo);
}
void Renderer::bindFBO(const priv::FramebufferObject& fbo){ 
    Renderer::bindFBO(fbo.address()); 
}
bool Renderer::bindRBO(priv::RenderbufferObject& rbo){
    return Renderer::bindRBO(rbo.address()); 
}
void Renderer::bindFBO(const GLuint fbo){
    Renderer::bindReadFBO(fbo);
    Renderer::bindDrawFBO(fbo);
}
bool Renderer::bindRBO(const GLuint rbo){
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
unsigned int Renderer::getUniformLoc(const char* location) {
    return renderManager->m_Pipeline->getUniformLocation(location);
}
unsigned int Renderer::getUniformLocUnsafe(const char* location) {
    return renderManager->m_Pipeline->getUniformLocationUnsafe(location);
}

void Renderer::alignmentOffset(const Alignment::Type align, float& x, float& y, const float width, const float height) {
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
void Renderer::renderTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment::Type align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment::Type align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Renderer::renderTexture(Texture& tex, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment::Type align, const glm::vec4& scissor){
    renderManager->m_Pipeline->renderTexture(tex, p, c, a, s, d, align, scissor);
}
void Renderer::renderText(const string& t, const Font& fnt, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment::Type align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderText(t, fnt, p, c, a, s, d, align, scissor);
}
void Renderer::renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment::Type align, const glm::vec4& scissor) {
    renderManager->m_Pipeline->renderBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}

void Renderer::renderFullscreenQuad() {
    renderManager->m_Pipeline->renderFullscreenQuad();
}

void Renderer::renderFullscreenTriangle() {
    renderManager->m_Pipeline->renderFullscreenTriangle();
}
