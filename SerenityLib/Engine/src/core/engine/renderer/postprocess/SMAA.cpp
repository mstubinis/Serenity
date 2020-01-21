#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/SMAA_LUT.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/shaders/ShaderProgram.h>


Engine::priv::SMAA Engine::priv::SMAA::smaa;

Engine::priv::SMAA::SMAA() {
    THRESHOLD                        = 0.05f;
    MAX_SEARCH_STEPS                 = 32;
    MAX_SEARCH_STEPS_DIAG            = 16;
    CORNER_ROUNDING                  = 25;
    LOCAL_CONTRAST_ADAPTATION_FACTOR = 2.0f;
    DEPTH_THRESHOLD                  = (0.1f * THRESHOLD);
    PREDICATION                      = false;
    PREDICATION_THRESHOLD            = 0.01f;
    PREDICATION_SCALE                = 2.0f;
    PREDICATION_STRENGTH             = 0.4f;
    REPROJECTION                     = false;
    REPROJECTION_WEIGHT_SCALE        = 30.0f;
    AREATEX_MAX_DISTANCE             = 16;
    AREATEX_MAX_DISTANCE_DIAG        = 20;
    AREATEX_PIXEL_SIZE               = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
    AREATEX_SUBTEX_SIZE              = 0.14285714285f; //(1 / 7)
    AreaTexture                      = 0;
    SearchTexture                    = 0;
}
Engine::priv::SMAA::~SMAA() {
    glDeleteTextures(1, &SearchTexture);
    glDeleteTextures(1, &AreaTexture);
}
void Engine::priv::SMAA::init() {
    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, AreaTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, SMAA_areaTexBytes);

    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, SearchTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 64, 16, 0, GL_RED, GL_UNSIGNED_BYTE, SMAA_searchTexBytes);
}
void Engine::priv::SMAA::passEdge(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture, const unsigned int& outTexture) {
    gbuffer.bindFramebuffers(outTexture); //probably the lighting buffer
    program.bind();

    Engine::Renderer::Settings::clear(true, false, true);//lighting rgba, stencil is completely filled with 0's

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR, GL_INCR);
    Engine::Renderer::GLEnable(GL_STENCIL_TEST);

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendUniform4Safe("SMAAInfo1Floats", THRESHOLD, DEPTH_THRESHOLD, LOCAL_CONTRAST_ADAPTATION_FACTOR, PREDICATION_THRESHOLD);
    Engine::Renderer::sendUniform2Safe("SMAAInfo1FloatsA", PREDICATION_SCALE, PREDICATION_STRENGTH);

    Engine::Renderer::sendUniform1Safe("SMAA_PREDICATION", static_cast<int>(PREDICATION));

    Engine::Renderer::sendTexture("textureMap", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("texturePredication", gbuffer.getTexture(GBufferType::Diffuse), 1);

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_EQUAL, 0x00000001, 0x00000001);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil
}
void Engine::priv::SMAA::passBlend(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& outTexture) {
    gbuffer.bindFramebuffers(GBufferType::Normal);
    Engine::Renderer::Settings::clear(true, false, false); //clear color only

    program.bind();
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendTexture("edge_tex", gbuffer.getTexture(outTexture), 0);
    Engine::Renderer::sendTexture("area_tex", AreaTexture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("search_tex", SearchTexture, 2, GL_TEXTURE_2D);

    Engine::Renderer::sendUniform1Safe("SMAA_MAX_SEARCH_STEPS", MAX_SEARCH_STEPS);

    Engine::Renderer::sendUniform4Safe("SMAAInfo2Ints", MAX_SEARCH_STEPS_DIAG, AREATEX_MAX_DISTANCE, AREATEX_MAX_DISTANCE_DIAG, CORNER_ROUNDING);
    Engine::Renderer::sendUniform4Safe("SMAAInfo2Floats", AREATEX_PIXEL_SIZE.x, AREATEX_PIXEL_SIZE.y, AREATEX_SUBTEX_SIZE, (static_cast<float>(CORNER_ROUNDING) / 100.0f));

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);

    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
}
void Engine::priv::SMAA::passNeighbor(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    program.bind();
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);
    Engine::Renderer::sendTextureSafe("textureMap", gbuffer.getTexture(sceneTexture), 0); //need original final image from first smaa pass
    Engine::Renderer::sendTextureSafe("blend_tex", gbuffer.getTexture(GBufferType::Normal), 1);

    Engine::Renderer::renderFullscreenTriangle(fboWidth, fboHeight);
}
void Engine::priv::SMAA::passFinal(ShaderProgram& program, Engine::priv::GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight) {
    /*
    //this pass is optional. lets skip it for now
    //gbuffer.bindFramebuffers(GBufferType::Lighting);
    gbuffer.stop();
    program.bind();
    Engine::Renderer::renderFullscreenTriangle(fboWidth,fboHeight);
    */
}
void Engine::Renderer::smaa::setThreshold(const float f) {
    Engine::priv::SMAA::smaa.THRESHOLD = f;
}
void Engine::Renderer::smaa::setSearchSteps(const unsigned int s) {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = s;
}
void Engine::Renderer::smaa::disableCornerDetection() {
    Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
}
void Engine::Renderer::smaa::enableCornerDetection(const unsigned int c) {
    Engine::priv::SMAA::smaa.CORNER_ROUNDING = c;
}
void Engine::Renderer::smaa::disableDiagonalDetection() {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
}
void Engine::Renderer::smaa::enableDiagonalDetection(const unsigned int d) {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = d;
}
void Engine::Renderer::smaa::setPredicationThreshold(const float f) {
    Engine::priv::SMAA::smaa.PREDICATION_THRESHOLD = f;
}
void Engine::Renderer::smaa::setPredicationScale(const float f) {
    Engine::priv::SMAA::smaa.PREDICATION_SCALE = f;
}
void Engine::Renderer::smaa::setPredicationStrength(const float s) {
    Engine::priv::SMAA::smaa.PREDICATION_STRENGTH = s;
}
void Engine::Renderer::smaa::setReprojectionScale(const float s) {
    Engine::priv::SMAA::smaa.REPROJECTION_WEIGHT_SCALE = s;
}
void Engine::Renderer::smaa::enablePredication(const bool b) {
    Engine::priv::SMAA::smaa.PREDICATION = b;
}
void Engine::Renderer::smaa::disablePredication() {
    Engine::priv::SMAA::smaa.PREDICATION = false;
}
void Engine::Renderer::smaa::enableReprojection(const bool b) {
    Engine::priv::SMAA::smaa.REPROJECTION = b;
}
void Engine::Renderer::smaa::disableReprojection() {
    Engine::priv::SMAA::smaa.REPROJECTION = false;
}
void Engine::Renderer::smaa::setQuality(const SMAAQualityLevel::Level level) {
    switch (level) {
        case SMAAQualityLevel::Low: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.15f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 4;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::Medium: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.1f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 8;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::High: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.1f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 16;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 8;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 25;
            break;
        }case SMAAQualityLevel::Ultra: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.05f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 32;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 16;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 25;
            break;
        }default: {
            break;
        }
    }
}