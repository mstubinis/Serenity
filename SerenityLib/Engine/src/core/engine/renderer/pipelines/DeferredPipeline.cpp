#include <core/engine/renderer/pipelines/DeferredPipeline.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/opengl/UniformBufferObject.h>
#include <core/engine/system/Engine.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <core/engine/scene/Camera.h>

#include <ecs/ComponentBody.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/DepthOfField.h>
#include <core/engine/renderer/postprocess/Bloom.h>
#include <core/engine/renderer/postprocess/FXAA.h>
#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

using namespace std;
using namespace Engine;
using namespace Engine::priv;
using namespace Engine::Renderer;


struct ShaderProgramEnum final {
    enum Program : unsigned int {
        BulletPhysics,
        ZPrepass,
        Deferred2DAPI,
        DeferredBlur,
        DeferredFinal,
        DepthAndTransparency,
        DeferredSkybox,
        CopyDepth,
        DeferredLighting,
        DeferredLightingGI,
        CubemapConvolude,
        CubemapPrefilterEnv,
        BRDFPrecomputeCookTorrance,
        //Grayscale,
        StencilPass,
        Particle,
        _TOTAL,
    };
};


void GLScissorDisable() {
    const auto winSize = Resources::getWindowSize();
    glScissor(0, 0, winSize.x, winSize.y);
}
void GLScissor(const glm::vec4& s) {
    if (s == glm::vec4(-1.0f)) {
        GLScissorDisable();
    }else{
        glScissor(static_cast<GLint>(s.x), static_cast<GLint>(s.y), static_cast<GLsizei>(s.z), static_cast<GLsizei>(s.w));
    }
}




DeferredPipeline::DeferredPipeline() {
    const auto window_size = Resources::getWindowSize();
    m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(window_size.x), 0.0f, static_cast<float>(window_size.y), 0.005f, 3000.0f);
    m_GBuffer   = nullptr;
    m_UBOCamera = nullptr;
}
DeferredPipeline::~DeferredPipeline() {
    SAFE_DELETE(m_GBuffer);
}

