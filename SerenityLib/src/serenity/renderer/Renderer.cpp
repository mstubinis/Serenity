
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


namespace {
    Engine::view_ptr<RenderModule> RENDER_MODULE = nullptr;

    std::vector<float>    BufferFloat;
    std::vector<double>   BufferDouble;
    std::vector<int>      BufferInt;
    std::vector<uint32_t> BufferUInt;

    template<class T> inline std::vector<T>&        getBuffer() noexcept = delete;
    template<>        inline std::vector<float>&    getBuffer() noexcept { return BufferFloat; }
    template<>        inline std::vector<double>&   getBuffer() noexcept { return BufferDouble; }
    template<>        inline std::vector<int>&      getBuffer() noexcept { return BufferInt; }
    template<>        inline std::vector<uint32_t>& getBuffer() noexcept { return BufferUInt; }

    inline uint32_t getNumElements(const auto& matrix) noexcept {
        return matrix.length() * matrix[0].length();
    }

    inline void sendUniformValues1(const char* location, const auto x, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), x);
    }
    inline void sendUniformValues2(const char* location, const auto x, const auto y, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), x, y);
    }
    inline void sendUniformValues3(const char* location, const auto x, const auto y, const auto z, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), x, y, z);
    }
    inline void sendUniformValues4(const char* location, const auto x, const auto y, const auto z, const auto w, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), x, y, z, w);
    }
    template<class ... ARGS>
    void executeSafeFunction(const char* location, const auto func, ARGS&&... args) {
        const auto o = Engine::Renderer::getUniformLoc(location);
        if (o == -1)
            return;
        func(o, std::forward<ARGS>(args)...);
    }


    inline void sendUniformValues1Safe(const char* location, const auto x, const auto func) {
        executeSafeFunction(location, func, x);
    }
    inline void sendUniformValues2Safe(const char* location, const auto x, const auto y, const auto func) {
        executeSafeFunction(location, func, x, y);
    }
    inline void sendUniformValues3Safe(const char* location, const auto x, const auto y, const auto z, const auto func) {
        executeSafeFunction(location, func, x, y, z);
    }
    inline void sendUniformValues4Safe(const char* location, const auto x, const auto y, const auto z, const auto w, const auto func) {
        executeSafeFunction(location, func, x, y, z, w);
    }
    inline void sendUniformValues(const char* location, auto* values, const uint32_t i, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), i, glm::value_ptr(values[0]));
    }
    inline void sendUniformValuesSafe(const char* location, auto* values, const uint32_t i, const auto func) {
        executeSafeFunction(location, func, i, glm::value_ptr(values[0]));
    }
    inline void sendUniformValuesBasic(const char* location, auto* values, const uint32_t i, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(location), i, values);
    }
    inline void sendUniformValuesSafeBasic(const char* location, auto* values, const uint32_t i, const auto func) {
        executeSafeFunction(location, func, i, values);
    }



    inline void sendUniformMatrix(const char* l, const auto& matrix, const auto func) {
        func(Engine::Renderer::getUniformLocUnsafe(l), 1, 0, glm::value_ptr(matrix));
    }
    template<class VAL_TYPE>
    void sendUniformMatrixV(const char* location, const auto& matrices, const uint32_t count, const auto func) {
        const uint32_t numElements = getNumElements(matrices[0]);
        auto& buffer = getBuffer<VAL_TYPE>();
        buffer.resize(matrices.size() * numElements);
        uint32_t inc = 0;
        for (auto& matrix : matrices) {
            auto* values = glm::value_ptr(matrix);
            for (uint32_t i = 0; i < numElements; ++i) {
                buffer[inc] = values[i];
                ++inc;
            }
        }
        func(Engine::Renderer::getUniformLoc(location), count, 0, &buffer[0]);
    }
    inline void sendUniformMatrixSafe(const char* location, const auto& matrix, const auto func) {
        executeSafeFunction(location, func, 1, 0, glm::value_ptr(matrix));
    }
    template<class VAL_TYPE>
    void sendUniformMatrixVSafe(const char* location, const auto& matrices, const uint32_t count, const auto func) {
        const auto o = Engine::Renderer::getUniformLoc(location);
        if (o == -1 || matrices.size() == 0)
            return;
        const uint32_t numElements = getNumElements(matrices[0]);
        auto& buffer = getBuffer<VAL_TYPE>();
        buffer.resize(matrices.size() * numElements);
        uint32_t inc = 0;
        for (auto& matrix : matrices) {
            auto* values = glm::value_ptr(matrix);
            for (uint32_t i = 0; i < numElements; ++i) {
                buffer[inc] = values[i];
                ++inc;
            }
        }
        func(o, count, 0, &buffer[0]);
    }
}


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
bool RenderModule::setShadowCaster(DirectionalLight& directionalLight, bool isShadowCaster, uint32_t shadowMapWidth, uint32_t shadowMapSize, LightShadowFrustumType frustumType, float nearFactor, float farFactor) {
    bool res = false;
    if (isShadowCaster) {
        res = m_Pipeline->buildShadowCaster(directionalLight, shadowMapWidth, shadowMapSize, frustumType, nearFactor, farFactor);
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
        } else {
            Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh().get<Mesh>()->m_CPUData.m_VertexData->bind();
        }
    }
    return res;
}
bool RenderModule::unbind(Mesh* mesh) const {
    bool res = m_Pipeline->unbind(mesh);
    if (mesh->isLoaded()) {
        mesh->m_CustomUnbindFunctor(mesh, this);
    } else {
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
    RENDER_MODULE->m_Pipeline->restoreDefaultState();
}
void Engine::Renderer::restoreCurrentOpenGLState() {
    RENDER_MODULE->m_Pipeline->restoreCurrentState();
}
void Engine::Renderer::Settings::Lighting::enable(bool lighting){
    RENDER_MODULE->m_Lighting = lighting;
}
void Engine::Renderer::Settings::Lighting::disable(){
    RENDER_MODULE->m_Lighting = false;
}
float Engine::Renderer::Settings::Lighting::getGIContributionGlobal(){
    return RENDER_MODULE->m_GI_Global;
}
void Engine::Renderer::Settings::Lighting::setGIContributionGlobal(float gi){
    RENDER_MODULE->m_GI_Global = glm::clamp(gi,0.001f,0.999f);
    RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        RENDER_MODULE->m_GI_Diffuse,
        RENDER_MODULE->m_GI_Specular,
        RENDER_MODULE->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionDiffuse(){
    return RENDER_MODULE->m_GI_Diffuse;
}
void Engine::Renderer::Settings::Lighting::setGIContributionDiffuse(float gi){
    RENDER_MODULE->m_GI_Diffuse = glm::clamp(gi, 0.001f, 0.999f);
    RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        RENDER_MODULE->m_GI_Diffuse,
        RENDER_MODULE->m_GI_Specular,
        RENDER_MODULE->m_GI_Global
    );
}
float Engine::Renderer::Settings::Lighting::getGIContributionSpecular(){
    return RENDER_MODULE->m_GI_Specular;
}
void Engine::Renderer::Settings::Lighting::setGIContributionSpecular(float gi){
    RENDER_MODULE->m_GI_Specular = glm::clamp(gi, 0.001f, 0.999f);
    RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        RENDER_MODULE->m_GI_Diffuse,
        RENDER_MODULE->m_GI_Specular,
        RENDER_MODULE->m_GI_Global
    );
}
void Engine::Renderer::Settings::Lighting::setGIContribution(float g, float d, float s){
    RENDER_MODULE->m_GI_Global = glm::clamp(g, 0.001f, 0.999f);
    RENDER_MODULE->m_GI_Diffuse = glm::clamp(d, 0.001f, 0.999f);
    RENDER_MODULE->m_GI_Specular = glm::clamp(s, 0.001f, 0.999f);
    RENDER_MODULE->m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(
        RENDER_MODULE->m_GI_Diffuse,
        RENDER_MODULE->m_GI_Specular,
        RENDER_MODULE->m_GI_Global
    );
}
LightingAlgorithm Engine::Renderer::Settings::Lighting::getLightingAlgorithm() {
    return RENDER_MODULE->m_LightingAlgorithm;
}
bool Engine::Renderer::Settings::Lighting::setLightingAlgorithm(LightingAlgorithm algorithm) {
    if (RENDER_MODULE->m_LightingAlgorithm != algorithm) {
        RENDER_MODULE->m_LightingAlgorithm = algorithm;
        return true;
    }
    return false;
}
void Engine::Renderer::Settings::enableSkybox(bool enabled) {
    RENDER_MODULE->m_DrawSkybox = enabled;
}
void Engine::Renderer::Settings::disableSkybox() {
    RENDER_MODULE->m_DrawSkybox = false;
}
bool Engine::Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm algorithm){
    switch (algorithm) {
        case AntiAliasingAlgorithm::None: {
            break;
        } case AntiAliasingAlgorithm::FXAA: {
            break;
        } case AntiAliasingAlgorithm::SMAA_LOW: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Low);
            break;
        } case AntiAliasingAlgorithm::SMAA_MED: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Medium);
            break;
        } case AntiAliasingAlgorithm::SMAA_HIGH: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::High);
            break;
        } case AntiAliasingAlgorithm::SMAA_ULTRA: {
            Engine::Renderer::smaa::setQuality(SMAAQualityLevel::Ultra);
            break;
        }
    }
    if (RENDER_MODULE->m_AA_algorithm != algorithm) {
        RENDER_MODULE->m_AA_algorithm = algorithm;
        return true;
    }
    return false;
}
bool Engine::Renderer::stencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    return RENDER_MODULE->m_Pipeline->stencilOperation(sfail, dpfail, dppass);
}
bool Engine::Renderer::stencilMask(GLuint mask) {
    return RENDER_MODULE->m_Pipeline->stencilMask(mask);
}
bool Engine::Renderer::cullFace(GLenum state){
    return RENDER_MODULE->m_Pipeline->cullFace(state);
}
void Engine::Renderer::Settings::clear(bool color, bool depth, bool stencil){
    return RENDER_MODULE->m_Pipeline->clear(color, depth, stencil);
}
void Engine::Renderer::Settings::applyGlobalAnisotropicFiltering(float filtering) {
    const auto textures = Engine::priv::Core::m_Engine->m_ResourceManager.GetAllResourcesOfType<Texture>(); // std::list - expensive
    std::for_each(std::cbegin(textures), std::cend(textures), [filtering](const auto& texture) {
        texture->setAnisotropicFiltering(filtering);
    });
}
void Engine::Renderer::Settings::enableDrawPhysicsInfo(bool enableDrawPhysics){
    RENDER_MODULE->m_DrawPhysicsDebug = enableDrawPhysics;
}
void Engine::Renderer::Settings::disableDrawPhysicsInfo(){
    RENDER_MODULE->m_DrawPhysicsDebug = false;
}
void Engine::Renderer::Settings::setGamma(float gamma){
    RENDER_MODULE->m_Gamma = gamma;
}
const float Engine::Renderer::Settings::getGamma(){
    return RENDER_MODULE->m_Gamma;
}
bool Engine::Renderer::setDepthFunc(GLenum func){
    return RENDER_MODULE->m_Pipeline->setDepthFunction(func);
}
bool Engine::Renderer::setViewport(float x, float y, float w, float h){
    return RENDER_MODULE->m_Pipeline->setViewport(x, y, w, h);
}
bool Engine::Renderer::stencilFunc(GLenum func, GLint ref, GLuint mask) {
    return RENDER_MODULE->m_Pipeline->stencilFunction(func, ref, mask);
}
bool Engine::Renderer::colorMask(bool r, bool g, bool b, bool a) {
    return RENDER_MODULE->m_Pipeline->colorMask(r, g, b, a);
}
bool Engine::Renderer::clearColor(float r, float g, float b, float a) {
    return RENDER_MODULE->m_Pipeline->clearColor(r, g, b, a);
}
uint32_t Engine::Renderer::getCurrentlyBoundTextureOfType(uint32_t textureType) noexcept {
    return RENDER_MODULE->m_Pipeline->getCurrentBoundTextureOfType(textureType);
}
bool Engine::Renderer::bindTextureForModification(TextureType textureType, GLuint textureObject) {
    return RENDER_MODULE->m_Pipeline->bindTextureForModification(textureType, textureObject);
}

