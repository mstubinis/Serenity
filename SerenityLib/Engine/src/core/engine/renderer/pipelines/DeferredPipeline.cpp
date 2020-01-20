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

#include <core/engine/scene/Camera.h>

#include <ecs/ComponentBody.h>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;
using namespace Engine::Renderer;



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

void DeferredPipeline::renderSkybox(Skybox* skybox, ShaderProgram& shaderProgram, Scene& scene, Viewport& viewport, Camera& camera) {
    glm::mat4 view = camera.getView();
    Math::removeMatrixPosition(view);

    shaderProgram.bind();
    if(skybox)
        sendUniform1("IsFake", 0);
    else
        sendUniform1("IsFake", 1);
    sendUniformMatrix4("VP", camera.getProjection() * view);
    const auto& bgColor = scene.getBackgroundColor();
    sendUniform4Safe("Color", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    sendTextureSafe("Texture", skybox->texture()->address(0), 0, GL_TEXTURE_CUBE_MAP);
    Skybox::bindMesh();

    sendTextureSafe("Texture", 0, 0, GL_TEXTURE_CUBE_MAP); //this is needed to render stuff in geometry transparent using the normal deferred shader. i do not know why just yet...
    //could also change sendTexture("Texture", skybox->texture()->address(0),0, GL_TEXTURE_CUBE_MAP); above to use a different slot...
}

void DeferredPipeline::renderDirectionalLight(Camera& c, DirectionalLight& d) {
    if (!d.isActive())
        return;
    auto& body = *d.getComponent<ComponentBody>();
    const auto _forward = glm::vec3(body.forward());
    const auto& col = d.color();
    sendUniform4("light.DataA", d.getAmbientIntensity(), d.getDiffuseIntensity(), d.getSpecularIntensity(), _forward.x);
    sendUniform4("light.DataB", _forward.y, _forward.z, 0.0f, 0.0f);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(d.type()));
    sendUniform1Safe("Type", 0.0f);
    renderFullscreenTriangle();
}
void DeferredPipeline::renderSunLight(Camera& c, SunLight& s) {
    if (!s.isActive())
        return;
    auto& body = *s.getComponent<ComponentBody>();
    const auto pos = glm::vec3(body.position());
    const auto& col = s.color();
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), 0.0f);
    sendUniform4("light.DataC", 0.0f, pos.x, pos.y, pos.z);
    sendUniform4("light.DataD", col.x, col.y, col.z, static_cast<float>(s.type()));
    sendUniform1Safe("Type", 0.0f);
    renderFullscreenTriangle();
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
    auto& pointLightMesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPointLightBounds();

    pointLightMesh.bind();
    pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    pointLightMesh.unbind();
    cullFace(GL_BACK);
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
    sendUniform4("light.DataA", s.getAmbientIntensity(), s.getDiffuseIntensity(), s.getSpecularIntensity(), _forward.x);
    sendUniform4("light.DataB", _forward.y, _forward.z, s.getConstant(), s.getLinear());
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
    auto& spotLightMesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getSpotLightBounds();

    spotLightMesh.bind();
    spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    spotLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
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
    auto& rodLightMesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getRodLightBounds();

    rodLightMesh.bind();
    rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    rodLightMesh.unbind();
    cullFace(GL_BACK);
    sendUniform1Safe("Type", 0.0f); //is this really needed?
}
void DeferredPipeline::renderParticle(Particle& particle) {
    particle.getMaterial()->bind();

    auto maxTextures = epriv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;

    Camera& camera = *particle.scene().getActiveCamera();
    Renderer::sendTextureSafe("gDepthMap", m_GBuffer->getTexture(Engine::epriv::GBufferType::Depth), maxTextures);
    Renderer::sendUniform4Safe("Object_Color", particle.color());
    Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, particle.position());
    modelMatrix *= glm::mat4_cast(camera.getOrientation());
    modelMatrix = glm::rotate(modelMatrix, particle.angle(), glm::vec3(0, 0, 1));
    const auto& scale = particle.getScale();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1.0f));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

    epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh().render();
}