void DeferredPipeline::init() {
    m_UBOCamera = NEW UniformBufferObject("Camera", sizeof(UBOCameraDataStruct));
    m_UBOCamera->updateData(&m_UBOCameraDataStruct);
}
void DeferredPipeline::internal_generate_pbr_data_for_texture(ShaderProgram& covoludeShaderProgram, ShaderProgram& prefilterShaderProgram, Texture& texture, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize) {
    const auto texType = texture.type();
    if (texType != GL_TEXTURE_CUBE_MAP) {
        //cout << "(Texture) : Only cubemaps can be precomputed for IBL. Ignoring genPBREnvMapData() call..." << endl;
        return;
    }
    unsigned int size = convoludeTextureSize;
    Engine::Renderer::bindTextureForModification(texType, texture.address(1));
    Engine::Renderer::unbindFBO();
    priv::FramebufferObject fbo(texture.name() + "_fbo_envData", size, size); //try without a depth format
    fbo.bind();

    //make these 2 variables global in the renderer class?
    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f);
    const glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f),glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
        glm::lookAt(glm::vec3(0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
    };
    //m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapConvolude]->bind();
    covoludeShaderProgram.bind();

    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType);
    Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(size), static_cast<float>(size));
    for (unsigned int i = 0; i < 6; ++i) {
        const glm::mat4 vp = captureProjection * captureViews[i];
        Engine::Renderer::sendUniformMatrix4("VP", vp);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture.address(1), 0);
        Engine::Renderer::Settings::clear(true, true, false);
        Skybox::bindMesh();
    }
    //TODO: re-check if this is needed
    //Resources::getWindow().display(); //prevent opengl & windows timeout


    //now gen EnvPrefilterMap for specular IBL
    size = preEnvFilterSize;
    Engine::Renderer::bindTextureForModification(texType, texture.address(2));
    //m_InternalShaderPrograms[EngineInternalShaderPrograms::CubemapPrefilterEnv]->bind();
    prefilterShaderProgram.bind();
    Engine::Renderer::sendTexture("cubemap", texture.address(), 0, texType);
    Engine::Renderer::sendUniform1("PiFourDividedByResSquaredTimesSix", 12.56637f / float((texture.width() * texture.width()) * 6));
    Engine::Renderer::sendUniform1("NUM_SAMPLES", 32);
    const unsigned int maxMipLevels = 5;
    for (unsigned int m = 0; m < maxMipLevels; ++m) {
        const unsigned int mipSize(size * glm::pow(0.5, m)); // reisze framebuffer according to mip-level size.
        fbo.resize(mipSize, mipSize);
        const float roughness = static_cast<float>(m) / static_cast<float>(maxMipLevels - 1);
        Engine::Renderer::sendUniform1("roughness", roughness);
        const float a = roughness * roughness;
        Engine::Renderer::sendUniform1("a2", a * a);
        for (unsigned int i = 0; i < 6; ++i) {
            const glm::mat4 vp = captureProjection * captureViews[i];
            Engine::Renderer::sendUniformMatrix4("VP", vp);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture.address(2), m);
            Engine::Renderer::Settings::clear(true, true, false);
            Skybox::bindMesh();
        }
    }
    //TODO: re-check if this is needed
    //Resources::getWindow().display(); //prevent opengl & windows timeout
    fbo.unbind();
}
void DeferredPipeline::internal_generate_brdf_lut(ShaderProgram& program, const unsigned int& brdfSize) {
    FramebufferObject fbo("BRDFLUT_Gen_CookTorr_FBO", brdfSize, brdfSize); //try without a depth format
    fbo.bind();

    Engine::Renderer::bindTextureForModification(GL_TEXTURE_2D, Texture::BRDF->address());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToEdge);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture::BRDF->address(), 0);

    //m_InternalShaderPrograms[ShaderProgramEnum::BRDFPrecomputeCookTorrance]->bind();
    program.bind();
    Engine::Renderer::sendUniform1("NUM_SAMPLES", 256);
    Engine::Renderer::Settings::clear(true, true, false);
    Engine::Renderer::colorMask(true, true, false, false);
    Engine::Renderer::renderFullscreenTriangle(brdfSize, brdfSize);
    Engine::Renderer::colorMask(true, true, true, true);
}
void DeferredPipeline::onPipelineChanged() {

}