bool Engine::Renderer::bindVAO(GLuint vaoObject){
    return RENDER_MODULE->m_Pipeline->bindVAO(vaoObject);
}
bool Engine::Renderer::deleteVAO(GLuint& vaoObject) {
    return RENDER_MODULE->m_Pipeline->deleteVAO(vaoObject);
}
void Engine::Renderer::genAndBindTexture(TextureType textureType, GLuint& textureObject){
    RENDER_MODULE->m_Pipeline->generateAndBindTexture(textureType, textureObject);
}
void Engine::Renderer::genAndBindVAO(GLuint& vaoObject){
    RENDER_MODULE->m_Pipeline->generateAndBindVAO(vaoObject);
}
bool Engine::Renderer::GLEnable(GLenum apiEnum) {
    return RENDER_MODULE->m_Pipeline->enableAPI(apiEnum);
}
bool Engine::Renderer::GLDisable(GLenum apiEnum) {
    return RENDER_MODULE->m_Pipeline->disableAPI(apiEnum);
}
bool Engine::Renderer::GLEnablei(GLenum apiEnum, GLuint index) {
    return RENDER_MODULE->m_Pipeline->enableAPI_i(apiEnum, index);
}
bool Engine::Renderer::GLDisablei(GLenum apiEnum, GLuint index) {
    return RENDER_MODULE->m_Pipeline->disableAPI_i(apiEnum, index);
}
void Engine::Renderer::clearTexture(int unit, GLuint glTextureType) {
    RENDER_MODULE->m_Pipeline->clearTexture(unit, glTextureType);
}
void Engine::Renderer::sendTexture(const char* location, Texture& texture, int unit){
    RENDER_MODULE->m_Pipeline->sendTexture(location, texture, unit);
}
void Engine::Renderer::sendTexture(const char* location, TextureCubemap& cubemap, int unit) {
    RENDER_MODULE->m_Pipeline->sendTexture(location, cubemap, unit);
}
void Engine::Renderer::sendTexture(const char* location, GLuint textureObject, int unit, GLuint textureTarget){
    RENDER_MODULE->m_Pipeline->sendTexture(location, textureObject, unit, textureTarget);
}
void Engine::Renderer::sendTextureSafe(const char* location, Texture& texture, int unit){
    RENDER_MODULE->m_Pipeline->sendTextureSafe(location, texture, unit);
}
void Engine::Renderer::sendTextureSafe(const char* location, TextureCubemap& cubemap, int unit) {
    RENDER_MODULE->m_Pipeline->sendTextureSafe(location, cubemap, unit);
}
void Engine::Renderer::sendTextureSafe(const char* location, GLuint textureObject, int unit, GLuint textureTarget){
    RENDER_MODULE->m_Pipeline->sendTextureSafe(location, textureObject, unit, textureTarget);
}
void Engine::Renderer::sendTextures(const char* location, const Texture** data, int slot, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, arrSize);
}
void Engine::Renderer::sendTextures(const char* location, const TextureCubemap** data, int slot, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, arrSize);
}
void Engine::Renderer::sendTextures(const char* location, const GLuint* data, int slot, GLuint glTextureType, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTextures(location, data, slot, glTextureType, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const Texture** data, int slot, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const TextureCubemap** data, int slot, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, arrSize);
}
void Engine::Renderer::sendTexturesSafe(const char* location, const GLuint* data, int slot, GLuint glTextureType, const int arrSize) {
    RENDER_MODULE->m_Pipeline->sendTexturesSafe(location, data, slot, glTextureType, arrSize);
}

