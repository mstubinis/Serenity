#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Engine_Physics.h"
#include "ShaderProgram.h"
#include "GBuffer.h"
#include "Camera.h"
#include "Light.h"
#include "Font.h"
#include "Scene.h"
#include "Texture.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Particles.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <random>

using namespace Engine;

#ifdef _WIN32
IDXGISwapChain* Renderer::Detail::RenderManagement::m_DirectXSwapChain;
ID3D11Device* Renderer::Detail::RenderManagement::m_DirectXDevice;
ID3D11DeviceContext* Renderer::Detail::RenderManagement::m_DirectXDeviceContext;
ID3D11RenderTargetView* Renderer::Detail::RenderManagement::m_DirectXBackBuffer;
#endif

bool Renderer::Detail::RendererInfo::BloomInfo::bloom = true;
float Renderer::Detail::RendererInfo::BloomInfo::bloom_radius = 0.34f;
float Renderer::Detail::RendererInfo::BloomInfo::bloom_strength = 4.0f;

bool Renderer::Detail::RendererInfo::LightingInfo::lighting = true;

bool Renderer::Detail::RendererInfo::DebugDrawingInfo::debug = false;

bool Renderer::Detail::RendererInfo::GodRaysInfo::godRays = true;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_exposure = 0.0032f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_decay = 1.0f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_density = 0.30f;
float Renderer::Detail::RendererInfo::GodRaysInfo::godRays_weight = 5.05f;
unsigned int Renderer::Detail::RendererInfo::GodRaysInfo::godRays_samples = 75;

bool Renderer::Detail::RendererInfo::SSAOInfo::ssao = true;
bool Renderer::Detail::RendererInfo::SSAOInfo::ssao_do_blur = true;
unsigned int Renderer::Detail::RendererInfo::SSAOInfo::ssao_samples = 5;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_scale = 0.03f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_intensity = 2.2f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_bias = 0.02f;
float Renderer::Detail::RendererInfo::SSAOInfo::ssao_radius = 0.09f;
glm::vec2 Renderer::Detail::RendererInfo::SSAOInfo::ssao_Kernels[64];
GLuint Renderer::Detail::RendererInfo::SSAOInfo::ssao_noise_texture;
unsigned int Renderer::Detail::RendererInfo::SSAOInfo::ssao_noise_texture_size = 16;

bool Renderer::Detail::RendererInfo::HDRInfo::hdr = true;
float Renderer::Detail::RendererInfo::HDRInfo::hdr_exposure = 1.2f;
float Renderer::Detail::RendererInfo::HDRInfo::hdr_gamma = 0.8f;

GBuffer* Renderer::Detail::RenderManagement::m_gBuffer = nullptr;
glm::mat4 Renderer::Detail::RenderManagement::m_2DProjectionMatrix = glm::mat4(1);

std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ObjectsToBeRendered;
std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ForegroundObjectsToBeRendered;
std::vector<FontRenderInfo> Renderer::Detail::RenderManagement::m_FontsToBeRendered;
std::vector<TextureRenderInfo> Renderer::Detail::RenderManagement::m_TexturesToBeRendered;
std::vector<GeometryRenderInfo> Renderer::Detail::RenderManagement::m_ObjectsToBeForwardRendered;