void DeferredPipeline::onFullscreen() {
    SAFE_DELETE(m_GBuffer);

    //TODO: move these lines to a more generic area, all rendering pipelines will pretty much do this
    restoreCurrentOpenGLState();
    //sfWindow->create(videoMode, winName, style, settings);



    GLEnable(GL_CULL_FACE);
    GLEnable(GL_DEPTH_CLAMP);

    const auto winSize = Resources::getWindowSize();
    m_GBuffer = NEW GBuffer(winSize.x, winSize.y);
}
//DONE
void DeferredPipeline::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    m_2DProjectionMatrix = glm::ortho(0.0f, static_cast<float>(newWidth), 0.0f, static_cast<float>(newHeight), 0.005f, 3000.0f);
    m_GBuffer->resize(newWidth, newHeight);
}
void DeferredPipeline::onOpenGLContextCreation() {
    const auto winSize = Resources::getWindowSize();

    //TODO: move to a more generic area
    //epriv::RenderManager::GLSL_VERSION = _glslVersion;
    //epriv::RenderManager::OPENGL_VERSION = _openglVersion;

    GLEnable(GL_CULL_FACE);
    SAFE_DELETE(m_GBuffer);
    m_GBuffer = NEW GBuffer(winSize.x, winSize.y);
}
//DONE
void DeferredPipeline::renderSkybox(Skybox* skybox, ShaderProgram& shaderProgram, const Scene& scene, const Viewport& viewport, const Camera& camera) {
    glm::mat4 view_no_position = camera.getView();
    Math::removeMatrixPosition(view_no_position);

    shaderProgram.bind();
    if (skybox) {
        Engine::Renderer::sendUniform1("IsFake", 0);
        Engine::Renderer::sendTextureSafe("Texture", skybox->texture()->address(0), 0, GL_TEXTURE_CUBE_MAP);
    }else{
        Engine::Renderer::sendUniform1("IsFake", 1);
        const auto& bgColor = scene.getBackgroundColor();
        Engine::Renderer::sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    }
    Engine::Renderer::sendUniformMatrix4("VP", camera.getProjection() * view_no_position);
    Skybox::bindMesh();

    Engine::Renderer::sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
    //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...
}
//DONE
void DeferredPipeline::renderDirectionalLight(const Camera& c, const DirectionalLight& d) {
    if (!d.isActive())
        return;
    const auto& body    = *d.getComponent<ComponentBody>();
    const auto forward  = glm::vec3(body.forward());
    const auto& col     = d.color();
    sendUniform4("light.DataA", d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), forward.x);
    sendUniform4("light.DataB", forward.y, forward.z, 0.0f, 0.0f);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(d.type()));
    sendUniform1Safe("Type", 0.0f);
    renderFullscreenTriangle();
}
//DONE
void DeferredPipeline::renderSunLight(const Camera& c, const SunLight& s) {
    if (!s.isActive())
        return;
    const auto& body = *s.getComponent<ComponentBody>();
    const auto pos   = glm::vec3(body.position());
    const auto& col  = s.color();
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
    sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    sendUniform1Safe("Type", 0.0f);
    renderFullscreenTriangle();
}
//DONE
void DeferredPipeline::renderPointLight(const Camera& c, const PointLight& p) {
    if (!p.isActive())
        return;
    const auto& body  = *p.getComponent<ComponentBody>();
    const auto pos    = glm::vec3(body.position());
    const auto cull   = p.getCullingRadius();
    const auto factor = 1100.0f * cull;
    const auto distSq = static_cast<float>(c.getDistanceSquared(pos));
    if ((!c.sphereIntersectTest(pos, cull)) || (distSq > factor * factor))
        return;

    const auto& col = p.color();
    sendUniform4("light.DataA", p.getAmbientIntensity(), p.getDiffuseIntensity(), p.getSpecularIntensity(), 0.0f);
    sendUniform4("light.DataB", 0.0f, 0.0f, p.getConstant(), p.getLinear());
    sendUniform4("light.DataC", p.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(p.type()));
    sendUniform4Safe("light.DataE", 0.0f, 0.0f, static_cast<float>(p.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& pointLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPointLightBounds();

    pointLightMesh.bind();
    pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    pointLightMesh.unbind();
    cullFace(GL_BACK);
}
//DONE
void DeferredPipeline::renderSpotLight(const Camera& c, const SpotLight& s) {
    return;
    const auto& body    = *s.entity().getComponent<ComponentBody>();
    const auto pos      = glm::vec3(body.position());
    const auto forward  = glm::vec3(body.forward());
    const auto cull     = s.getCullingRadius();
    const auto factor   = 1100.0f * cull;
    const auto distSq   = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cull) || (distSq > factor * factor))
        return;
    const auto& col = s.color();
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), forward.x);
    sendUniform4("light.DataB", forward.y, forward.z, s.getConstant(), s.getLinear());
    sendUniform4("light.DataC", s.getExponent(), pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    sendUniform4Safe("light.DataE", s.getCutoff(), s.getCutoffOuter(), static_cast<float>(s.getAttenuationModel()), 0.0f);
    sendUniform2Safe("VertexShaderData", s.getCutoffOuter(), cull);
    sendUniform1Safe("Type", 2.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cull * cull)) { //inside the light volume                                                 
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& spotLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getSpotLightBounds();

    spotLightMesh.bind();
    spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    spotLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
}
//DONE
void DeferredPipeline::renderRodLight(const Camera& c, const RodLight& r) {
    if (!r.isActive())
        return;
    const auto& body           = *r.entity().getComponent<ComponentBody>();
    const auto pos             = glm::vec3(body.position());
    const auto cullingDistance = r.rodLength() + (r.getCullingRadius() * 2.0f);
    const auto factor          = 1100.0f * cullingDistance;
    const auto distSq          = static_cast<float>(c.getDistanceSquared(pos));
    if (!c.sphereIntersectTest(pos, cullingDistance) || (distSq > factor * factor))
        return;
    const auto& col       = r.color();
    const float half      = r.rodLength() / 2.0f;
    const auto firstEndPt = pos + (glm::vec3(body.forward()) * half);
    const auto secndEndPt = pos - (glm::vec3(body.forward()) * half);
    sendUniform4("light.DataA", r.getAmbientIntensity(), r.getDiffuseIntensity(), r.getSpecularIntensity(), firstEndPt.x);
    sendUniform4("light.DataB", firstEndPt.y, firstEndPt.z, r.getConstant(), r.getLinear());
    sendUniform4("light.DataC", r.getExponent(), secndEndPt.x, secndEndPt.y, secndEndPt.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(r.type()));
    sendUniform4Safe("light.DataE", r.rodLength(), 0.0f, static_cast<float>(r.getAttenuationModel()), 0.0f);
    sendUniform1Safe("Type", 1.0f);

    const auto model = body.modelMatrix();

    sendUniformMatrix4("Model", model);
    sendUniformMatrix4("VP", m_UBOCameraDataStruct.ViewProj);

    if (distSq <= (cullingDistance * cullingDistance)) {
        cullFace(GL_FRONT);
    }else{
        cullFace(GL_BACK);
    }
    auto& rodLightMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getRodLightBounds();

    rodLightMesh.bind();
    rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    rodLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
}