bool Engine::Renderer::bindReadFBO(const GLuint fbo){
    return RENDER_MODULE->m_Pipeline->bindReadFBO(fbo);
}
bool Engine::Renderer::bindDrawFBO(const GLuint fbo) {
    return RENDER_MODULE->m_Pipeline->bindDrawFBO(fbo);
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
    return RENDER_MODULE->m_Pipeline->bindRBO(rbo);
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
    return RENDER_MODULE->m_Pipeline->getUniformLocation(location);
}
uint32_t Engine::Renderer::getUniformLocUnsafe(const char* location) {
    return RENDER_MODULE->m_Pipeline->getUniformLocationUnsafe(location);
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
    RENDER_MODULE->m_Pipeline->renderTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Engine::Renderer::renderRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Engine::Renderer::renderTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor){
    RENDER_MODULE->m_Pipeline->renderTexture(texture, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderTexture(uint32_t textureAddress,int textureWidth, int textureHeight, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderTexture(textureAddress, textureWidth, textureHeight, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderText(t, font, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}


void Engine::Renderer::renderBackgroundTriangle(const glm::vec2& position, const glm::vec4& color, float angle, float width, float height, float depth, Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBackgroundTriangle(position, color, angle, width, height, depth, align, scissor);
}
void Engine::Renderer::renderBackgroundRectangle(const glm::vec2& pos, const glm::vec4& col, float width, float height, float angle, float depth, const Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBackgroundRectangle(pos, col, width, height, angle, depth, align, scissor);
}
void Engine::Renderer::renderBackgroundTexture(Handle texture, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBackgroundTexture(texture, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBackgroundText(const std::string& t, Handle font, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBackgroundText(t, font, p, c, a, s, d, align, scissor);
}
void Engine::Renderer::renderBackgroundBorder(float borderSize, const glm::vec2& pos, const glm::vec4& col, float w, float h, float angle, float depth, Alignment align, const glm::vec4& scissor) {
    RENDER_MODULE->m_Pipeline->renderBackgroundBorder(borderSize, pos, col, w, h, angle, depth, align, scissor);
}



void Engine::Renderer::renderFullscreenQuad() {
    const auto winSize = glm::vec2{ Engine::Resources::getWindowSize() };
    RENDER_MODULE->m_Pipeline->renderFullscreenQuad(winSize.x, winSize.y);
}
void Engine::Renderer::renderFullscreenQuad(float width, float height) {
    RENDER_MODULE->m_Pipeline->renderFullscreenQuad(width, height);
}
void Engine::Renderer::renderFullscreenTriangle() {
    RENDER_MODULE->m_Pipeline->renderFullscreenTriangle();
}



#pragma region UniformSending
void Engine::Renderer::sendUniform1(const char* l, double x) {
    sendUniformValues1(l, x, glUniform1d);
}
void Engine::Renderer::sendUniform1(const char* l, int x) {
    sendUniformValues1(l, x, glUniform1i);
}
void Engine::Renderer::sendUniform1(const char* l, float x) {
    sendUniformValues1(l, x, glUniform1f);
}
void Engine::Renderer::sendUniform1(const char* l, uint32_t x) {
    sendUniformValues1(l, x, glUniform1ui);
}
void Engine::Renderer::sendUniform1Safe(const char* l, double x) {
    sendUniformValues1Safe(l, x, glUniform1d);
}
void Engine::Renderer::sendUniform1Safe(const char* l, int x) {
    sendUniformValues1Safe(l, x, glUniform1i);
}
void Engine::Renderer::sendUniform1Safe(const char* l, float x) {
    sendUniformValues1Safe(l, x, glUniform1f);
}
void Engine::Renderer::sendUniform1Safe(const char* l, uint32_t x) {
    sendUniformValues1Safe(l, x, glUniform1ui);
}
void Engine::Renderer::sendUniform1v(const char* l, const std::vector<double>& d, const uint32_t i) {
    sendUniformValuesBasic(l, d.data(), i, glUniform1dv);
}
void Engine::Renderer::sendUniform1v(const char* l, const std::vector<int>& d, const uint32_t i) {
    sendUniformValuesBasic(l, d.data(), i, glUniform1iv);
}
void Engine::Renderer::sendUniform1v(const char* l, const std::vector<float>& d, const uint32_t i) {
    sendUniformValuesBasic(l, d.data(), i, glUniform1fv);
}
void Engine::Renderer::sendUniform1v(const char* l, double* d, const uint32_t i) {
    sendUniformValuesBasic(l, d, i, glUniform1dv);
}
void Engine::Renderer::sendUniform1v(const char* l, int* d, const uint32_t i) {
    sendUniformValuesBasic(l, d, i, glUniform1iv);
}
void Engine::Renderer::sendUniform1v(const char* l, float* d, const uint32_t i) {
    sendUniformValuesBasic(l, d, i, glUniform1fv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, const std::vector<float>& d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d.data(), i, glUniform1fv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, const std::vector<double>& d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d.data(), i, glUniform1dv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, const std::vector<int>& d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d.data(), i, glUniform1iv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, float* d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d, i, glUniform1fv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, double* d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d, i, glUniform1dv);
}
void Engine::Renderer::sendUniform1vSafe(const char* l, int* d, const uint32_t i) {
    sendUniformValuesSafeBasic(l, d, i, glUniform1iv);
}





void Engine::Renderer::sendUniform2(const char* l, const glm::dvec2& v) {
    sendUniformValues2(l, v.x, v.y, glUniform2d);
}
void Engine::Renderer::sendUniform2(const char* l, const glm::ivec2& v) {
    sendUniformValues2(l, v.x, v.y, glUniform2i);
}
void Engine::Renderer::sendUniform2(const char* l, const glm::vec2& v) {
    sendUniformValues2(l, v.x, v.y, glUniform2f);
}
void Engine::Renderer::sendUniform2(const char* l, const glm::uvec2& v) {
    sendUniformValues2(l, v.x, v.y, glUniform2ui);
}
void Engine::Renderer::sendUniform2Safe(const char* l, const glm::dvec2& v) {
    sendUniformValues2Safe(l, v.x, v.y, glUniform2d);
}
void Engine::Renderer::sendUniform2Safe(const char* l, const glm::ivec2& v) {
    sendUniformValues2Safe(l, v.x, v.y, glUniform2i);
}
void Engine::Renderer::sendUniform2Safe(const char* l, const glm::vec2& v) {
    sendUniformValues2Safe(l, v.x, v.y, glUniform2f);
}
void Engine::Renderer::sendUniform2Safe(const char* l, const glm::uvec2& v) {
    sendUniformValues2Safe(l, v.x, v.y, glUniform2ui);
}
void Engine::Renderer::sendUniform2(const char* l, double x, double y) {
    sendUniformValues2(l, x, y, glUniform2d);
}
void Engine::Renderer::sendUniform2(const char* l, int x, int y) {
    sendUniformValues2(l, x, y, glUniform2i);
}
void Engine::Renderer::sendUniform2(const char* l, float x, float y) {
    sendUniformValues2(l, x, y, glUniform2f);
}
void Engine::Renderer::sendUniform2(const char* l, uint32_t x, uint32_t y) {
    sendUniformValues2(l, x, y, glUniform2ui);
}
void Engine::Renderer::sendUniform2Safe(const char* l, double x, double y) {
    sendUniformValues2Safe(l, x, y, glUniform2d);
}
void Engine::Renderer::sendUniform2Safe(const char* l, int x, int y) {
    sendUniformValues2Safe(l, x, y, glUniform2i);
}
void Engine::Renderer::sendUniform2Safe(const char* l, float x, float y) {
    sendUniformValues2Safe(l, x, y, glUniform2f);
}
void Engine::Renderer::sendUniform2Safe(const char* l, uint32_t x, uint32_t y) {
    sendUniformValues2Safe(l, x, y, glUniform2ui);
}
void Engine::Renderer::sendUniform2v(const char* l, const std::vector<glm::dvec2>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform2dv);
}
void Engine::Renderer::sendUniform2v(const char* l, const std::vector<glm::ivec2>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform2iv);
}
void Engine::Renderer::sendUniform2v(const char* l, const std::vector<glm::vec2>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform2fv);
}
void Engine::Renderer::sendUniform2v(const char* l, glm::dvec2* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform2dv);
}
void Engine::Renderer::sendUniform2v(const char* l, glm::ivec2* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform2iv);
}
void Engine::Renderer::sendUniform2v(const char* l, glm::vec2* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform2fv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, const std::vector<glm::vec2>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform2fv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, glm::vec2* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform2fv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, const std::vector<glm::dvec2>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform2dv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, glm::dvec2* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform2dv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, const std::vector<glm::ivec2>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform2iv);
}
void Engine::Renderer::sendUniform2vSafe(const char* l, glm::ivec2* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform2iv);
}






