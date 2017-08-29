#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Engine_Physics.h"
#include "Engine_Window.h"
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
#include "FramebufferObject.h"
#include "SMAA_LUT.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/lexical_cast.hpp>
#include <random>
#include <math.h>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

float Detail::RendererInfo::FXAAInfo::FXAA_REDUCE_MIN = 0.0078125f; // (1 / 128)
float Detail::RendererInfo::FXAAInfo::FXAA_REDUCE_MUL = 0.125f; // (1 / 8)
float Detail::RendererInfo::FXAAInfo::FXAA_SPAN_MAX = 8.0f;

GLuint Detail::RendererInfo::SMAAInfo::SMAA_AreaTexture;
GLuint Detail::RendererInfo::SMAAInfo::SMAA_SearchTexture;
float Detail::RendererInfo::SMAAInfo::SMAA_THRESHOLD = 0.05f;
uint Detail::RendererInfo::SMAAInfo::SMAA_MAX_SEARCH_STEPS = 32;
uint Detail::RendererInfo::SMAAInfo::SMAA_MAX_SEARCH_STEPS_DIAG = 16;
uint Detail::RendererInfo::SMAAInfo::SMAA_CORNER_ROUNDING = 25;
float Detail::RendererInfo::SMAAInfo::SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR = 2.0f;
float Detail::RendererInfo::SMAAInfo::SMAA_DEPTH_THRESHOLD = (0.1f * Detail::RendererInfo::SMAAInfo::SMAA_THRESHOLD);

bool Detail::RendererInfo::SMAAInfo::SMAA_PREDICATION = false;
float Detail::RendererInfo::SMAAInfo::SMAA_PREDICATION_THRESHOLD = 0.01f;
float Detail::RendererInfo::SMAAInfo::SMAA_PREDICATION_SCALE = 2.0f;
float Detail::RendererInfo::SMAAInfo::SMAA_PREDICATION_STRENGTH = 0.4f;

bool Detail::RendererInfo::SMAAInfo::SMAA_REPROJECTION = false;
float Detail::RendererInfo::SMAAInfo::SMAA_REPROJECTION_WEIGHT_SCALE = 30.0f;

uint Detail::RendererInfo::SMAAInfo::SMAA_AREATEX_MAX_DISTANCE = 16;
uint Detail::RendererInfo::SMAAInfo::SMAA_AREATEX_MAX_DISTANCE_DIAG = 20;
glm::vec2 Detail::RendererInfo::SMAAInfo::SMAA_AREATEX_PIXEL_SIZE = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
float Detail::RendererInfo::SMAAInfo::SMAA_AREATEX_SUBTEX_SIZE = 1.0f / 7.0f;


float Detail::RendererInfo::GeneralInfo::gamma = 2.2f;
bool Detail::RendererInfo::GeneralInfo::alpha_test = false;
bool Detail::RendererInfo::GeneralInfo::depth_test = true;
bool Detail::RendererInfo::GeneralInfo::depth_mask = true;
ShaderP* Detail::RendererInfo::GeneralInfo::current_shader_program = nullptr;
string Detail::RendererInfo::GeneralInfo::current_bound_material = "";
unsigned char Detail::RendererInfo::GeneralInfo::cull_face_status = 0; /* 0 = back | 1 = front | 2 = front and back */
bool Detail::RendererInfo::GeneralInfo::cull_face_enabled = false; //its disabled by default
GLuint Detail::RendererInfo::GeneralInfo::current_bound_read_fbo = 0;
GLuint Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo = 0;
GLuint Detail::RendererInfo::GeneralInfo::current_bound_rbo = 0;
AntiAliasingAlgorithm::Algorithm Detail::RendererInfo::GeneralInfo::aa_algorithm = AntiAliasingAlgorithm::FXAA;
glm::uvec4 Detail::RendererInfo::GeneralInfo::gl_viewport_data = glm::uvec4(0,0,0,0);


#ifdef _DEBUG
    bool Detail::RendererInfo::GeneralInfo::draw_physics_debug = true;
#else
    bool Detail::RendererInfo::GeneralInfo::draw_physics_debug = false;
#endif

bool Detail::RendererInfo::BloomInfo::bloom = true;
float Detail::RendererInfo::BloomInfo::bloom_radius = 0.84f;
float Detail::RendererInfo::BloomInfo::bloom_strength = 2.5f;

bool Detail::RendererInfo::LightingInfo::lighting = true;

bool Detail::RendererInfo::GodRaysInfo::godRays = true;
float Detail::RendererInfo::GodRaysInfo::godRays_exposure = 0.0034f;
float Detail::RendererInfo::GodRaysInfo::godRays_decay = 1.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_density = 0.84f;
float Detail::RendererInfo::GodRaysInfo::godRays_weight = 5.65f;
uint Detail::RendererInfo::GodRaysInfo::godRays_samples = 45;
float Detail::RendererInfo::GodRaysInfo::godRays_fovDegrees = 75.0f;
float Detail::RendererInfo::GodRaysInfo::godRays_alphaFalloff = 2.0f;

bool Detail::RendererInfo::SSAOInfo::ssao = true;
bool Detail::RendererInfo::SSAOInfo::ssao_do_blur = true;
uint Detail::RendererInfo::SSAOInfo::ssao_samples = 8;
float Detail::RendererInfo::SSAOInfo::ssao_blur_strength = 0.5f;
float Detail::RendererInfo::SSAOInfo::ssao_scale = 1.0f;
float Detail::RendererInfo::SSAOInfo::ssao_intensity = 1.1f;
float Detail::RendererInfo::SSAOInfo::ssao_bias = 0.495f;
float Detail::RendererInfo::SSAOInfo::ssao_radius = 0.5f;
glm::vec3 Detail::RendererInfo::SSAOInfo::ssao_Kernels[Renderer::Detail::RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT];
GLuint Detail::RendererInfo::SSAOInfo::ssao_noise_texture;