void DeferredPipeline::renderParticle(const Particle& particle) {
    particle.getMaterial()->bind();

    auto maxTextures = priv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;

    Camera& camera = *particle.scene().getActiveCamera();
    Engine::Renderer::sendTextureSafe("gDepthMap", m_GBuffer->getTexture(Engine::priv::GBufferType::Depth), maxTextures);
    Engine::Renderer::sendUniform4Safe("Object_Color", particle.color());
    Engine::Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, particle.position());
    modelMatrix *= glm::mat4_cast(camera.getOrientation());
    modelMatrix = glm::rotate(modelMatrix, particle.angle(), glm::vec3(0, 0, 1));
    const auto& scale = particle.getScale();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));

    Engine::Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

    priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().render();
}
void DeferredPipeline::renderMesh(const Mesh& mesh, const unsigned int mode) {
    const auto indicesSize = mesh.getVertexData().indices.size();
    if (indicesSize == 0)
        return;
    //if (instancing && priv::InternalMeshPublicInterface::SupportsInstancing()) {
        //const uint& instancesCount = m_InstanceCount;
        //if (instancesCount == 0) 
        //    return;
        //if (Renderer::OPENGL_VERSION >= 31) {
        //    glDrawElementsInstanced(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (OpenGLExtension::supported(OpenGLExtension::EXT_draw_instanced)) {
        //    glDrawElementsInstancedEXT(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //} else if (OpenGLExtension::supported(OpenGLExtension::ARB_draw_instanced)) {
        //    glDrawElementsInstancedARB(mode, indicesSize, GL_UNSIGNED_SHORT, 0, instancesCount);
        //}
    //}
    //else {
        glDrawElements(mode, indicesSize, GL_UNSIGNED_SHORT, nullptr);
    //}
}
//DONE
void DeferredPipeline::internal_render_2d_text_left(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    unsigned int i = 0;
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (auto& character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight;
            x = 0.0f;
        }else if (character != '\0') {
            accumulator = i * 4;
            ++i;
            const FontGlyph& chr = font.getGlyphData(character);
            startingY = -int(chr.height + chr.yoffset) - y;

            m_Text_Indices.emplace_back(accumulator + 0);
            m_Text_Indices.emplace_back(accumulator + 1);
            m_Text_Indices.emplace_back(accumulator + 2);
            m_Text_Indices.emplace_back(accumulator + 3);
            m_Text_Indices.emplace_back(accumulator + 1);
            m_Text_Indices.emplace_back(accumulator + 0);

            startingX = x + chr.xoffset;
            x        += chr.xadvance;

            for (unsigned char i = 0; i < 4; ++i)
                m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

            for (unsigned char i = 0; i < 4; ++i)
                m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
        }
    }
}
//DONE
void DeferredPipeline::internal_render_2d_text_center(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string>          lines;
    vector<unsigned short>  lines_sizes;
    string                  line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            lines_sizes.push_back(static_cast<unsigned short>(x));
            line_accumulator = "";
            x = 0.0f;
            continue;
        }else if (character != '\0') {
            const FontGlyph& chr = font.getGlyphData(character);
            line_accumulator += character;
            x += chr.xadvance;
        }
    }
    if (!line_accumulator.empty()) {
        lines.push_back(line_accumulator);
        lines_sizes.push_back(static_cast<unsigned short>(x));
    }

    x = 0.0f;
    unsigned int i = 0;
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (uint l = 0; l < lines.size(); ++l) {
        const auto& line = lines[l];
        const auto& line_size = lines_sizes[l] / 2;
        for (auto& character : line) {
            if (character != '\0') {
                accumulator = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accumulator + 0);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 2);
                m_Text_Indices.emplace_back(accumulator + 3);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 0);

                startingX = x + chr.xoffset;
                x += chr.xadvance;

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x - line_size, startingY + chr.pts[i].y, z);

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}
//DONE
void DeferredPipeline::internal_render_2d_text_right(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string> lines;
    string line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            line_accumulator = "";
            continue;
        }
        else if (character != '\0') {
            line_accumulator += character;
        }
    }
    if (lines.size() == 0)
        lines.push_back(line_accumulator);

    unsigned int i = 0;
    float startingX;
    float startingY;
    unsigned int accumulator;
    for (auto& line : lines) {
        const int line_size = static_cast<int>(line.size());
        int k = 0;
        for (int j = line_size; j >= 0; --j) {
            const auto& character = line[j];
            if (character != '\0') {
                accumulator = i * 4;
                ++i;
                const FontGlyph& chr = font.getGlyphData(character);
                startingY = -int(chr.height + chr.yoffset) - y;

                m_Text_Indices.emplace_back(accumulator + 0);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 2);
                m_Text_Indices.emplace_back(accumulator + 3);
                m_Text_Indices.emplace_back(accumulator + 1);
                m_Text_Indices.emplace_back(accumulator + 0);

                if (k == 0) {
                    x -= chr.width;
                }
                startingX = x + chr.xoffset;
                x -= chr.xadvance;

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + chr.pts[i].x, startingY + chr.pts[i].y, z);

                for (unsigned char i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(chr.uvs[i].x, chr.uvs[i].y);
                ++k;
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    }
}

