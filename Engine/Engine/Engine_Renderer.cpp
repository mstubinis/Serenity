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
#include "Material.h"
#include "Object.h"
#include "ObjectDisplay.h"
#include "ObjectDynamic.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <random>
#include <math.h>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

bool Detail::RendererInfo::GeneralInfo::alpha_test = false;
bool Detail::RendererInfo::GeneralInfo::depth_test = true;
bool Detail::RendererInfo::GeneralInfo::depth_mask = true;
ShaderP* Detail::RendererInfo::GeneralInfo::current_shader_program = nullptr;
string Detail::RendererInfo::GeneralInfo::current_bound_material = "";
unsigned char Detail::RendererInfo::GeneralInfo::cull_face_status = 0; /* 0 = back | 1 = front | 2 = front and back */
bool Detail::RendererInfo::GeneralInfo::cull_face_enabled = false; //its disabled by default
GLuint Detail::RendererInfo::GeneralInfo::current_bound_read_fbo = 0;
GLuint Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo = 0;
AntiAliasingAlgorithm Detail::RendererInfo::GeneralInfo::aa_algorithm = AntiAliasingAlgorithm::FXAA;

bool Detail::RendererInfo::BloomInfo::bloom = true;
float Detail::RendererInfo::BloomInfo::bloom_radius = 0.84f;
float Detail::RendererInfo::BloomInfo::bloom_strength = 2.5f;

bool Detail::RendererInfo::LightingInfo::lighting = true;

bool Detail::RendererInfo::DebugDrawingInfo::debug = false;

bool Detail::RendererInfo::GodRaysInfo::godRays = true;
float Detail::RendererInfo::GodRaysInfo::godRays_exposure = 0.0034f;
float Detail::RendererInfo::GodRaysInfo::godRays_decay = 1.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_density = 0.84f;
float Detail::RendererInfo::GodRaysInfo::godRays_weight = 5.65f;
uint Detail::RendererInfo::GodRaysInfo::godRays_samples = 45;
float Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees = 55.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff = 2.0f;

bool Detail::RendererInfo::SSAOInfo::ssao = true;
bool Detail::RendererInfo::SSAOInfo::ssao_do_blur = true;
uint Detail::RendererInfo::SSAOInfo::ssao_samples = 16;
float Detail::RendererInfo::SSAOInfo::ssao_blur_strength = 0.5f;
float Detail::RendererInfo::SSAOInfo::ssao_scale = 0.1f;
float Detail::RendererInfo::SSAOInfo::ssao_intensity = 5.0f;
float Detail::RendererInfo::SSAOInfo::ssao_bias = 0.495f;
float Detail::RendererInfo::SSAOInfo::ssao_radius = 0.115f;
glm::vec2 Detail::RendererInfo::SSAOInfo::ssao_Kernels[Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT];
GLuint Detail::RendererInfo::SSAOInfo::ssao_noise_texture;

bool Detail::RendererInfo::HDRInfo::hdr = true;
float Detail::RendererInfo::HDRInfo::hdr_exposure = 3.2f;
float Detail::RendererInfo::HDRInfo::hdr_gamma = 1.3f;
HDRToneMapAlgorithm::Algorithm Detail::RendererInfo::HDRInfo::hdr_algorithm = HDRToneMapAlgorithm::EXPOSURE;

GBuffer* Detail::RenderManagement::m_gBuffer = nullptr;
glm::mat4 Detail::RenderManagement::m_2DProjectionMatrix;

vector<FontRenderInfo> Detail::RenderManagement::m_FontsToBeRendered;
vector<TextureRenderInfo> Detail::RenderManagement::m_TexturesToBeRendered;

vector<ShaderP*> Detail::RenderManagement::m_GeometryPassShaderPrograms;
vector<ShaderP*> Detail::RenderManagement::m_ForwardPassShaderPrograms;

void Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm algorithm){
    if(Detail::RendererInfo::GeneralInfo::aa_algorithm != algorithm){
        Detail::RendererInfo::GeneralInfo::aa_algorithm = algorithm;
    }
}
void Settings::enableCullFace(bool b){
    if(b && !Detail::RendererInfo::GeneralInfo::cull_face_enabled){
        glEnable(GL_CULL_FACE);
        Detail::RendererInfo::GeneralInfo::cull_face_enabled = true;
    }
    else if(!b && Detail::RendererInfo::GeneralInfo::cull_face_enabled){
        glDisable(GL_CULL_FACE);
        Detail::RendererInfo::GeneralInfo::cull_face_enabled = false;
    }
}
void Settings::disableCullFace(){
    if(Detail::RendererInfo::GeneralInfo::cull_face_enabled){
        glDisable(GL_CULL_FACE);
        Detail::RendererInfo::GeneralInfo::cull_face_enabled = false;
    }
}
void Settings::cullFace(uint s){
    //0 = back | 1 = front | 2 = front and back
    if(s == GL_BACK){
        if(Detail::RendererInfo::GeneralInfo::cull_face_status != 0){
            glCullFace(GL_BACK);
            Detail::RendererInfo::GeneralInfo::cull_face_status = 0;
        }
    }
    else if(s == GL_FRONT){
        if(Detail::RendererInfo::GeneralInfo::cull_face_status != 1){
            glCullFace(GL_FRONT);
            Detail::RendererInfo::GeneralInfo::cull_face_status = 1;
        }
    }
    else if(s == GL_FRONT_AND_BACK){
        if(Detail::RendererInfo::GeneralInfo::cull_face_status != 2){
            glCullFace(GL_FRONT_AND_BACK);
            Detail::RendererInfo::GeneralInfo::cull_face_status = 2;
        }
    }
}
void Settings::clear(bool color, bool depth, bool stencil){
    if(depth){ 
        enableDepthMask();
        if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
        else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
        else{ glClear(GL_DEPTH_BUFFER_BIT); }
    }else{
        if(color && stencil){ glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); }
        else if(color && !stencil){ glClear(GL_COLOR_BUFFER_BIT); }
        else{ glClear(GL_STENCIL_BUFFER_BIT); }
    }
}
void Settings::enableAlphaTest(bool b){
    if(b && !Renderer::Detail::RendererInfo::GeneralInfo::alpha_test){
        glEnable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = true;
    }
    else if(!b && Detail::RendererInfo::GeneralInfo::alpha_test){
        glDisable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = false;
    }
}
void Settings::disableAlphaTest(){
    if(Detail::RendererInfo::GeneralInfo::alpha_test){
        glDisable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = false;
    }
}
void Settings::enableDepthTest(bool b){
    if(b && !Detail::RendererInfo::GeneralInfo::depth_test){
        glEnable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = true;
    }
    else if(!b && Detail::RendererInfo::GeneralInfo::depth_test){
        glDisable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = false;
    }
}
void Settings::disableDepthTest(){
    if(Detail::RendererInfo::GeneralInfo::depth_test){
        glDisable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = false;
    }
}
void Settings::enableDepthMask(bool b){
    if(b && !Detail::RendererInfo::GeneralInfo::depth_mask){
        glDepthMask(GL_TRUE);
        Detail::RendererInfo::GeneralInfo::depth_mask = true;
    }
    else if(!b && Detail::RendererInfo::GeneralInfo::depth_mask){
        glDepthMask(GL_FALSE);
        Detail::RendererInfo::GeneralInfo::depth_mask = false;
    }
}
void Settings::disableDepthMask(){
    if(Detail::RendererInfo::GeneralInfo::depth_mask){
        glDepthMask(GL_FALSE);
        Detail::RendererInfo::GeneralInfo::depth_mask = false;
    }
}