void DeferredPipeline::internal_render_2d_text_left(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    unsigned int i = 0;
    for (auto& character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight + 7;
            x = 0.0f;
        }else if (character != '\0') {
            const auto accum = i * 4;
            ++i;
            const FontGlyph& fontGlyph = font.getGlyphData(character);
            const float startingY = -int(fontGlyph.height + fontGlyph.yoffset) - y;

            m_Text_Indices.emplace_back(accum + 0);
            m_Text_Indices.emplace_back(accum + 1);
            m_Text_Indices.emplace_back(accum + 2);
            m_Text_Indices.emplace_back(accum + 3);
            m_Text_Indices.emplace_back(accum + 1);
            m_Text_Indices.emplace_back(accum + 0);

            const float startingX = x + fontGlyph.xoffset;
            x += fontGlyph.xadvance;

            for (unsigned char i = 0; i < 4; ++i)
                m_Text_Points.emplace_back(startingX + fontGlyph.pts[i].x, startingY + fontGlyph.pts[i].y, z);
            for(unsigned char i = 0; i < 4; ++i)
                m_Text_UVs.emplace_back(fontGlyph.uvs[i].x, fontGlyph.uvs[i].y);
        }
    }
}
void DeferredPipeline::internal_render_2d_text_center(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string>         lines;
    vector<unsigned short> lines_sizes;
    string line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            lines_sizes.push_back(static_cast<unsigned short>(x));
            line_accumulator = "";
            x = 0.0f;
            continue;
        }else if (character != '\0') {
            const auto& fontGlyph = font.getGlyphData(character);
            line_accumulator += character;
            x += fontGlyph.xadvance;
        }
    }
    if (lines.size() == 0) {
        lines.push_back(line_accumulator);
        lines_sizes.push_back(static_cast<unsigned short>(x));
    }
    x = 0.0f;
    unsigned int i = 0;
    for (unsigned int l = 0; l < lines.size(); ++l) {
        const auto& line = lines[l];
        const auto& line_size = lines_sizes[l] / 2;
        for (auto& character : line) {
            if (character != '\0') {
                const auto accum = i * 4;
                ++i;
                const auto& fontGlyph = font.getGlyphData(character);
                const float startingY = -int(fontGlyph.height + fontGlyph.yoffset) - y;

                m_Text_Indices.emplace_back(accum + 0);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 2);
                m_Text_Indices.emplace_back(accum + 3);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 0);

                const float startingX = x + fontGlyph.xoffset;
                x += fontGlyph.xadvance;

                for(unsigned short i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + fontGlyph.pts[i].x - line_size, startingY + fontGlyph.pts[i].y, z);
                for (unsigned short i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(fontGlyph.uvs[i].x, fontGlyph.uvs[i].y);
            }
        }
        y += newLineGlyphHeight + 7;
        x = 0.0f;
    }
}
void DeferredPipeline::internal_render_2d_text_right(const string& text, const Font& font, const float& newLineGlyphHeight, float& x, float& y, const float& z) {
    vector<string>  lines;
    string          line_accumulator = "";
    for (auto& character : text) {
        if (character == '\n') {
            lines.push_back(line_accumulator);
            line_accumulator = "";
            continue;
        }else if (character != '\0') {
            line_accumulator += character;
        }
    }
    if (lines.size() == 0)
        lines.push_back(line_accumulator);
    unsigned int i = 0;
    for (auto& line : lines) {
        for (int j = static_cast<int>(line.size()); j >= 0; --j) {
            const auto& character = line[j];
            if (character != '\0') {
                const auto accum = i * 4;
                ++i;
                const auto& fontGlyph = font.getGlyphData(character);
                const float startingY = -int(fontGlyph.height + fontGlyph.yoffset) - y;

                m_Text_Indices.emplace_back(accum + 0);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 2);
                m_Text_Indices.emplace_back(accum + 3);
                m_Text_Indices.emplace_back(accum + 1);
                m_Text_Indices.emplace_back(accum + 0);

                const float startingX = x + fontGlyph.xoffset;
                x -= fontGlyph.xadvance;

                for (unsigned short i = 0; i < 4; ++i)
                    m_Text_Points.emplace_back(startingX + fontGlyph.pts[i].x, startingY + fontGlyph.pts[i].y, z);
                for (unsigned short i = 0; i < 4; ++i)
                    m_Text_UVs.emplace_back(fontGlyph.uvs[i].x, fontGlyph.uvs[i].y);
            }
        }
        y += newLineGlyphHeight + 7;
        x = 0.0f;
    }
}