bool Detail::RendererInfo::HDRInfo::hdr = true;
float Detail::RendererInfo::HDRInfo::hdr_exposure = 3.0f;
HDRToneMapAlgorithm::Algorithm Detail::RendererInfo::HDRInfo::hdr_algorithm = HDRToneMapAlgorithm::UNCHARTED;

GBuffer* Detail::RenderManagement::m_gBuffer = nullptr;
glm::mat4 Detail::RenderManagement::m_2DProjectionMatrix;

vector<FontRenderInfo> Detail::RenderManagement::m_FontsToBeRendered;
vector<TextureRenderInfo> Detail::RenderManagement::m_TexturesToBeRendered;

vector<ShaderP*> Detail::RenderManagement::m_GeometryPassShaderPrograms;
vector<ShaderP*> Detail::RenderManagement::m_ForwardPassShaderPrograms;



void _generateBRDFLUTCookTorrance(uint brdfSize){
    uint& prevReadBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_read_fbo;
    uint& prevDrawBuffer = Renderer::Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo;

    FramebufferObject* fbo = new FramebufferObject("BRDFLUT_Gen_CookTorr_FBO",brdfSize,brdfSize,ImageInternalFormat::Depth16);
    fbo->bind();

    Texture* t = Resources::getTexture("BRDFCookTorrance");

    glBindTexture(GL_TEXTURE_2D, t->address());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfSize, brdfSize, 0, GL_RG, GL_FLOAT, 0);
    Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToEdge);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,t->address(), 0);

    ShaderP* p = Resources::getShaderProgram("BRDF_Precompute_CookTorrance"); p->bind();
    Renderer::sendUniform1i("NUM_SAMPLES",256);
    Renderer::Settings::clear(true,true,false);
    glColorMask(GL_TRUE,GL_TRUE,GL_FALSE,GL_FALSE);
    Renderer::Detail::renderFullscreenQuad(brdfSize,brdfSize);
    cout << "----  BRDF LUT (Cook Torrance) completed ----" << endl;
    p->unbind();
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    delete fbo;
    Renderer::bindReadFBO(prevReadBuffer);
    Renderer::bindDrawFBO(prevDrawBuffer);
}

void Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::Algorithm algorithm){
    if(Detail::RendererInfo::GeneralInfo::aa_algorithm != algorithm){
        Detail::RendererInfo::GeneralInfo::aa_algorithm = algorithm;
    }
}
void Settings::enableCullFace(bool b){
    if(b == true && Detail::RendererInfo::GeneralInfo::cull_face_enabled == false){
        glEnable(GL_CULL_FACE);
        Detail::RendererInfo::GeneralInfo::cull_face_enabled = true;
    }
    else if(b == false && Detail::RendererInfo::GeneralInfo::cull_face_enabled == true){
        glDisable(GL_CULL_FACE);
        Detail::RendererInfo::GeneralInfo::cull_face_enabled = false;
    }
}
void Settings::disableCullFace(){
    if(Detail::RendererInfo::GeneralInfo::cull_face_enabled == true){
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
    if(!color && !depth && !stencil) return;

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
    if(b == true && Renderer::Detail::RendererInfo::GeneralInfo::alpha_test == false){
        glEnable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = true;
    }
    else if(b == false && Detail::RendererInfo::GeneralInfo::alpha_test == true){
        glDisable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = false;
    }
}
void Settings::disableAlphaTest(){
    if(Detail::RendererInfo::GeneralInfo::alpha_test == true){
        glDisable(GL_ALPHA_TEST);
        Detail::RendererInfo::GeneralInfo::alpha_test = false;
    }
}
void Settings::enableDepthTest(bool b){
    if(b == true && Detail::RendererInfo::GeneralInfo::depth_test == false){
        glEnable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = true;
    }
    else if(b == false && Detail::RendererInfo::GeneralInfo::depth_test == true){
        glDisable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = false;
    }
}
void Settings::disableDepthTest(){
    if(Detail::RendererInfo::GeneralInfo::depth_test == true){
        glDisable(GL_DEPTH_TEST);
        Detail::RendererInfo::GeneralInfo::depth_test = false;
    }
}
void Settings::enableDepthMask(bool b){
    if(b == true && Detail::RendererInfo::GeneralInfo::depth_mask == false){
        glDepthMask(GL_TRUE);
        Detail::RendererInfo::GeneralInfo::depth_mask = true;
    }
    else if(b == false && Detail::RendererInfo::GeneralInfo::depth_mask == true){
        glDepthMask(GL_FALSE);
        Detail::RendererInfo::GeneralInfo::depth_mask = false;
    }
}
void Settings::disableDepthMask(){
    if(Detail::RendererInfo::GeneralInfo::depth_mask == true){
        glDepthMask(GL_FALSE);
        Detail::RendererInfo::GeneralInfo::depth_mask = false;
    }
}
void Settings::enableDrawPhysicsInfo(bool b){ Detail::RendererInfo::GeneralInfo::draw_physics_debug = b; }
void Settings::disableDrawPhysicsInfo(){ Detail::RendererInfo::GeneralInfo::draw_physics_debug = false; }
void Settings::setGamma(float g){ Detail::RendererInfo::GeneralInfo::gamma = g; }
float Settings::getGamma(){ return Detail::RendererInfo::GeneralInfo::gamma; }

void Renderer::setViewport(uint x, uint y, uint _w, uint _h){
    glm::uvec4& viewport = Detail::RendererInfo::GeneralInfo::gl_viewport_data;
    if(viewport.x == x && viewport.y == y && viewport.z == _w && viewport.w == _h) return;
    glViewport(GLint(x), GLint(y), GLsizei(_w), GLsizei(_h));
    Detail::RendererInfo::GeneralInfo::gl_viewport_data = glm::uvec4(x,y,_w,_h);
}
void Renderer::bindTexture(const char* l,Texture* t,uint s){Renderer::bindTexture(l,t->address(),s,t->type());}
void Renderer::bindTexture(const char* l,GLuint a,uint s,GLuint t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t,a);
    sendUniform1i(l,s);
}
void Renderer::bindTextureSafe(const char* l,Texture* t,uint slot){Renderer::bindTextureSafe(l,t->address(),slot,t->type());}
void Renderer::bindTextureSafe(const char* l,GLuint a,uint s,GLuint t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t,a);
    sendUniform1iSafe(l,s);
}
void Renderer::bindReadFBO(GLuint r){
    if(Detail::RendererInfo::GeneralInfo::current_bound_read_fbo != r){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, r);
        Detail::RendererInfo::GeneralInfo::current_bound_read_fbo = r;
    }
}
void Renderer::bindFBO(FramebufferObject* fbo){
    Renderer::bindFBO(fbo->address());
}
void Renderer::bindRBO(RenderbufferObject* rbo){
    Renderer::bindRBO(rbo->address());
}
void Renderer::bindDrawFBO(GLuint d){
    if(Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo != d){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, d);
        Detail::RendererInfo::GeneralInfo::current_bound_draw_fbo = d;
    }
}
void Renderer::bindFBO(GLuint f){Renderer::bindReadFBO(f);Renderer::bindDrawFBO(f);}
void Renderer::bindRBO(GLuint r){
    if(Detail::RendererInfo::GeneralInfo::current_bound_rbo != r){
        glBindRenderbuffer(GL_RENDERBUFFER, r);
        Detail::RendererInfo::GeneralInfo::current_bound_rbo = r;
    }
}
void Renderer::unbindFBO(){ Renderer::bindFBO(GLuint(0)); }
void Renderer::unbindRBO(){ Renderer::bindRBO(GLuint(0)); }
void Renderer::unbindReadFBO(){ Renderer::bindReadFBO(0); }
void Renderer::unbindDrawFBO(){ Renderer::bindDrawFBO(0); }
void Renderer::unbindTexture(uint s,Texture* t){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(t->type(),0);
}
void Renderer::unbindTexture2D(uint s){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(GL_TEXTURE_2D,0);
}
void Renderer::unbindTextureCubemap(uint s){
    glActiveTexture(GL_TEXTURE0+s);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
}
void Detail::RenderManagement::init(){
    uniform_real_distribution<float> randFloats(0.0f,1.0f);
    default_random_engine gen;
    vector<glm::vec3> kernels;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT; ++i){
        glm::vec3 sample(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,randFloats(gen));
        sample = glm::normalize(sample);
        sample *= randFloats(gen);
        float scale = float(i) / float(RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);
        // scale samples s.t. they're more aligned to center of kernel
        float a = 0.1f; float b = 1.0f; float f = scale * scale;
        scale = a + f * (b - a); //basic lerp   
        sample *= scale;
        kernels.push_back(sample);
    }
    copy(kernels.begin(),kernels.end(),RendererInfo::SSAOInfo::ssao_Kernels);
    vector<glm::vec3> ssaoNoise;
    for(uint i = 0; i < RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE*RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE; i++){
        glm::vec3 noise(randFloats(gen)*2.0-1.0,randFloats(gen)*2.0-1.0,0.0f); 
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
    Settings::enableDepthTest(true);
    glDepthFunc(GL_LEQUAL);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1); //for non Power of Two textures
    
    //recommended for specular IBL. but causes HUGE fps drops (prob because OGL 3.3 does not support this). investigate this...
    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // Create SMAA lookup textures
    //area
    glGenTextures(1,&RendererInfo::SMAAInfo::SMAA_AreaTexture);
    glBindTexture(GL_TEXTURE_2D,RendererInfo::SMAAInfo::SMAA_AreaTexture);
    Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG8,160,560,0,GL_RG,GL_UNSIGNED_BYTE,areaTexBytes);
    glBindTexture(GL_TEXTURE_2D,0);

    //search
    glGenTextures(1,&RendererInfo::SMAAInfo::SMAA_SearchTexture);
    glBindTexture(GL_TEXTURE_2D,RendererInfo::SMAAInfo::SMAA_SearchTexture);
    Texture::setFilter(GL_TEXTURE_2D,TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D,TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R8,64,16,0,GL_RED,GL_UNSIGNED_BYTE,searchTexBytes);
    glBindTexture(GL_TEXTURE_2D,0);
}
void Detail::RenderManagement::postInit(){
    _generateBRDFLUTCookTorrance(512);
}
void Detail::RenderManagement::destruct(){
    SAFE_DELETE(RenderManagement::m_gBuffer);
    glDeleteTextures(1,&RendererInfo::SSAOInfo::ssao_noise_texture);
    glDeleteTextures(1,&RendererInfo::SMAAInfo::SMAA_SearchTexture);
    glDeleteTextures(1,&RendererInfo::SMAAInfo::SMAA_AreaTexture);
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
void Detail::RenderManagement::_renderTextures(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
    ShaderP* p = Resources::getShaderProgram("Deferred_HUD"); p->bind();
    Resources::getMesh("Plane")->bind();
    for(auto item:m_TexturesToBeRendered){
        Texture* texture = nullptr;
        if(item.texture != ""){
            texture = Resources::Detail::ResourceManagement::m_Textures.at(item.texture).get();
            bindTexture("DiffuseTexture",texture,0);
            sendUniform1i("DiffuseTextureEnabled",1);
        }
        else{
            bindTexture("DiffuseTexture",(GLuint)0,0);
            sendUniform1i("DiffuseTextureEnabled",0);
        }
        sendUniform4f("Object_Color",item.col.r,item.col.g,item.col.b,item.col.a);

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(item.pos.x,item.pos.y,-0.001f - item.depth));
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
void Detail::RenderManagement::_renderText(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
    ShaderP* p = Resources::getShaderProgram("Deferred_HUD"); p->bind();
    for(auto item:m_FontsToBeRendered){
        Font* font = Resources::Detail::ResourceManagement::m_Fonts.at(item.texture).get();

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
void Detail::RenderManagement::_passGeometry(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,Object* ignore){
    if(Detail::RendererInfo::GodRaysInfo::godRays)
        gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); 
    else
        gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA");
 
    Settings::clear();
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND); //disable blending on all mrts

    Scene* scene = Resources::getCurrentScene();
    glm::vec3 clear = scene->getBackgroundColor();
    const float colors[4] = { clear.r,clear.g,clear.b,1.0f };

    glClearBufferfv(GL_COLOR,0,colors);
    if(Detail::RendererInfo::GodRaysInfo::godRays){
        const float godRays[4] = { 0.03f,0.023f,0.032f,1.0f };
        glClearBufferfv(GL_COLOR,3,godRays);
    }
    gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,"RGBA");
    scene->renderSkybox();
    if(Detail::RendererInfo::GodRaysInfo::godRays)
        gbuffer->start(GBufferType::Diffuse,GBufferType::Normal,GBufferType::Misc,GBufferType::Lighting,"RGBA"); 

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
                        if(scene->objects().count(object->name()) && (object != ignore)){
                            if(object->checkRender(c)){
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
void Detail::RenderManagement::_passForwardRendering(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,Object* ignore){
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
                        if(scene->objects().count(object->name()) && (object != ignore)){
                            if(object->checkRender(c)){
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
void Detail::RenderManagement::_passCopyDepth(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    ShaderP* p = Resources::getShaderProgram("Copy_Depth"); p->bind();

    bindTexture("gDepthMap",gbuffer->getTexture(GBufferType::Depth),0);

    renderFullscreenQuad(fbufferWidth,fbufferHeight);

    unbindTexture2D(0);
    p->unbind();
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
}
void Detail::RenderManagement::_passLighting(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,bool mainRenderFunc){
    ShaderP* pNormal = Resources::getShaderProgram("Deferred_Light"); pNormal->bind();
    ShaderP* pGI = Resources::getShaderProgram("Deferred_Light_GI");
    ShaderP* pSpot = Resources::getShaderProgram("Deferred_Light_Spot");
    ShaderP* p = pNormal;

    glm::mat4 invVP = c->getViewProjInverted();
    glm::mat4 invP = glm::inverse(c->getProjection());
    glm::vec3 campos = c->getPosition();
    float cNear = c->getNear(); float cFar = c->getFar();
    
    sendUniformMatrix4fSafe("VP",c->getViewProjection());
    sendUniformMatrix4fSafe("invVP",invVP);
    sendUniformMatrix4fSafe("invP",invP);
    sendUniform4fSafe("CamPosGamma",campos.x, campos.y, campos.z,RendererInfo::GeneralInfo::gamma);

    sendUniform4fvSafe("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());

    sendUniform4fSafe("ScreenData",cNear,cFar,(float)fbufferWidth,(float)fbufferHeight);

    bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0);
    bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),1);
    bindTextureSafe("gMiscMap",gbuffer->getTexture(GBufferType::Misc),2);
    bindTextureSafe("gDepthMap",gbuffer->getTexture(GBufferType::Depth),3);

    for (auto light:Resources::getCurrentScene()->lights()){
        light.second->lighten();
    }
    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
    p->unbind();

    //do GI here.
    p = pGI; p->bind();
    sendUniformMatrix4fSafe("invVP",invVP);
    sendUniformMatrix4fSafe("invP",invP);
    sendUniform4fvSafe("materials[0]",Material::m_MaterialProperities,Material::m_MaterialProperities.size());
    sendUniform4fSafe("CamPosGamma",campos.x, campos.y, campos.z,RendererInfo::GeneralInfo::gamma);
    sendUniform4fSafe("ScreenData",cNear,cFar,float(fbufferWidth),float(fbufferHeight));
    bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0);
    bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),1);
    bindTextureSafe("gDepthMap",gbuffer->getTexture(GBufferType::Depth),2);
    sendUniform1fSafe("gamma",RendererInfo::GeneralInfo::gamma);

    Scene* s = Resources::getCurrentScene();
    SkyboxEmpty* sky = s->getSkybox();
    if(mainRenderFunc){
        //if(s->m_LightProbes.size() > 0){
            //LightProbe* p = s->m_LightProbes.at("CapsuleLightProbe");
            //bindTextureSafe("irradianceMap",p->getIrriadianceMap(),3,GL_TEXTURE_CUBE_MAP);
            //bindTextureSafe("prefilterMap",p->getPrefilterMap(),4,GL_TEXTURE_CUBE_MAP);
            //bindTextureSafe("brdfLUT",Resources::getTexture("BRDFCookTorrance"),5);
        //}
        if(sky != nullptr && sky->texture()->numAddresses() >= 3){
            bindTextureSafe("irradianceMap",sky->texture()->address(1),3,GL_TEXTURE_CUBE_MAP);
            bindTextureSafe("prefilterMap",sky->texture()->address(2),4,GL_TEXTURE_CUBE_MAP);
            bindTextureSafe("brdfLUT",Resources::getTexture("BRDFCookTorrance"),5);
        }
    }
    Renderer::Detail::renderFullscreenQuad(fbufferWidth,fbufferHeight);
    for(uint i = 0; i < 3; i++){ unbindTexture2D(i); }
    unbindTextureCubemap(3);
    unbindTextureCubemap(4);
    unbindTexture2D(5);
    p->unbind();
}
void Detail::RenderManagement::render(GBuffer* gbuffer,Camera* c,uint fboWidth,uint fboHeight,bool doSSAO, bool doGodRays, bool doAA,bool HUD, Object* ignore,bool mainRenderFunc,GLuint fbo, GLuint rbo){
    if(mainRenderFunc){
        if(Resources::getCurrentScene()->m_LightProbes.size() > 0){
            for(auto lightProbe:Resources::getCurrentScene()->m_LightProbes){
                lightProbe.second->renderCubemap();
            }
            Renderer::Detail::RenderManagement::m_gBuffer->resize(fboWidth,fboHeight);
        }
    }

    if(doSSAO == false) Renderer::Settings::SSAO::disable();
    if(doGodRays == false) Renderer::Settings::GodRays::disable();
    if(doAA == false) Renderer::Detail::RendererInfo::GeneralInfo::aa_algorithm = AntiAliasingAlgorithm::None;

    _passGeometry(gbuffer,c,fboWidth,fboHeight,ignore);

    if(Renderer::Detail::RendererInfo::GodRaysInfo::godRays){
        gbuffer->start(GBufferType::GodRays,"RGBA",false);
        Object* o = Resources::getObject("Sun");
        glm::vec3 sp = Math::getScreenCoordinates(o->getPosition(),false);
        glm::vec3 camPos = c->getPosition();
        glm::vec3 oPos = o->getPosition();
        glm::vec3 camVec = c->getViewVector();
        bool behind = Math::isPointWithinCone(camPos,-camVec,oPos,Math::toRadians(RendererInfo::GodRaysInfo::godRays_fovDegrees));
        float alpha = Math::getAngleBetweenTwoVectors(camVec,camPos - oPos,true) / RendererInfo::GodRaysInfo::godRays_fovDegrees;

        alpha = glm::pow(alpha,RendererInfo::GodRaysInfo::godRays_alphaFalloff);
        alpha = glm::clamp(alpha,0.0001f,0.9999f);

        _passGodsRays(gbuffer,c,fboWidth,fboHeight,glm::vec2(sp.x,sp.y),!behind,1.0f-alpha);
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    if(RendererInfo::LightingInfo::lighting == true && Resources::getCurrentScene()->lights().size() > 0){
        gbuffer->start(GBufferType::Lighting,"RGB");
        Renderer::Settings::clear(true,false,false);//this is needed for godrays
        _passLighting(gbuffer,c,fboWidth,fboHeight,mainRenderFunc);
    }
    glDisable(GL_BLEND);


    //_passForwardRendering(c,fboWidth,fbufferHeight,ignore);

    string _channels;
    bool isdoingssao = false;
    if(doSSAO && RendererInfo::SSAOInfo::ssao){ isdoingssao = true; _channels = "RGBA"; }
    else{                                       _channels = "RGB";  }

    gbuffer->start(GBufferType::Bloom,_channels,false);
    _passSSAO(gbuffer,c,fboWidth,fboHeight); //ssao AND bloom

    if(RendererInfo::SSAOInfo::ssao_do_blur || RendererInfo::BloomInfo::bloom){
        gbuffer->start(GBufferType::Free2,_channels,false);
        _passBlur(gbuffer,c,fboWidth,fboHeight,"H",GBufferType::Bloom,_channels);
        gbuffer->start(GBufferType::Bloom,_channels,false);
        _passBlur(gbuffer,c,fboWidth,fboHeight,"V",GBufferType::Free2,_channels);
    }

    gbuffer->start(GBufferType::Misc);
    _passHDR(gbuffer,c,fboWidth,fboHeight);


    bool doingaa = false;
    if(doAA && RendererInfo::GeneralInfo::aa_algorithm != AntiAliasingAlgorithm::None) doingaa = true;

    if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::None || !doingaa){
        gbuffer->stop(fbo,rbo);
        _passFinal(gbuffer,c,fboWidth,fboHeight);
    }
    else if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::FXAA && doingaa){
        gbuffer->start(GBufferType::Lighting);
        _passFinal(gbuffer,c,fboWidth,fboHeight);

        //_passEdgeCanny(gbuffer,c,fboWidth,fboHeight,GBufferType::Lighting);

        gbuffer->stop(fbo,rbo);
        _passFXAA(gbuffer,c,fboWidth,fboHeight,doingaa);
    }
    else if(RendererInfo::GeneralInfo::aa_algorithm == AntiAliasingAlgorithm::SMAA && doingaa){
        gbuffer->start(GBufferType::Lighting);
        _passFinal(gbuffer,c,fboWidth,fboHeight);
        _passSMAA(gbuffer,c,fboWidth,fboHeight,doingaa);
    }
	if(mainRenderFunc){

		//gbuffer->stop();
		//glDepthFunc(GL_ALWAYS);
		//Settings::enableDepthMask(true);
		//Settings::enableDepthTest(true); //has to be enabled for some reason
		_passCopyDepth(gbuffer,c,fboWidth,fboHeight);

		glEnable(GL_BLEND);
		Settings::disableDepthTest();
		Settings::disableDepthMask();
		if(Detail::RendererInfo::GeneralInfo::draw_physics_debug && c == Resources::getActiveCamera()){
			Physics::Detail::PhysicsManagement::render();
		}

	    //to try and see what the lightprobe is outputting
		/*
		Renderer::unbindFBO();
		Settings::clear();
		LightProbe* pr  = static_cast<LightProbe*>(Resources::getCamera("MainLightProbe"));
		Skybox* sky = static_cast<Skybox*>(Resources::getCurrentScene()->getSkybox());
		if(pr != nullptr){
			ShaderP* p = Resources::getShaderProgram("Deferred_Skybox"); p->bind();
			glm::mat4 view = glm::mat4(glm::mat3(c->getView()));
			Renderer::sendUniformMatrix4f("VP",c->getProjection() * view);
			Renderer::bindTexture("Texture",pr->getEnvMap(),0,GL_TEXTURE_CUBE_MAP);
			Skybox::bindMesh();
			Renderer::unbindTextureCubemap(0);
			p->unbind();
		}
		*/

		//render HUD
		if(HUD == true){
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			Settings::enableDepthTest();
			Settings::enableDepthMask();
			Settings::clear(false,true,false); //clear depth only
			Settings::enableAlphaTest();
			glAlphaFunc(GL_GREATER, 0.1f);
			_renderTextures(gbuffer,c,fboWidth,fboHeight);
			_renderText(gbuffer,c,fboWidth,fboHeight);
			Settings::disableAlphaTest();
		}
		vector_clear(m_FontsToBeRendered);
		vector_clear(m_TexturesToBeRendered);
	}
}
void Detail::RenderManagement::_passSSAO(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight){
    ShaderP* p = Resources::getShaderProgram("Deferred_SSAO"); p->bind();

    sendUniform1iSafe("doSSAO",int(RendererInfo::SSAOInfo::ssao));
    sendUniform1iSafe("doBloom",int(RendererInfo::BloomInfo::bloom));

    glm::vec3 camPos = c->getPosition();
    sendUniformMatrix4fSafe("View",c->getView());
    sendUniformMatrix4fSafe("Projection",c->getProjection());
    sendUniformMatrix4fSafe("invVP",c->getViewProjInverted());
    sendUniformMatrix4fSafe("invP",glm::inverse(c->getProjection()));
    sendUniform1fSafe("nearz",c->getNear());
    sendUniform1fSafe("farz",c->getFar());

    sendUniform3fSafe("CameraPosition",camPos.x,camPos.y,camPos.z);
    
    sendUniform4fSafe("SSAOInfo",RendererInfo::SSAOInfo::ssao_radius,RendererInfo::SSAOInfo::ssao_intensity,
        RendererInfo::SSAOInfo::ssao_bias,RendererInfo::SSAOInfo::ssao_scale);
    
    sendUniform1iSafe("Samples",RendererInfo::SSAOInfo::ssao_samples);
    sendUniform1iSafe("NoiseTextureSize",RendererInfo::SSAOInfo::SSAO_NORMALMAP_SIZE);
    
    float _divisor = gbuffer->getBuffer(GBufferType::Bloom)->divisor();
    sendUniform1fSafe("fbufferDivisor",_divisor);
    
    sendUniform3fvSafe("poisson[0]",RendererInfo::SSAOInfo::ssao_Kernels,RendererInfo::SSAOInfo::SSAO_KERNEL_COUNT);

    bindTexture("gNormalMap",gbuffer->getTexture(GBufferType::Normal),0);
    bindTexture("gRandomMap",RendererInfo::SSAOInfo::ssao_noise_texture,1,GL_TEXTURE_2D);
    bindTexture("gMiscMap",gbuffer->getTexture(GBufferType::Misc),2);
    bindTexture("gLightMap",gbuffer->getTexture(GBufferType::Lighting),3);
    bindTexture("gDepthMap",gbuffer->getTexture(GBufferType::Depth),4);

    renderFullscreenQuad(fboWidth,fboHeight);

    for(uint i = 0; i < 5; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::_passEdgeCanny(GBuffer* gbuffer,Camera* c,uint& fboWidth,uint& fboHeight,GLuint texture){
    
    //texture is the lighting buffer which is the final pass results
    
    gbuffer->start(GBufferType::Misc);
    ShaderP* p = Resources::getShaderProgram("Greyscale_Frag"); p->bind();
    bindTexture("texture",gbuffer->getTexture(texture),0);
    renderFullscreenQuad(fboWidth,fboHeight);
    unbindTexture2D(0);
    p->unbind();
    
    //misc is now greyscale scene. lighting is still final scene

    gbuffer->start(GBufferType::Diffuse);
    p = Resources::getShaderProgram("Deferred_Edge_Canny_Blur"); p->bind();
    bindTexture("texture",gbuffer->getTexture(GBufferType::Misc),0);
    renderFullscreenQuad(fboWidth,fboHeight);
    unbindTexture2D(0);
    p->unbind();
    /*
    //blur it again
    gbuffer->start(GBufferType::Misc);
    p->bind();
    bindTexture("texture",gbuffer->getTexture(GBufferType::Diffuse),0);
    renderFullscreenQuad(fboWidth,fboHeight);
    unbindTexture2D(0);
    p->unbind();
    //blur it again
    gbuffer->start(GBufferType::Diffuse);
    p->bind();
    bindTexture("texture",gbuffer->getTexture(GBufferType::Misc),0);
    renderFullscreenQuad(fboWidth,fboHeight);
    unbindTexture2D(0);
    p->unbind();
    */

    //misc is now the final blurred greyscale image
    gbuffer->start(GBufferType::Misc);
    p = Resources::getShaderProgram("Deferred_Edge_Canny"); p->bind();
    bindTexture("texture",gbuffer->getTexture(GBufferType::Diffuse),0);
    renderFullscreenQuad(fboWidth,fboHeight);
    unbindTexture2D(0);
    p->unbind();
    //diffuse is the end result of the edge program. lighting is the final pass that we still need
}
void Detail::RenderManagement::_passGodsRays(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,glm::vec2 lightScrnPos,bool behind,float alpha){
    Settings::clear(true,false,false);
    ShaderP* p = Resources::getShaderProgram("Deferred_GodsRays"); p->bind();

    sendUniform4f("RaysInfo",RendererInfo::GodRaysInfo::godRays_exposure,
        RendererInfo::GodRaysInfo::godRays_decay,RendererInfo::GodRaysInfo::godRays_density,
        RendererInfo::GodRaysInfo::godRays_weight);

    sendUniform2f("lightPositionOnScreen",lightScrnPos.x/float(fbufferWidth),lightScrnPos.y/float(fbufferHeight));

    sendUniform1i("samples",RendererInfo::GodRaysInfo::godRays_samples);
    sendUniform1i("behind",int(behind));
    sendUniform1f("alpha",alpha);
    
    float _divisor = gbuffer->getBuffer(GBufferType::GodRays)->divisor();
    sendUniform1f("fbufferDivisor",_divisor);

    bindTexture("firstPass",gbuffer->getTexture(GBufferType::Lighting),0);

    renderFullscreenQuad(fbufferWidth,fbufferHeight);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passHDR(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight){
    ShaderP* p = Resources::getShaderProgram("Deferred_HDR"); p->bind();

    sendUniform4f("HDRInfo",RendererInfo::HDRInfo::hdr_exposure,float(int(RendererInfo::HDRInfo::hdr)),
        float(int(RendererInfo::BloomInfo::bloom)),float(int(RendererInfo::HDRInfo::hdr_algorithm)));

    sendUniform1iSafe("HasLighting",int(RendererInfo::LightingInfo::lighting));

    bindTextureSafe("lightingBuffer",gbuffer->getTexture(GBufferType::Lighting),0);
    bindTextureSafe("bloomBuffer",gbuffer->getTexture(GBufferType::Bloom),1);
    bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),2);
    bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),3);
    renderFullscreenQuad(fbufferWidth,fbufferHeight);

    for(uint i = 0; i < 4; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::_passBlur(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,string type, GLuint texture,string channels){
    ShaderP* p = Resources::getShaderProgram("Deferred_Blur"); p->bind();

    sendUniform1f("radius",RendererInfo::BloomInfo::bloom_radius);
    sendUniform4f("strengthModifier",RendererInfo::BloomInfo::bloom_strength,RendererInfo::BloomInfo::bloom_strength,RendererInfo::BloomInfo::bloom_strength,RendererInfo::SSAOInfo::ssao_blur_strength);

    float _divisor = gbuffer->getBuffer(GBufferType::Bloom)->divisor();
    sendUniform1f("fbufferDivisor",_divisor);

    glm::vec4 rgba(0.0f);
    if(channels.find("R") != string::npos) rgba.x = 1.0f;
    if(channels.find("G") != string::npos) rgba.y = 1.0f;
    if(channels.find("B") != string::npos) rgba.z = 1.0f;
    if(channels.find("A") != string::npos) rgba.w = 1.0f;

    sendUniform4f("RGBA",rgba.x,rgba.y,rgba.z,rgba.w);

    if(type == "H"){ sendUniform2f("HV",1.0f,0.0f); }
    else{            sendUniform2f("HV",0.0f,1.0f); }

    bindTexture("texture",gbuffer->getTexture(texture),0);

    renderFullscreenQuad(fbufferWidth,fbufferHeight);

    unbindTexture2D(0);
    p->unbind();
}
void Detail::RenderManagement::_passFXAA(GBuffer* gbuffer,Camera* c,uint& fbufferWidth, uint& fbufferHeight,bool renderAA){
    if(!renderAA) return;

    ShaderP* p = Resources::getShaderProgram("Deferred_FXAA"); p->bind();

    sendUniform1f("FXAA_REDUCE_MIN",RendererInfo::FXAAInfo::FXAA_REDUCE_MIN);
    sendUniform1f("FXAA_REDUCE_MUL",RendererInfo::FXAAInfo::FXAA_REDUCE_MUL);
    sendUniform1f("FXAA_SPAN_MAX",RendererInfo::FXAAInfo::FXAA_SPAN_MAX);

    sendUniform2f("resolution",float(fbufferWidth),float(fbufferHeight));
    bindTexture("sampler0",gbuffer->getTexture(GBufferType::Lighting),0);
    bindTextureSafe("edgeTexture",gbuffer->getTexture(GBufferType::Misc),1);
    bindTexture("depthTexture",gbuffer->getTexture(GBufferType::Depth),2);
    renderFullscreenQuad(fbufferWidth,fbufferHeight);

    for(uint i = 0; i < 3; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::RenderManagement::_passSMAA(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight,bool renderAA){
    if(!renderAA) return;

    glm::vec4 SMAA_PIXEL_SIZE = glm::vec4(float(1.0f / float(fboWidth)), float(1.0f / float(fboHeight)), float(fboWidth), float(fboHeight));

    gbuffer->start(GBufferType::Misc); //we save the original image to Diffuse buffer so it can be used later
    Settings::clear(true,false,false);
    ShaderP* p = Resources::getShaderProgram("Deferred_SMAA_1"); p->bind();
    sendUniform4fSafe("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
    sendUniform1fSafe("SMAA_THRESHOLD",RendererInfo::SMAAInfo::SMAA_THRESHOLD);
    sendUniform1fSafe("SMAA_DEPTH_THRESHOLD",RendererInfo::SMAAInfo::SMAA_DEPTH_THRESHOLD);
    sendUniform1fSafe("SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR",RendererInfo::SMAAInfo::SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR);
    sendUniform1fSafe("SMAA_PREDICATION_THRESHOLD",RendererInfo::SMAAInfo::SMAA_PREDICATION_THRESHOLD);
    sendUniform1fSafe("SMAA_PREDICATION_SCALE",RendererInfo::SMAAInfo::SMAA_PREDICATION_SCALE);
    sendUniform1fSafe("SMAA_PREDICATION_STRENGTH",RendererInfo::SMAAInfo::SMAA_PREDICATION_STRENGTH);
    bindTextureSafe("texture",gbuffer->getTexture(GBufferType::Lighting),0);

    //edge pass
    renderFullscreenQuad(fboWidth,fboHeight);

    for(uint i = 0; i < 1; i++){ unbindTexture2D(i); }
    p->unbind();


    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    gbuffer->start(GBufferType::Normal);
    Settings::clear(true,false,false);
    //gbuffer->stop();
    p = Resources::getShaderProgram("Deferred_SMAA_2"); p->bind();
    sendUniform4fSafe("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
    sendUniform1iSafe("SMAA_MAX_SEARCH_STEPS",RendererInfo::SMAAInfo::SMAA_MAX_SEARCH_STEPS);

    bindTextureSafe("edge_tex",gbuffer->getTexture(GBufferType::Misc),0);
    bindTextureSafe("area_tex",RendererInfo::SMAAInfo::SMAA_AreaTexture,1,GL_TEXTURE_2D);
    bindTextureSafe("search_tex",RendererInfo::SMAAInfo::SMAA_SearchTexture,2,GL_TEXTURE_2D);

    sendUniform1iSafe("SMAA_MAX_SEARCH_STEPS_DIAG",RendererInfo::SMAAInfo::SMAA_MAX_SEARCH_STEPS_DIAG);
    sendUniform1iSafe("SMAA_AREATEX_MAX_DISTANCE",RendererInfo::SMAAInfo::SMAA_AREATEX_MAX_DISTANCE);
    sendUniform1iSafe("SMAA_AREATEX_MAX_DISTANCE_DIAG",RendererInfo::SMAAInfo::SMAA_AREATEX_MAX_DISTANCE_DIAG);
    sendUniform2fSafe("SMAA_AREATEX_PIXEL_SIZE",RendererInfo::SMAAInfo::SMAA_AREATEX_PIXEL_SIZE);
    sendUniform1fSafe("SMAA_AREATEX_SUBTEX_SIZE",RendererInfo::SMAAInfo::SMAA_AREATEX_SUBTEX_SIZE);
    sendUniform1iSafe("SMAA_CORNER_ROUNDING",RendererInfo::SMAAInfo::SMAA_CORNER_ROUNDING);
    sendUniform1fSafe("SMAA_CORNER_ROUNDING_NORM",(float(RendererInfo::SMAAInfo::SMAA_CORNER_ROUNDING) / 100.0f));

    //blend pass
    renderFullscreenQuad(fboWidth,fboHeight);

    for(uint i = 0; i < 3; i++){ unbindTexture2D(i); }
    p->unbind();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //gbuffer->start(GBufferType::Misc);
    gbuffer->stop();
    p = Resources::getShaderProgram("Deferred_SMAA_3"); p->bind();
    sendUniform4fSafe("SMAA_PIXEL_SIZE",SMAA_PIXEL_SIZE);
    bindTextureSafe("texture",gbuffer->getTexture(GBufferType::Lighting),0); //need original final image from first smaa pass
    bindTextureSafe("blend_tex",gbuffer->getTexture(GBufferType::Normal),1);

    //neighbor pass
    renderFullscreenQuad(fboWidth,fboHeight);

    for(uint i = 0; i < 2; i++){ unbindTexture2D(i); }
    p->unbind();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////


    /*
    //this pass is optional. lets skip it for now
    //gbuffer->start(GBufferType::Lighting);
    gbuffer->stop();
    p = Resources::getShaderProgram("Deferred_SMAA_4"); p->bind();
    renderFullscreenQuad(fboWidth,fboHeight);
    p->unbind();
    */
    
}
void Detail::RenderManagement::_passFinal(GBuffer* gbuffer,Camera* c,uint& fboWidth, uint& fboHeight){
    ShaderP* p = Resources::getShaderProgram("Deferred_Final"); p->bind();

    sendUniform1iSafe("HasSSAO",int(Detail::RendererInfo::SSAOInfo::ssao));
    sendUniform1iSafe("HasRays",int(Detail::RendererInfo::GodRaysInfo::godRays));

    bindTextureSafe("gDiffuseMap",gbuffer->getTexture(GBufferType::Diffuse),0);
    bindTextureSafe("gLightMap",gbuffer->getTexture(GBufferType::Lighting),1);
    bindTextureSafe("gMiscMap",gbuffer->getTexture(GBufferType::Misc),2);
    bindTextureSafe("gGodsRaysMap",gbuffer->getTexture(GBufferType::GodRays),3);
    bindTextureSafe("gBloomMap",gbuffer->getTexture(GBufferType::Bloom),4);
    bindTextureSafe("gNormalMap",gbuffer->getTexture(GBufferType::Normal),5);

    renderFullscreenQuad(fboWidth,fboHeight);

    for(uint i = 0; i < 6; i++){ unbindTexture2D(i); }
    p->unbind();
}
void Detail::renderFullscreenQuad(uint width,uint height){
    float w2 = float(width)/2.0f;
    float h2 = float(height)/2.0f;
    glm::mat4 m(1.0f);
    glm::mat4 p = glm::ortho(-w2,w2,-h2,h2);
    sendUniformMatrix4f("Model",m);
    sendUniformMatrix4f("VP",p);
    setViewport(0,0,width,height);
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);  glVertex2f(-w2, -h2);
        glTexCoord2f(1.0f, 0.0f);  glVertex2f( w2, -h2);
        glTexCoord2f(1.0f, 1.0f);  glVertex2f( w2,  h2);
        glTexCoord2f(0.0f, 1.0f);  glVertex2f(-w2,  h2);
    glEnd();
}