void Renderer::bindTexture(const char* l,Texture* t,uint slot){Renderer::bindTexture(l,t->address(),slot,t->type());}
void Renderer::bindTexture(const char* l,GLuint address,uint slot,GLuint type){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(type, address);
    sendUniform1i(l,slot);
}
void Renderer::bindTextureSafe(const char* l,Texture* t,uint slot){Renderer::bindTextureSafe(l,t->address(),slot,t->type());}
void Renderer::bindTextureSafe(const char* l,GLuint address,uint slot,GLuint type){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(type, address);
    sendUniform1iSafe(l,slot);
}
void Renderer::bindReadFBO(GLuint r){
    if(Detail::RendererInfo::GeneralInfo::current_bound_read_fbo != r){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, r);
        Detail::RendererInfo::GeneralInfo::current_bound_read_fbo = r;
    }
}
void Renderer::bindDrawFBO(GLuint d){
    if(Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo != d){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d);
        Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo = d;
    }
}
void Renderer::bindFBO(GLuint fbo){Renderer::bindReadFBO(fbo); Renderer::bindDrawFBO(fbo);}
void Renderer::unbindTexture2D(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Renderer::unbindTextureCubemap(uint slot){
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Detail::RenderManagement::init(){
    #ifdef _DEBUG
    RendererInfo::DebugDrawingInfo::debug = true;
    #else
    RendererInfo::DebugDrawingInfo::debug = false;
    #endif

    uniform_real_distribution<float> randFloats(0.0f,1.0f);//random floats between 0.0-1.0
    uniform_real_distribution<float> randFloats1(0.0f,1.0f);
    default_random_engine gen;
    vector<glm::vec2> kernels;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT; ++i){
        glm::vec2 sample(randFloats(gen)*2.0f-1.0f,randFloats(gen)*2.0f-1.0f);
        sample = glm::normalize(sample);
        sample *= randFloats(gen);
        float scale = float(i) / float(RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);
        float a = 0.1f; float b = 1.0f; float f = scale * scale;
        scale = a + f * (b - a); //basic lerp
        sample *= scale;
        kernels.push_back(sample);
    }
    copy(kernels.begin(),kernels.end(),RendererInfo::SSAOInfo::ssao_Kernels);
    vector<glm::vec3> ssaoNoise;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE*RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE; i++){
        glm::vec3 noise(randFloats1(gen)*2.0-1.0,randFloats1(gen)*2.0-1.0,0.0f); 
        ssaoNoise.push_back(noise);
    } 
    glGenTextures(1, &RendererInfo::SSAOInfo::ssao_noise_texture);
    glBindTexture(GL_TEXTURE_2D, RendererInfo::SSAOInfo::ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE, 
                                              RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,float(Resources::getWindowSize().x),0.0f,float(Resources::getWindowSize().y),0.005f,1000.0f);
    
    glDepthFunc(GL_LEQUAL);
}
void Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
}
void Renderer::renderRectangle(glm::vec2& pos, glm::vec4& col, float w, float h, float angle, float depth){
    Detail::RenderManagement::getTextureRenderQueue().push_back(TextureRenderInfo("",pos,col,glm::vec2(w,h),angle,depth));
}
void Renderer::renderTexture(Texture* texture,glm::vec2& pos, glm::vec4& col,float angle, glm::vec2& scl, float depth){
    texture->render(pos,col,angle,scl,depth);
}
void Renderer::renderText(string& text,Font* font, glm::vec2& pos,glm::vec4& color, float angle, glm::vec2& scl, float depth){
    font->renderText(text,pos,color,angle,scl,depth);
}