void Engine::Renderer::Detail::RenderManagement::init(){
    #ifdef _DEBUG
    Detail::RendererInfo::DebugDrawingInfo::debug = true;
    #else
    Detail::RendererInfo::DebugDrawingInfo::debug = false;
    #endif

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;
    std::vector<glm::vec2> kernels;
    for (unsigned int i = 0; i < 64; ++i){
        glm::vec2 sample(
            randomFloats(generator) * 2.0f - 1.0f, 
            randomFloats(generator) * 2.0f - 1.0f
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;
        float a = 0.1f; float b = 1.0f; float f = scale * scale;
        scale = a + f * (b - a); //basic lerp

        sample *= scale;
        kernels.push_back(sample);
    }
    std::copy(kernels.begin(),kernels.end(),Detail::RendererInfo::SSAOInfo::ssao_Kernels);

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < Detail::RendererInfo::SSAOInfo::ssao_noise_texture_size*Detail::RendererInfo::SSAOInfo::ssao_noise_texture_size; i++){
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            0.0f); 
        ssaoNoise.push_back(noise);
    } 
    glGenTextures(1, &Detail::RendererInfo::SSAOInfo::ssao_noise_texture);
    glBindTexture(GL_TEXTURE_2D, Detail::RendererInfo::SSAOInfo::ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Detail::RendererInfo::SSAOInfo::ssao_noise_texture_size, 
                                              Detail::RendererInfo::SSAOInfo::ssao_noise_texture_size, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,(float)Resources::getWindowSize().x,0.0f,(float)Resources::getWindowSize().y,0.005f,1000.0f);
}
void Engine::Renderer::Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
    #ifdef _WIN32
    SAFE_DELETE_COM(RenderManagement::m_DirectXSwapChain);
    SAFE_DELETE_COM(RenderManagement::m_DirectXBackBuffer);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDevice);
    SAFE_DELETE_COM(RenderManagement::m_DirectXDeviceContext);
    #endif
}
void Engine::Renderer::renderRectangle(glm::vec2 pos, glm::vec4 color, float width, float height, float angle, float depth){
    Renderer::Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,color,glm::vec2(width,height),angle,depth));
}
void Engine::Renderer::Detail::RenderManagement::_renderObjects(){
    for(auto item:m_ObjectsToBeRendered){
		item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug,RendererInfo::GodRaysInfo::godRays);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderForegroundObjects(){
    for(auto item:m_ForegroundObjectsToBeRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderForwardRenderedObjects(){
    for(auto item:m_ObjectsToBeForwardRendered){
        item.object->draw(item.shader,RendererInfo::DebugDrawingInfo::debug);
    }
}
void Engine::Renderer::Detail::RenderManagement::_renderTextures(){
    GLuint shader = Resources::getShader("Deferred_HUD")->program();
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "far"),Resources::getActiveCamera()->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures[item.texture].get();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->address());
            glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);
        }
        else{
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
            glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),0);
        }
        glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);
        glUniform4f(glGetUniformLocation(shader,"Object_Color"),item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,
                                                Resources::getWindowSize().y-item.pos.y,
                                                -0.001f - item.depth));
        model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
        if(item.texture != "")
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
        model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

        glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(m_2DProjectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(model));

        Resources::getMesh("Plane")->render();
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_renderText(){
    GLuint shader = Resources::getShader("Deferred_HUD")->program();
    glUseProgram(shader);
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture].get();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font->getFontData()->getGlyphTexture()->address());
        glUniform1i(glGetUniformLocation(shader,"DiffuseMap"),0);
        glUniform1i(glGetUniformLocation(shader,"DiffuseMapEnabled"),1);
        glUniform1i(glGetUniformLocation(shader,"Shadeless"),1);

        glUniform4f(glGetUniformLocation(shader, "Object_Color"),item.col.x,item.col.y,item.col.z,item.col.w);

        float y_offset = 0;
        float x = item.pos.x;
        item.pos.y = Resources::getWindowSize().y - item.pos.y;
        for(auto c:item.text){
            if(c == '\n'){
                y_offset += (font->getFontData()->getGlyphData('X')->height+6) * item.scl.y;
                x = item.pos.x;
            }
            else{
                FontGlyph* glyph = font->getFontData()->getGlyphData(c);

                glyph->m_Model = glm::mat4(1);
                glyph->m_Model = glm::translate(glyph->m_Model, glm::vec3(x + glyph->xoffset ,item.pos.y - (glyph->height + glyph->yoffset) - y_offset,-0.001f - item.depth));
                glyph->m_Model = glm::rotate(glyph->m_Model, item.rot,glm::vec3(0,0,1));
                glyph->m_Model = glm::scale(glyph->m_Model, glm::vec3(item.scl.x,item.scl.y,1));

                glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(m_2DProjectionMatrix));
                glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(glyph->m_Model));

                glyph->char_mesh->render();
                x += (glyph->xadvance) * item.scl.x;
            }
        }
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passGeometry(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Scene* s = Resources::getCurrentScene();
    glm::vec3 clear = s->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    const float black[4] = {0.0f,0.0f,0.0f,0.0f};
    glClearBufferfv(GL_COLOR,BUFFER_TYPE_DIFFUSE,colors);
    glDisable(GL_BLEND); //disable blending on all mrts

    s->renderSkybox(RendererInfo::GodRaysInfo::godRays);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnablei(GL_BLEND,0); //enable blending on diffuse mrt only
    glBlendEquationi(GL_FUNC_ADD,0);
    glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

    _renderObjects();

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}
void Engine::Renderer::Detail::RenderManagement::_passLighting(){
    GLuint shader = Resources::getShader("Deferred_Light")->program();
    glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
    glUseProgram(shader);

    glUniform1i(glGetUniformLocation(shader,"HasSSAO"),int(RendererInfo::SSAOInfo::ssao));

    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(Resources::getActiveCamera()->getViewProjection()));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
    glUniform1i( glGetUniformLocation(shader,"gPositionMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_MISC));
    glUniform1i( glGetUniformLocation(shader,"gMiscMap"), 2 );

    glActiveTexture(GL_TEXTURE3);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE));
    glUniform1i( glGetUniformLocation(shader,"gDiffuseMap"), 3 );

    for (auto light:Resources::getCurrentScene()->getLights()){
        light.second->lighten(shader);
    }
    // Reset OpenGL state
    for(uint i = 0; i < 4; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::render(){
    for(auto object:Resources::getCurrentScene()->getObjects()){ object.second->render(0,RendererInfo::DebugDrawingInfo::debug); }

	if(!RendererInfo::GodRaysInfo::godRays)
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION);
	else
		m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_POSITION,BUFFER_TYPE_FREE1);
	RenderManagement::_passGeometry();
	m_gBuffer->stop();

	if(RendererInfo::GodRaysInfo::godRays){
		m_gBuffer->start(BUFFER_TYPE_GODSRAYS);
		Object* o = Resources::getObject("Sun");
		glm::vec3 sp = Math::getScreenCoordinates(glm::vec3(o->getPosition()),false);
		_passGodsRays(glm::vec2(sp.x,sp.y));
		m_gBuffer->stop();
	}
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    if(RendererInfo::SSAOInfo::ssao){
        m_gBuffer->start(BUFFER_TYPE_MISC,"G");
        RenderManagement::_passSSAO();
        m_gBuffer->stop();
        if(RendererInfo::SSAOInfo::ssao_do_blur){
            m_gBuffer->start(BUFFER_TYPE_FREE1,"G");
            RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_MISC,0.5f,0.5f,"G");
            m_gBuffer->stop();
            m_gBuffer->start(BUFFER_TYPE_MISC,"G");
            RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE1,0.5f,0.5f,"G");
            m_gBuffer->stop();
        }
    }
    if(RendererInfo::LightingInfo::lighting){
        m_gBuffer->start(BUFFER_TYPE_LIGHTING,BUFFER_TYPE_BLOOM);
        RenderManagement::_passLighting();
        m_gBuffer->stop();
    }
    if(RendererInfo::BloomInfo::bloom){
        glDisable(GL_BLEND);
        m_gBuffer->start(BUFFER_TYPE_FREE1);
        RenderManagement::_passBlur("Horizontal",BUFFER_TYPE_BLOOM,RendererInfo::BloomInfo::bloom_radius,RendererInfo::BloomInfo::bloom_strength);
        m_gBuffer->stop();
        m_gBuffer->start(BUFFER_TYPE_BLOOM);
        RenderManagement::_passBlur("Vertical",BUFFER_TYPE_FREE1,RendererInfo::BloomInfo::bloom_radius,RendererInfo::BloomInfo::bloom_strength);
        m_gBuffer->stop();
    }
    if(RendererInfo::HDRInfo::hdr){
        m_gBuffer->start(BUFFER_TYPE_MISC);
        RenderManagement::_passHDR();
        m_gBuffer->stop();
    }
    RenderManagement::_passFinal();

    //copy depth over
    glColorMask(0,0,0,0);
    GLuint shader = Resources::getShader("Copy_Depth")->program();
    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DEPTH));
    glUniform1i(glGetUniformLocation(shader,"gDepthMap"), 0 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
    glColorMask(1,1,1,1);
    /////////////

    glEnable(GL_BLEND);
    if(RendererInfo::DebugDrawingInfo::debug)
        Physics::Detail::PhysicsManagement::render();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    _renderForwardRenderedObjects();
    _renderForegroundObjects();

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_DEPTH_BUFFER_BIT);

    RenderManagement::_renderTextures();
    RenderManagement::_renderText();

    glDisable(GL_ALPHA_TEST);

	m_ObjectsToBeRendered.clear();
	m_ForegroundObjectsToBeRendered.clear();
	m_ObjectsToBeForwardRendered.clear();
	m_FontsToBeRendered.clear();
	m_TexturesToBeRendered.clear();
}
void Engine::Renderer::Detail::RenderManagement::_passSSAO(){
    GLuint shader = Resources::getShader("Deferred_SSAO")->program();
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniform1f(glGetUniformLocation(shader,"gIntensity"),RendererInfo::SSAOInfo::ssao_intensity);
    glUniform1f(glGetUniformLocation(shader,"gBias"),RendererInfo::SSAOInfo::ssao_bias);
    glUniform1f(glGetUniformLocation(shader,"gRadius"),RendererInfo::SSAOInfo::ssao_radius);
    glUniform1f(glGetUniformLocation(shader,"gScale"),RendererInfo::SSAOInfo::ssao_scale);
    glUniform1i(glGetUniformLocation(shader,"gSampleCount"),RendererInfo::SSAOInfo::ssao_samples);
    glUniform1i(glGetUniformLocation(shader,"gNoiseTextureSize"),RendererInfo::SSAOInfo::ssao_noise_texture_size);
    glUniform2fv(glGetUniformLocation(shader,"poisson"),64, glm::value_ptr(RendererInfo::SSAOInfo::ssao_Kernels[0]));
    glUniform1i(glGetUniformLocation(shader,"far"),int(Resources::getActiveCamera()->getFar()));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i(glGetUniformLocation(shader,"gNormalMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_POSITION));
    glUniform1i(glGetUniformLocation(shader,"gPositionMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,RendererInfo::SSAOInfo::ssao_noise_texture);
    glUniform1i(glGetUniformLocation(shader,"gRandomMap"), 2 );


    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 3; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passEdge(GLuint texture, float radius){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_Edge")->program();
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniform1f(glGetUniformLocation(shader,"radius"), radius);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
    glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passGodsRays(glm::vec2 lightPositionOnScreen){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_GodsRays")->program();
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader,"decay"), RendererInfo::GodRaysInfo::godRays_decay);
    glUniform1f(glGetUniformLocation(shader,"density"), RendererInfo::GodRaysInfo::godRays_density);
    glUniform1f(glGetUniformLocation(shader,"exposure"), RendererInfo::GodRaysInfo::godRays_exposure);
    glUniform1i(glGetUniformLocation(shader,"samples"), RendererInfo::GodRaysInfo::godRays_samples);
    glUniform1f(glGetUniformLocation(shader,"weight"), RendererInfo::GodRaysInfo::godRays_weight);
    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniform2f(glGetUniformLocation(shader,"lightPositionOnScreen"),float(lightPositionOnScreen.x),float(lightPositionOnScreen.y));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_FREE1));
    glUniform1i(glGetUniformLocation(shader,"firstPass"), 0 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passHDR(){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_HDR")->program();
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader,"gamma"),RendererInfo::HDRInfo::hdr_gamma);
    glUniform1f(glGetUniformLocation(shader,"exposure"),RendererInfo::HDRInfo::hdr_exposure);
    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
    glUniform1i(glGetUniformLocation(shader,"lightingBuffer"), 0 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 1; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, i);
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passBlur(std::string type, GLuint texture, float radius,float strengthModifier,std::string channels){
    GLuint shader = Resources::getShader("Deferred_Blur")->program();
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glUniform1f(glGetUniformLocation(shader,"radius"), radius);
    glUniform1f(glGetUniformLocation(shader,"strengthModifier"), strengthModifier);

    if(channels.find("R") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"R"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"R"), 0);
    if(channels.find("G") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"G"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"G"), 0);
    if(channels.find("B") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"B"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"B"), 0);
    if(channels.find("A") != std::string::npos) glUniform1i(glGetUniformLocation(shader,"A"), 1);
    else                                        glUniform1i(glGetUniformLocation(shader,"A"), 0);

    if(type == "Horizontal"){ 
        glUniform2f(glGetUniformLocation(shader,"HV"), 1.0f,0.0f);
    }
    else{
        glUniform2f(glGetUniformLocation(shader,"HV"), 0.0f,1.0f);
    }
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(texture));
    glUniform1i(glGetUniformLocation(shader,"texture"), 0 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}