void Engine::Renderer::sendUniform3(const char* l, const glm::dvec3& v) {
    sendUniformValues3(l, v.x, v.y, v.z, glUniform3d);
}
void Engine::Renderer::sendUniform3(const char* l, const glm::ivec3& v) {
    sendUniformValues3(l, v.x, v.y, v.z, glUniform3i);
}
void Engine::Renderer::sendUniform3(const char* l, const glm::vec3& v) {
    sendUniformValues3(l, v.x, v.y, v.z, glUniform3f);
}
void Engine::Renderer::sendUniform3(const char* l, const glm::uvec3& v) {
    sendUniformValues3(l, v.x, v.y, v.z, glUniform3ui);
}
void Engine::Renderer::sendUniform3Safe(const char* l, const glm::dvec3& v) {
    sendUniformValues3Safe(l, v.x, v.y, v.z, glUniform3d);
}
void Engine::Renderer::sendUniform3Safe(const char* l, const glm::ivec3& v) {
    sendUniformValues3Safe(l, v.x, v.y, v.z, glUniform3i);
}
void Engine::Renderer::sendUniform3Safe(const char* l, const glm::vec3& v) {
    sendUniformValues3Safe(l, v.x, v.y, v.z, glUniform3f);
}
void Engine::Renderer::sendUniform3Safe(const char* l, const glm::uvec3& v) {
    sendUniformValues3Safe(l, v.x, v.y, v.z, glUniform3ui);
}
void Engine::Renderer::sendUniform3(const char* l, double x, double y, double z) {
    sendUniformValues3(l, x, y, z, glUniform3d);
}
void Engine::Renderer::sendUniform3(const char* l, int x, int y, int z) {
    sendUniformValues3(l, x, y, z, glUniform3i);
}
void Engine::Renderer::sendUniform3(const char* l, float x, float y, float z) {
    sendUniformValues3(l, x, y, z, glUniform3f);
}
void Engine::Renderer::sendUniform3(const char* l, uint32_t x, uint32_t y, uint32_t z) {
    sendUniformValues3(l, x, y, z, glUniform3ui);
}
void Engine::Renderer::sendUniform3Safe(const char* l, double x, double y, double z) {
    sendUniformValues3Safe(l, x, y, z, glUniform3d);
}
void Engine::Renderer::sendUniform3Safe(const char* l, int x, int y, int z) {
    sendUniformValues3Safe(l, x, y, z, glUniform3i);
}
void Engine::Renderer::sendUniform3Safe(const char* l, float x, float y, float z) {
    sendUniformValues3Safe(l, x, y, z, glUniform3f);
}
void Engine::Renderer::sendUniform3Safe(const char* l, uint32_t x, uint32_t y, uint32_t z) {
    sendUniformValues3Safe(l, x, y, z, glUniform3ui);
}
void Engine::Renderer::sendUniform3v(const char* l, const std::vector<glm::dvec3>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform3dv);
}
void Engine::Renderer::sendUniform3v(const char* l, const std::vector<glm::ivec3>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform3iv);
}
void Engine::Renderer::sendUniform3v(const char* l, const std::vector<glm::vec3>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform3fv);
}
void Engine::Renderer::sendUniform3v(const char* l, glm::dvec3* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform3dv);
}
void Engine::Renderer::sendUniform3v(const char* l, glm::ivec3* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform3iv);
}
void Engine::Renderer::sendUniform3v(const char* l, glm::vec3* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform3fv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, const std::vector<glm::vec3>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform3fv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, glm::vec3* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform3fv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, const std::vector<glm::dvec3>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform3dv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, glm::dvec3* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform3dv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, const std::vector<glm::ivec3>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform3iv);
}
void Engine::Renderer::sendUniform3vSafe(const char* l, glm::ivec3* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform3iv);
}