//DONE
void DeferredPipeline::render2DText(const string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    auto& fontPlane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();
    fontPlane.bind();

    const auto  newLineGlyphHeight = font.getGlyphData('X').height;
    const auto& texture = font.getGlyphTexture();
    float y = 0.0f;
    float x = 0.0f;
    float z = -0.001f - depth;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1));

    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    Engine::Renderer::sendTexture("DiffuseTexture", texture, 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    if (textAlignment == TextAlignment::Left) {
        internal_render_2d_text_left(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Right) {
        internal_render_2d_text_right(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Center) {
        internal_render_2d_text_center(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }
    fontPlane.modifyVertices(0, m_Text_Points, MeshModifyFlags::Default); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(1, m_Text_UVs);
    fontPlane.modifyIndices(m_Text_Indices);
    fontPlane.render();


}
//DONE
void DeferredPipeline::render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& plane = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    plane.bind();

    float translationX = position.x;
    float translationY = position.y;
    float totalSizeX   = scale.x;
    float totalSizeY   = scale.y;
    if (texture) {
        totalSizeX *= texture->width();
        totalSizeY *= texture->height();
        Engine::Renderer::sendTexture("DiffuseTexture", *texture, 0);
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 1);
    }else{
        Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
        Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
    }
    Engine::Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(totalSizeX, totalSizeY, 1.0f));

    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    plane.render();
}
//DONE
void DeferredPipeline::render2DTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& alignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& triangle = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh();
    triangle.bind();

    float translationX = position.x;
    float translationY = position.y;

    Engine::Renderer::alignmentOffset(alignment, translationX, translationY, width, height);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(width, height, 1));

    Engine::Renderer::sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
    Engine::Renderer::sendUniform1("DiffuseTextureEnabled", 0);
    Engine::Renderer::sendUniform4("Object_Color", color);
    Engine::Renderer::sendUniformMatrix4("Model", modelMatrix);

    triangle.render();
}