void Engine::Renderer::Detail::RenderManagement::_passFinal(){
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint shader = Resources::getShader("Deferred_Final")->program();
    glUseProgram(shader);

	glUniform1f(glGetUniformLocation(shader,"gamma"),RendererInfo::HDRInfo::hdr_gamma);
    glUniform2f(glGetUniformLocation(shader,"gScreenSize"),float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    glm::vec3 ambient = Resources::getCurrentScene()->getAmbientLightColor();
    glUniform3f(glGetUniformLocation(shader,"gAmbientColor"),ambient.x,ambient.y,ambient.z);

    glUniform1i( glGetUniformLocation(shader,"HasLighting"),int(RendererInfo::LightingInfo::lighting));
    glUniform1i( glGetUniformLocation(shader,"HasBloom"),int(RendererInfo::BloomInfo::bloom));
	glUniform1i( glGetUniformLocation(shader,"HasHDR"),int(RendererInfo::HDRInfo::hdr));

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE));
    glUniform1i( glGetUniformLocation(shader,"gDiffuseMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING));
    glUniform1i( glGetUniformLocation(shader,"gLightMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_BLOOM));
    glUniform1i( glGetUniformLocation(shader,"gBloomMap"), 2 );

    glActiveTexture(GL_TEXTURE3);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_NORMAL));
    glUniform1i( glGetUniformLocation(shader,"gNormalMap"), 3 );

    glActiveTexture(GL_TEXTURE4);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_MISC));
    glUniform1i( glGetUniformLocation(shader,"gMiscMap"), 4 );

    glActiveTexture(GL_TEXTURE5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(BUFFER_TYPE_GODSRAYS));
    glUniform1i( glGetUniformLocation(shader,"gGodsRaysMap"), 5 );

    Engine::Renderer::Detail::renderFullscreenQuad(shader,Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 6; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}
void Engine::Renderer::Detail::renderFullscreenQuad(GLuint shader,uint width,uint height,float scale){
    glm::mat4 m(1);
    glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(m));
    glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(m));
    glViewport(0,0,Resources::getWindowSize().x,Resources::getWindowSize().y);
    glBegin(GL_QUADS);
        glVertex2f(-1,-1);
        glVertex2f(1,-1);
        glVertex2f(1,1);
        glVertex2f(-1,1);
    glEnd();
}

#ifdef _WIN32
void Engine::Renderer::Detail::RenderManagement::renderDirectX(){
    // clear the back buffer to a deep blue
    Scene* s = Resources::getCurrentScene();
    glm::vec3 clear = s->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    m_DirectXDeviceContext->ClearRenderTargetView(m_DirectXBackBuffer,colors);
    // do 3D rendering on the back buffer here

    // switch the back buffer and the front buffer
    m_DirectXSwapChain->Present(0, 0);
}
#endif