void Engine::Renderer::sendUniform4(const char* l, const glm::dvec4& v) {
    sendUniformValues4(l, v.x, v.y, v.z, v.w, glUniform4d);
}
void Engine::Renderer::sendUniform4(const char* l, const glm::ivec4& v) {
    sendUniformValues4(l, v.x, v.y, v.z, v.w, glUniform4i);
}
void Engine::Renderer::sendUniform4(const char* l, const glm::vec4& v) {
    sendUniformValues4(l, v.x, v.y, v.z, v.w, glUniform4f);
}
void Engine::Renderer::sendUniform4(const char* l, const glm::uvec4& v) {
    sendUniformValues4(l, v.x, v.y, v.z, v.w, glUniform4ui);
}
void Engine::Renderer::sendUniform4Safe(const char* l, const glm::dvec4& v) {
    sendUniformValues4Safe(l, v.x, v.y, v.z, v.w, glUniform4d);
}
void Engine::Renderer::sendUniform4Safe(const char* l, const glm::ivec4& v) {
    sendUniformValues4Safe(l, v.x, v.y, v.z, v.w, glUniform4i);
}
void Engine::Renderer::sendUniform4Safe(const char* l, const glm::vec4& v) {
    sendUniformValues4Safe(l, v.x, v.y, v.z, v.w, glUniform4f);
}
void Engine::Renderer::sendUniform4Safe(const char* l, const glm::uvec4& v) {
    sendUniformValues4Safe(l, v.x, v.y, v.z, v.w, glUniform4ui);
}
void Engine::Renderer::sendUniform4(const char* l, double x, double y, double z, double w) {
    sendUniformValues4(l, x, y, z, w, glUniform4d);
}
void Engine::Renderer::sendUniform4(const char* l, int x, int y, int z, int w) {
    sendUniformValues4(l, x, y, z, w, glUniform4i);
}
void Engine::Renderer::sendUniform4(const char* l, float x, float y, float z, float w) {
    sendUniformValues4(l, x, y, z, w, glUniform4f);
}
void Engine::Renderer::sendUniform4(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w) {
    sendUniformValues4(l, x, y, z, w, glUniform4ui);
}
void Engine::Renderer::sendUniform4Safe(const char* l, double x, double y, double z, double w) {
    sendUniformValues4Safe(l, x, y, z, w, glUniform4d);
}
void Engine::Renderer::sendUniform4Safe(const char* l, int x, int y, int z, int w) {
    sendUniformValues4Safe(l, x, y, z, w, glUniform4i);
}
void Engine::Renderer::sendUniform4Safe(const char* l, float x, float y, float z, float w) {
    sendUniformValues4Safe(l, x, y, z, w, glUniform4f);
}
void Engine::Renderer::sendUniform4Safe(const char* l, uint32_t x, uint32_t y, uint32_t z, uint32_t w) {
    sendUniformValues4Safe(l, x, y, z, w, glUniform4ui);
}
void Engine::Renderer::sendUniform4v(const char* l, const std::vector<glm::dvec4>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform4dv);
}
void Engine::Renderer::sendUniform4v(const char* l, const std::vector<glm::ivec4>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform4iv);
}
void Engine::Renderer::sendUniform4v(const char* l, const std::vector<glm::vec4>& d, const uint32_t i) {
    sendUniformValues(l, d.data(), i, glUniform4fv);
}
void Engine::Renderer::sendUniform4v(const char* l, glm::dvec4* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform4dv);
}
void Engine::Renderer::sendUniform4v(const char* l, glm::ivec4* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform4iv);
}
void Engine::Renderer::sendUniform4v(const char* l, glm::vec4* d, const uint32_t i) {
    sendUniformValues(l, d, i, glUniform4fv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, const std::vector<glm::vec4>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform4fv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, glm::vec4* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform4fv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, const std::vector<glm::dvec4>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform4dv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, glm::dvec4* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform4dv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, const std::vector<glm::ivec4>& d, const uint32_t i) {
    sendUniformValuesSafe(l, d.data(), i, glUniform4iv);
}
void Engine::Renderer::sendUniform4vSafe(const char* l, glm::ivec4* d, const uint32_t i) {
    sendUniformValuesSafe(l, d, i, glUniform4iv);
}



void Engine::Renderer::sendUniformMatrix2(const char* l, const glm::mat2& m) {
    sendUniformMatrix(l, m, glUniformMatrix2fv);
}
void Engine::Renderer::sendUniformMatrix2v(const char* l, const std::vector<glm::mat2>& m, const uint32_t count) {
    sendUniformMatrixV<glm::mat2::value_type>(l, m, count, glUniformMatrix2fv);
}
void Engine::Renderer::sendUniformMatrix2(const char* l, const glm::dmat2& m) {
    sendUniformMatrix(l, m, glUniformMatrix2dv);
}
void Engine::Renderer::sendUniformMatrix2v(const char* l, const std::vector<glm::dmat2>& m, const uint32_t count) {
    sendUniformMatrixV<glm::dmat2::value_type>(l, m, count, glUniformMatrix2dv);
}
void Engine::Renderer::sendUniformMatrix2Safe(const char* l, const glm::mat2& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix2fv);
}
void Engine::Renderer::sendUniformMatrix2vSafe(const char* l, const std::vector<glm::mat2>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::mat2::value_type>(l, m, count, glUniformMatrix2fv);
}
void Engine::Renderer::sendUniformMatrix2Safe(const char* l, const glm::dmat2& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix2dv);
}
void Engine::Renderer::sendUniformMatrix2vSafe(const char* l, const std::vector<glm::dmat2>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::dmat2::value_type>(l, m, count, glUniformMatrix2dv);
}