void DeferredPipeline::render2DText(const string& text, const Font& font, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const TextAlignment::Type& textAlignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    auto& mesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getFontMesh();
    mesh.bind();
    sendUniform1("DiffuseTextureEnabled", 1);

    const auto  newLineGlyphHeight = font.getGlyphData('X').height;
    const auto& texture = font.getGlyphTexture();
    sendTexture("DiffuseTexture", texture, 0);
    sendUniform4("Object_Color", color);
    float y = 0.0f;
    float x = 0.0f;
    float z = -0.001f - depth;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(scale.x, scale.y, 1));
    sendUniformMatrix4("Model", modelMatrix);

    if (textAlignment == TextAlignment::Left) {
        internal_render_2d_text_left(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Right) {
        internal_render_2d_text_right(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }else if (textAlignment == TextAlignment::Center) {
        internal_render_2d_text_center(text, font, static_cast<float>(newLineGlyphHeight), x, y, z);
    }
    mesh.modifyVertices(0, m_Text_Points, MeshModifyFlags::Default); //prevent gpu upload until after all the data is collected
    mesh.modifyVertices(1, m_Text_UVs);
    mesh.modifyIndices(m_Text_Indices);
    mesh.render();
}
void render2DTexture(const Texture* texture, const glm::vec2& position, const glm::vec4& color, const float angle, const glm::vec2& scale, const float depth, const Alignment::Type& align, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& mesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    mesh.bind();

    float translationX = position.x;
    float translationY = position.y;
    float totalSizeX = scale.x;
    float totalSizeY = scale.y;
    if (texture) {
        totalSizeX *= texture->width();
        totalSizeY *= texture->height();
        sendTexture("DiffuseTexture", *texture, 0);
        sendUniform1("DiffuseTextureEnabled", 1);
    }else{
        sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
        sendUniform1("DiffuseTextureEnabled", 0);
    }
    sendUniform4("Object_Color", color);
    Renderer::alignmentOffset(align, translationX, translationY, totalSizeX, totalSizeY);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(totalSizeX, totalSizeY, 1.0f));
    sendUniformMatrix4("Model", modelMatrix);
    mesh.render();
}
void DeferredPipeline::render2DTriangle(const glm::vec2& position, const glm::vec4& color, const float angle, const float width, const float height, const float depth, const Alignment::Type& alignment, const glm::vec4& scissor) {
    GLScissor(scissor);

    auto& mesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getTriangleMesh();
    mesh.bind();

    float translationX = position.x;
    float translationY = position.y;

    Renderer::alignmentOffset(alignment, translationX, translationY, width, height);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix           = glm::translate(modelMatrix, glm::vec3(translationX, translationY, -0.001f - depth));
    modelMatrix           = glm::rotate(modelMatrix, Math::toRadians(angle), glm::vec3(0, 0, 1));
    modelMatrix           = glm::scale(modelMatrix, glm::vec3(width, height, 1));

    sendTexture("DiffuseTexture", 0, 0, GL_TEXTURE_2D);
    sendUniform1("DiffuseTextureEnabled", 0);
    sendUniform4("Object_Color", color);
    sendUniformMatrix4("Model", modelMatrix);

    mesh.render();
}


void DeferredPipeline::internal_render_per_frame_preparation() {

}
void DeferredPipeline::internal_pass_geometry() {

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



void DeferredPipeline::update(const double& dt) {

}
void DeferredPipeline::render() {
    internal_render_per_frame_preparation();
    internal_pass_geometry();
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