void Detail::RenderManagement::_renderTextures(){
    ShaderP* p = Resources::getShaderProgram("Deferred_HUD"); p->bind();
    Resources::getMesh("Plane")->bind();
    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures[item.texture].get();
            bindTexture("DiffuseTexture",texture,0);
            sendUniform1i("DiffuseTextureEnabled",1);
        }
        else{
            bindTexture("DiffuseTexture",(GLuint)0,0);
            sendUniform1i("DiffuseTextureEnabled",0);
        }
        sendUniform4f("Object_Color",item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,
                                                item.pos.y,
                                                -0.001f - item.depth));
        model = glm::rotate(model, item.rot,glm::vec3(0,0,1));
        if(item.texture != "")
            model = glm::scale(model, glm::vec3(texture->width(),texture->height(),1));
        model = glm::scale(model, glm::vec3(item.scl.x,item.scl.y,1));

        sendUniformMatrix4f("VP",m_2DProjectionMatrix);
        sendUniformMatrix4f("Model",model);

        Resources::getMesh("Plane")->render();
    }
    Resources::getMesh("Plane")->unbind();
    p->unbind();
}
void Detail::RenderManagement::_renderText(){
    ShaderP* p = Resources::getShaderProgram("Deferred_HUD"); p->bind();
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts[item.texture].get();

        bindTexture("DiffuseTexture",font->getFontData()->getGlyphTexture(),0);
        sendUniform1i("DiffuseTextureEnabled",1);
        sendUniform4f("Object_Color",item.col.x,item.col.y,item.col.z,item.col.w);

        float y_offset = 0;
        float x = item.pos.x;
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

                sendUniformMatrix4f("VP",m_2DProjectionMatrix);
                sendUniformMatrix4f("Model",glyph->m_Model);
                glyph->char_mesh->bind();
                glyph->char_mesh->render();
                glyph->char_mesh->unbind();
                x += (glyph->xadvance) * item.scl.x;
            }
        }
    }
    p->unbind();
}
void Detail::RenderManagement::_passGeometry(){
    Settings::clear();
    Scene* scene = Resources::getCurrentScene();
    glm::vec3 clear = scene->getBackgroundColor();
    const float colors[4] = { clear.r, clear.g, clear.b, 1.0f };
    glClearBufferfv(GL_COLOR,BUFFER_TYPE_DIFFUSE,colors);
    glDisable(GL_BLEND); //disable blending on all mrts
    scene->renderSkybox(RendererInfo::GodRaysInfo::godRays);

    glEnablei(GL_BLEND,0); //enable blending on diffuse mrt only
    glBlendEquationi(GL_FUNC_ADD,0);
    glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

    //RENDER BACKGROUND OBJECTS THAT ARE IN FRONT OF SKYBOX HERE

    Settings::enableDepthTest();
    Settings::enableDepthMask();


    //RENDER NORMAL OBJECTS HERE
    for(auto shaderProgram:m_GeometryPassShaderPrograms){
        vector<Material*>& shaderMaterials = shaderProgram->getMaterials(); if(shaderMaterials.size() > 0){
        shaderProgram->bind();
        for(auto material:shaderMaterials){
            vector<MaterialMeshEntry*>& materialMeshes = material->getMeshEntries(); if(materialMeshes.size() > 0){
            material->bind();
            for(auto meshEntry:materialMeshes){
                meshEntry->mesh()->bind();
                for(auto instance:meshEntry->meshInstances()){
                    boost::weak_ptr<Object> o = Resources::getObjectPtr(instance.first);
                    Object* object = o.lock().get();
                    if(exists(o)){
                        if(scene->objects().count(object->name())){
                            if(object->passedRenderCheck()){
                                object->bind();
                                for(auto meshInstance:instance.second){
                                    meshInstance->bind(); //render also
                                    meshInstance->unbind();
                                }
                                object->unbind();
                            }
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if(Detail::RendererInfo::GeneralInfo::current_shader_program != shaderProgram){
                        shaderProgram->bind();
                        material->bind();
                    }
                }
                meshEntry->mesh()->unbind();
            }
            material->unbind();}
        }
        shaderProgram->unbind();}
    }
    Settings::disableDepthTest();
    Settings::disableDepthMask();

    //RENDER FOREGROUND OBJECTS HERE
}
void Detail::RenderManagement::_passForwardRendering(){
    Scene* scene = Resources::getCurrentScene();
    for(auto shaderProgram:Detail::RenderManagement::m_ForwardPassShaderPrograms){
        vector<Material*>& shaderMaterials = shaderProgram->getMaterials(); if(shaderMaterials.size() > 0){
        shaderProgram->bind();   
        for(auto material:shaderMaterials){
            vector<MaterialMeshEntry*>& materialMeshes = material->getMeshEntries(); if(materialMeshes.size() > 0){
            material->bind();
            for(auto meshEntry:materialMeshes){
                meshEntry->mesh()->bind();
                for(auto instance:meshEntry->meshInstances()){
                    boost::weak_ptr<Object> o = Resources::getObjectPtr(instance.first);
                    Object* object = o.lock().get();
                    if(exists(o)){
                        if(scene->objects().count(object->name())){
                            if(object->passedRenderCheck()){
                                object->bind();
                                for(auto meshInstance:instance.second){
                                    meshInstance->bind(); //render also
                                    meshInstance->unbind();
                                }
                                object->unbind();
                            }
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if(Detail::RendererInfo::GeneralInfo::current_shader_program != shaderProgram){
                        shaderProgram->bind();
                        material->bind();
                    }
                }
                meshEntry->mesh()->unbind();
            }
            material->unbind();}
        }
        shaderProgram->unbind();}
    }
}
void Detail::RenderManagement::_passCopyDepth(){
    glColorMask(0,0,0,0);
    ShaderP* p = Resources::getShaderProgram("Copy_Depth"); p->bind();

    bindTexture("gDepthMap",m_gBuffer->getTexture(BUFFER_TYPE_DEPTH),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    unbindTexture2D(0);
    p->unbind();
    glColorMask(1,1,1,1);
}
void Detail::RenderManagement::_passLighting(){
    glm::vec3 camPos = glm::vec3(Resources::getActiveCamera()->getPosition());
    ShaderP* p = Resources::getShaderProgram("Deferred_Light"); p->bind();

    sendUniformMatrix4f("VP",Resources::getActiveCamera()->getViewProjection());
    sendUniformMatrix4f("invVP",Resources::getActiveCamera()->getViewProjInverted());

    glm::vec3 campos = glm::vec3(Resources::getActiveCamera()->getPosition());
    Renderer::sendUniform4f("CamPosGamma",campos.x, campos.y, campos.z,RendererInfo::HDRInfo::hdr_gamma);

    uint limit = (uint)MATERIAL_COUNT_LIMIT;
    sendUniform4fv("materials[0]",Material::m_MaterialProperities,limit);

    sendUniform4f("ScreenData",Resources::getActiveCamera()->getNear(),Resources::getActiveCamera()->getFar(),
        (float)Resources::getWindowSize().x,(float)Resources::getWindowSize().y);

    bindTexture("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),0);
    bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),1);
    bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),2);
    bindTexture("gDepthMap",m_gBuffer->getTexture(BUFFER_TYPE_DEPTH),3);

    for (auto light:Resources::getCurrentScene()->lights()){
        light.second->lighten();
    }
    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::render(){
    if(!RendererInfo::GodRaysInfo::godRays)
        m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,"RGBA");
    else
        m_gBuffer->start(BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_MISC,BUFFER_TYPE_LIGHTING,"RGBA");
    _passGeometry();

    if(RendererInfo::GodRaysInfo::godRays){
        
        m_gBuffer->start(BUFFER_TYPE_GODSRAYS,"RGBA",false);
        Object* o = Resources::getObject("Sun");
        glm::vec3 sp = Math::getScreenCoordinates(glm::vec3(o->getPosition()),false);

        bool behind = Math::isPointWithinCone(Resources::getActiveCamera()->getPosition(),glm::v3(-Resources::getActiveCamera()->getViewVector()),o->getPosition(),Math::toRadians(RendererInfo::GodRaysInfo::godRays_fovDegrees));
        float alpha = Math::getAngleBetweenTwoVectors(glm::vec3(Resources::getActiveCamera()->getViewVector()),
            glm::vec3(Resources::getActiveCamera()->getPosition() - o->getPosition()),true) / RendererInfo::GodRaysInfo::godRays_fovDegrees;
        
        alpha = glm::pow(alpha,RendererInfo::GodRaysInfo::godRays_alphaFalloff);
        alpha = glm::clamp(alpha,0.001f,0.999f);

        _passGodsRays(glm::vec2(sp.x,sp.y),!behind,1.0f-alpha);
        m_gBuffer->stop();
        
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    if(RendererInfo::LightingInfo::lighting){
        m_gBuffer->start(BUFFER_TYPE_LIGHTING,"RGB");
        _passLighting();
    }
    glDisable(GL_BLEND);

    m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
    _passSSAO(); //ssao AND bloom
    if(RendererInfo::SSAOInfo::ssao_do_blur || RendererInfo::BloomInfo::bloom){
        m_gBuffer->start(BUFFER_TYPE_FREE2,"RGBA",false);
        _passBlur("Horizontal",BUFFER_TYPE_BLOOM,"RGBA");
        m_gBuffer->start(BUFFER_TYPE_BLOOM,"RGBA",false);
        _passBlur("Vertical",BUFFER_TYPE_FREE2,"RGBA");
    }
    m_gBuffer->start(BUFFER_TYPE_MISC);
    _passHDR();

    if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::None){
        m_gBuffer->stop();
        _passFinal();
    }
    else if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::FXAA){
        m_gBuffer->start(BUFFER_TYPE_LIGHTING);
        _passFinal();
        m_gBuffer->stop();
        _passFXAA();
    }
    else if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::SMAA){
        //m_gBuffer->start(BUFFER_TYPE_LIGHTING);
        //_passFinal();
        //m_gBuffer->stop();
        //_passFXAA();
    }
    _passCopyDepth();
    
    _passForwardRendering();

    glEnable(GL_BLEND);
    if(RendererInfo::DebugDrawingInfo::debug){
        Physics::Detail::PhysicsManagement::render();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Settings::enableAlphaTest();
    glAlphaFunc(GL_GREATER, 0.1f);

    Settings::enableDepthTest();
    Settings::enableDepthMask();

    Settings::clear(false,true,false); //clear depth only

    _renderTextures();
    _renderText();

    Settings::disableAlphaTest();

    m_FontsToBeRendered.clear();
    m_TexturesToBeRendered.clear();
}
void Detail::RenderManagement::_passSSAO(){
    ShaderP* p = Resources::getShaderProgram("Deferred_SSAO"); p->bind();

    sendUniform1i("doSSAO",int(RendererInfo::SSAOInfo::ssao));
    sendUniform1i("doBloom",int(RendererInfo::BloomInfo::bloom));

    Camera* c = Resources::getActiveCamera();
    glm::vec3 camPos = glm::vec3(c->getPosition());

    sendUniformMatrix4f("invVP",c->getViewProjInverted());
    sendUniform1f("nearz",c->getNear());
    sendUniform1f("farz",c->getFar());

    sendUniform3fSafe("CameraPosition",camPos.x,camPos.y,camPos.z);
    
    sendUniform4f("SSAOInfo",RendererInfo::SSAOInfo::ssao_radius,RendererInfo::SSAOInfo::ssao_intensity,
        RendererInfo::SSAOInfo::ssao_bias,RendererInfo::SSAOInfo::ssao_scale);
    
    sendUniform1i("Samples",RendererInfo::SSAOInfo::ssao_samples);
    sendUniform1i("NoiseTextureSize",RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE);
    sendUniform2fv("poisson[0]",RendererInfo::SSAOInfo::ssao_Kernels,RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);

    bindTexture("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),0);
    bindTexture("gRandomMap",RendererInfo::SSAOInfo::ssao_noise_texture,1,GL_TEXTURE_2D);
    bindTexture("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),2);
    bindTexture("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),3);
    bindTexture("gDepthMap",m_gBuffer->getTexture(BUFFER_TYPE_DEPTH),4);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 5; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::_passEdge(GLuint texture, float radius){
    Settings::clear(true,false,false);

    ShaderP* p = Resources::getShaderProgram("Deferred_Edge"); p->bind();

    sendUniform2f("gScreenSize",float(Resources::getWindowSize().x),float(Resources::getWindowSize().y));
    sendUniform1f("radius", radius);

    bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passGodsRays(glm::vec2 lightPositionOnScreen,bool behind,float alpha){
    Settings::clear(true,false,false);

    ShaderP* p = Resources::getShaderProgram("Deferred_GodsRays"); p->bind();
 
    sendUniform4f("RaysInfo",RendererInfo::GodRaysInfo::godRays_exposure,
        RendererInfo::GodRaysInfo::godRays_decay,RendererInfo::GodRaysInfo::godRays_density,
        RendererInfo::GodRaysInfo::godRays_weight);
    
    sendUniform2f("lightPositionOnScreen",
        float(lightPositionOnScreen.x)/float(Resources::getWindowSize().x),
        float(lightPositionOnScreen.y/float(Resources::getWindowSize().y))
    );

    sendUniform1i("samples",RendererInfo::GodRaysInfo::godRays_samples);
    sendUniform1i("behind",int(behind));
    sendUniform1f("alpha",alpha);

    bindTexture("firstPass",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passHDR(){
    Settings::clear(true,false,false);

    ShaderP* p = Resources::getShaderProgram("Deferred_HDR"); p->bind();

    sendUniform1fSafe("exposure",RendererInfo::HDRInfo::hdr_exposure);
    sendUniform1iSafe("HasHDR",int(RendererInfo::HDRInfo::hdr));
    sendUniform1iSafe("HasBloom",int(RendererInfo::BloomInfo::bloom));
    sendUniform1iSafe("HDRAlgorithm",int(RendererInfo::HDRInfo::hdr_algorithm));
    sendUniform1fSafe("gamma",RendererInfo::HDRInfo::hdr_gamma);

    bindTextureSafe("lightingBuffer",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),0);
    bindTextureSafe("bloomBuffer",m_gBuffer->getTexture(BUFFER_TYPE_BLOOM),1);
    bindTextureSafe("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),2);
    bindTextureSafe("gNormalMap",m_gBuffer->getTexture(BUFFER_TYPE_NORMAL),3);
    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::_passBlur(string type, GLuint texture,string channels){
    ShaderP* p = Resources::getShaderProgram("Deferred_Blur"); p->bind();

    sendUniform1f("radius",RendererInfo::BloomInfo::bloom_radius);
    sendUniform4f("strengthModifier",RendererInfo::BloomInfo::bloom_strength,
        RendererInfo::BloomInfo::bloom_strength,RendererInfo::BloomInfo::bloom_strength,RendererInfo::SSAOInfo::ssao_blur_strength);

    glm::ivec4 rgba(0);
    if(channels.find("R") != string::npos) rgba.x = 1;
    if(channels.find("G") != string::npos) rgba.y = 1;
    if(channels.find("B") != string::npos) rgba.z = 1;
    if(channels.find("A") != string::npos) rgba.w = 1;
    
    sendUniform4i("RGBA",rgba.x,rgba.y,rgba.z,rgba.w);

    if(type == "Horizontal"){ sendUniform2f("HV",1.0f,0.0f); }
    else{                     sendUniform2f("HV",0.0f,1.0f); }

    bindTexture("texture",m_gBuffer->getTexture(texture),0);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passFXAA(){
    Settings::clear(true,false,false);

    ShaderP* p = Resources::getShaderProgram("Deferred_FXAA"); p->bind();

    sendUniform2f("resolution",(float)Resources::getWindowSize().x,(float)Resources::getWindowSize().y);
    bindTexture("sampler0",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),0);
    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passFinal(){
    Settings::clear(true,false,false);

    ShaderP* p = Resources::getShaderProgram("Deferred_Final"); p->bind();

    sendUniform1iSafe("HasSSAO",int(RendererInfo::SSAOInfo::ssao));
    sendUniform1iSafe("HasLighting",int(RendererInfo::LightingInfo::lighting));
    sendUniform1iSafe("HasHDR",int(RendererInfo::HDRInfo::hdr));
    sendUniform1fSafe("gamma",RendererInfo::HDRInfo::hdr_gamma);

    bindTextureSafe("gDiffuseMap",m_gBuffer->getTexture(BUFFER_TYPE_DIFFUSE),0);
    bindTextureSafe("gLightMap",m_gBuffer->getTexture(BUFFER_TYPE_LIGHTING),1);
    bindTextureSafe("gMiscMap",m_gBuffer->getTexture(BUFFER_TYPE_MISC),2);
    bindTextureSafe("gGodsRaysMap",m_gBuffer->getTexture(BUFFER_TYPE_GODSRAYS),3);
    bindTextureSafe("gBloomMap",m_gBuffer->getTexture(BUFFER_TYPE_BLOOM),4);

    renderFullscreenQuad(Resources::getWindowSize().x,Resources::getWindowSize().y);

    for(uint i = 0; i < 5; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::renderFullscreenQuad(uint width,uint height){
    glm::mat4 m(1);
    glm::mat4 p = glm::ortho(-float(width)/2,float(width)/2,-float(height)/2,float(height)/2);

    sendUniformMatrix4f("Model",m);
    sendUniformMatrix4f("VP",p);

    glViewport(0,0,width,height);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(-float(width)/2,-float(height)/2);
        glTexCoord2f(1,0); glVertex2f(float(width)/2,-float(height)/2);
        glTexCoord2f(1,1); glVertex2f(float(width)/2,float(height)/2);
        glTexCoord2f(0,1); glVertex2f(-float(width)/2,float(height)/2);
    glEnd();
}