void Engine::Renderer::sendUniformMatrix3(const char* l, const glm::mat3& m) {
    sendUniformMatrix(l, m, glUniformMatrix3fv);
}
void Engine::Renderer::sendUniformMatrix3v(const char* l, const std::vector<glm::mat3>& m, const uint32_t count) {
    sendUniformMatrixV<glm::mat3::value_type>(l, m, count, glUniformMatrix3fv);
}
void Engine::Renderer::sendUniformMatrix3(const char* l, const glm::dmat3& m) {
    sendUniformMatrix(l, m, glUniformMatrix3dv);
}
void Engine::Renderer::sendUniformMatrix3v(const char* l, const std::vector<glm::dmat3>& m, const uint32_t count) {
    sendUniformMatrixV<glm::dmat3::value_type>(l, m, count, glUniformMatrix3dv);
}
void Engine::Renderer::sendUniformMatrix3Safe(const char* l, const glm::mat3& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix3fv);
}
void Engine::Renderer::sendUniformMatrix3vSafe(const char* l, const std::vector<glm::mat3>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::mat3::value_type>(l, m, count, glUniformMatrix3fv);
}
void Engine::Renderer::sendUniformMatrix3Safe(const char* l, const glm::dmat3& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix3dv);
}
void Engine::Renderer::sendUniformMatrix3vSafe(const char* l, const std::vector<glm::dmat3>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::dmat3::value_type>(l, m, count, glUniformMatrix3dv);
}





