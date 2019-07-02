#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/SMAA_LUT.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/textures/Texture.h>
#include <core/ShaderProgram.h>

using namespace Engine;

epriv::Postprocess_SMAA epriv::Postprocess_SMAA::SMAA;


epriv::Postprocess_SMAA::Postprocess_SMAA() {
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
epriv::Postprocess_SMAA::~Postprocess_SMAA() {
    glDeleteTextures(1, &SearchTexture);
    glDeleteTextures(1, &AreaTexture);
}


void epriv::Postprocess_SMAA::init() {
    Renderer::genAndBindTexture(GL_TEXTURE_2D, AreaTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, SMAA_areaTexBytes);

    Renderer::genAndBindTexture(GL_TEXTURE_2D, SearchTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 64, 16, 0, GL_RED, GL_UNSIGNED_BYTE, SMAA_searchTexBytes);
}



void epriv::Postprocess_SMAA::passEdge(ShaderP& program, GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture, const unsigned int& outTexture) {
    gbuffer.start(outTexture);
    program.bind();

    Renderer::Settings::clear(true, false, true);//color, stencil is completely filled with 0's

    glStencilMask(0xFFFFFFFF);
    glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    Renderer::GLEnable(GLState::STENCIL_TEST);

    Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Renderer::sendUniform4Safe("SMAAInfo1Floats", THRESHOLD, DEPTH_THRESHOLD, LOCAL_CONTRAST_ADAPTATION_FACTOR, PREDICATION_THRESHOLD);
    Renderer::sendUniform2Safe("SMAAInfo1FloatsA", PREDICATION_SCALE, PREDICATION_STRENGTH);

    Renderer::sendUniform1Safe("SMAA_PREDICATION", static_cast<int>(PREDICATION));

    Renderer::sendTexture("textureMap", gbuffer.getTexture(sceneTexture), 0);
    Renderer::sendTextureSafe("texturePredication", gbuffer.getTexture(GBufferType::Diffuse), 1);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);

    glStencilMask(0xFFFFFFFF);
    glStencilFunc(GL_EQUAL, 0x00000001, 0x00000001);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil
}
void epriv::Postprocess_SMAA::passBlend(ShaderP& program, GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& outTexture) {
    gbuffer.start(GBufferType::Normal);
    Renderer::Settings::clear(true, false, false); //clear color only

    program.bind();
    Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Renderer::sendTexture("edge_tex", gbuffer.getTexture(outTexture), 0);
    Renderer::sendTexture("area_tex", AreaTexture, 1, GL_TEXTURE_2D);
    Renderer::sendTexture("search_tex", SearchTexture, 2, GL_TEXTURE_2D);

    Renderer::sendUniform1Safe("SMAA_MAX_SEARCH_STEPS", MAX_SEARCH_STEPS);

    Renderer::sendUniform4Safe("SMAAInfo2Ints", MAX_SEARCH_STEPS_DIAG, AREATEX_MAX_DISTANCE, AREATEX_MAX_DISTANCE_DIAG, CORNER_ROUNDING);
    Renderer::sendUniform4Safe("SMAAInfo2Floats", AREATEX_PIXEL_SIZE.x, AREATEX_PIXEL_SIZE.y, AREATEX_SUBTEX_SIZE, (static_cast<float>(CORNER_ROUNDING) / 100.0f));

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);

    Renderer::GLDisable(GLState::STENCIL_TEST);
}
void epriv::Postprocess_SMAA::passNeighbor(ShaderP& program, GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const unsigned int& fboWidth, const unsigned int& fboHeight, const unsigned int& sceneTexture) {
    //gbuffer.start(GBufferType::Misc);
    gbuffer.stop();
    program.bind();
    Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);
    Renderer::sendTextureSafe("textureMap", gbuffer.getTexture(sceneTexture), 0); //need original final image from first smaa pass
    Renderer::sendTextureSafe("blend_tex", gbuffer.getTexture(GBufferType::Normal), 1);

    Renderer::renderFullscreenTriangle(fboWidth, fboHeight, 0, 0);
}
void epriv::Postprocess_SMAA::passFinal(ShaderP& program, GBuffer& gbuffer, const unsigned int& fboWidth, const unsigned int& fboHeight) {
    /*
    //this pass is optional. lets skip it for now
    //gbuffer.start(GBufferType::Lighting);
    gbuffer.stop();
    program.bind();
    Renderer::renderFullscreenTriangle(fboWidth,fboHeight,0,0);
    */
}




void Renderer::smaa::setThreshold(const float f) {
    epriv::Postprocess_SMAA::SMAA.THRESHOLD = f;
}
void Renderer::smaa::setSearchSteps(const unsigned int s) {
    epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS = s;
}
void Renderer::smaa::disableCornerDetection() {
    epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING = 0;
}
void Renderer::smaa::enableCornerDetection(const unsigned int c) {
    epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING = c;
}
void Renderer::smaa::disableDiagonalDetection() {
    epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = 0;
}
void Renderer::smaa::enableDiagonalDetection(const unsigned int d) {
    epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = d;
}
void Renderer::smaa::setPredicationThreshold(const float f) {
    epriv::Postprocess_SMAA::SMAA.PREDICATION_THRESHOLD = f;
}
void Renderer::smaa::setPredicationScale(const float f) {
    epriv::Postprocess_SMAA::SMAA.PREDICATION_SCALE = f;
}
void Renderer::smaa::setPredicationStrength(const float s) {
    epriv::Postprocess_SMAA::SMAA.PREDICATION_STRENGTH = s;
}
void Renderer::smaa::setReprojectionScale(const float s) {
    epriv::Postprocess_SMAA::SMAA.REPROJECTION_WEIGHT_SCALE = s;
}
void Renderer::smaa::enablePredication(const bool b) {
    epriv::Postprocess_SMAA::SMAA.PREDICATION = b;
}
void Renderer::smaa::disablePredication() {
    epriv::Postprocess_SMAA::SMAA.PREDICATION = false;
}
void Renderer::smaa::enableReprojection(const bool b) {
    epriv::Postprocess_SMAA::SMAA.REPROJECTION = b;
}
void Renderer::smaa::disableReprojection() {
    epriv::Postprocess_SMAA::SMAA.REPROJECTION = false;
}
void Renderer::smaa::setQuality(const SMAAQualityLevel::Level level) {
    if (level == SMAAQualityLevel::Low) {
        epriv::Postprocess_SMAA::SMAA.THRESHOLD             = 0.15f;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS      = 4;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = 0;
        epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING       = 0;
    }else if (level == SMAAQualityLevel::Medium) {
        epriv::Postprocess_SMAA::SMAA.THRESHOLD             = 0.1f;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS      = 8;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = 0;
        epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING       = 0;
    }else if (level == SMAAQualityLevel::High) {
        epriv::Postprocess_SMAA::SMAA.THRESHOLD             = 0.1f;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS      = 16;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = 8;
        epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING       = 25;
    }else if (level == SMAAQualityLevel::Ultra) {
        epriv::Postprocess_SMAA::SMAA.THRESHOLD             = 0.05f;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS      = 32;
        epriv::Postprocess_SMAA::SMAA.MAX_SEARCH_STEPS_DIAG = 16;
        epriv::Postprocess_SMAA::SMAA.CORNER_ROUNDING       = 25;
    }
}