void DeferredPipeline::internal_render_per_frame_preparation() {

}
void DeferredPipeline::internal_pass_geometry(const Viewport& viewport, const Camera& camera) {
    const Scene& scene     = viewport.getScene();
    const glm::vec4& clear = viewport.getBackgroundColor();
    const float colors[4]  = { clear.r, clear.g, clear.b, clear.a };

    m_GBuffer->bindFramebuffers(GBufferType::Diffuse, GBufferType::Normal, GBufferType::Misc, "RGBA");

    Engine::Renderer::Settings::clear(true, true, true); // (0,0,0,0)

    Engine::Renderer::setDepthFunc(GL_LEQUAL);

    glClearBufferfv(GL_COLOR, 0, colors);
    auto& godRays = GodRays::godRays;
    if (godRays.godRays_active) {
        const float godraysclearcolor[4] = { godRays.clearColor.r, godRays.clearColor.g, godRays.clearColor.b, godRays.clearColor.a };
        glClearBufferfv(GL_COLOR, 2, godraysclearcolor);
    }
    Engine::Renderer::GLEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    Engine::Renderer::GLEnablei(GL_BLEND, 0); //this is needed for sure
    InternalScenePublicInterface::RenderGeometryOpaque(scene, viewport, camera);
    if ((viewport.getRenderFlags() & ViewportRenderingFlag::Skybox)) {
        renderSkybox(scene.skybox(), *m_InternalShaderPrograms[ShaderProgramEnum::DeferredSkybox], scene, viewport, camera);
    }
    InternalScenePublicInterface::RenderGeometryTransparent(scene, viewport, camera);
    InternalScenePublicInterface::RenderGeometryTransparentTrianglesSorted(scene, viewport, camera, true);
}
void DeferredPipeline::internal_pass_ssao() {

}
void DeferredPipeline::internal_pass_stencil() {

}
void DeferredPipeline::internal_pass_lighting() {

}
void DeferredPipeline::internal_pass_forward() {

}
void DeferredPipeline::internal_pass_god_rays() {

}
void DeferredPipeline::internal_pass_hdr() {

}
void DeferredPipeline::internal_pass_bloom() {

}
void DeferredPipeline::internal_pass_depth_of_field() {

}
void DeferredPipeline::internal_pass_aa() {

}
void DeferredPipeline::internal_pass_final() {

}

void DeferredPipeline::internal_pass_depth_and_transparency() {

}
void DeferredPipeline::internal_pass_copy_depth() {

}
void DeferredPipeline::internal_pass_blur() {

}



void DeferredPipeline::update(const float& dt) {

}
void DeferredPipeline::render(const Viewport& viewport) {
    internal_render_per_frame_preparation();
    internal_pass_geometry(viewport, viewport.getCamera());
    internal_pass_ssao();
    internal_pass_stencil();
    internal_pass_lighting();
    internal_pass_forward();
    internal_pass_god_rays();
    internal_pass_hdr();
    internal_pass_bloom();
    internal_pass_depth_of_field();
    internal_pass_aa();
    internal_pass_final();

    internal_pass_depth_and_transparency(); //TODO: recheck this
    internal_pass_copy_depth();
    //internal_pass_blur();
}