void Engine::Renderer::sendUniformMatrix4(const char* l, const glm::mat4& m) {
    sendUniformMatrix(l, m, glUniformMatrix4fv);
}
void Engine::Renderer::sendUniformMatrix4v(const char* l, const std::vector<glm::mat4>& m, const uint32_t count) {
    sendUniformMatrixV<glm::mat4::value_type>(l, m, count, glUniformMatrix4fv);
}
void Engine::Renderer::sendUniformMatrix4(const char* l, const glm::dmat4& m) {
    sendUniformMatrix(l, m, glUniformMatrix4dv);
}
void Engine::Renderer::sendUniformMatrix4v(const char* l, const std::vector<glm::dmat4>& m, const uint32_t count) {
    sendUniformMatrixV<glm::dmat4::value_type>(l, m, count, glUniformMatrix4dv);
}
void Engine::Renderer::sendUniformMatrix4Safe(const char* l, const glm::mat4& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix4fv);
}
void Engine::Renderer::sendUniformMatrix4vSafe(const char* l, const std::vector<glm::mat4>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::mat4::value_type>(l, m, count, glUniformMatrix4fv);
}
void Engine::Renderer::sendUniformMatrix4Safe(const char* l, const glm::dmat4& m) {
    sendUniformMatrixSafe(l, m, glUniformMatrix4dv);
}
void Engine::Renderer::sendUniformMatrix4vSafe(const char* l, const std::vector<glm::dmat4>& m, const uint32_t count) {
    sendUniformMatrixVSafe<glm::dmat4::value_type>(l, m, count, glUniformMatrix4dv);
}

#pragma endregion