#include "GBuffer.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Renderer.h"

using namespace Engine;

class TextureBuffer::impl final{
    public:
        int m_BufferInternalFormat; int m_BufferFormat; int m_BufferType; int m_BufferAttatchment;
        float m_SizeScalar;
        void _init(int internalformat, int format, int type, int attatchment,uint width,uint height,float divisor,uint multisample,TextureBuffer* super){
            m_BufferInternalFormat = internalformat;
            m_BufferFormat = format;
            m_BufferType = type;
            m_BufferAttatchment = attatchment;
            m_SizeScalar = divisor;

            _resize(width,height,multisample,super);
        }
        void _resize(uint width,uint height,uint multisample,TextureBuffer* super){
			super->_constructAsFramebuffer(width,height,m_SizeScalar,m_BufferInternalFormat,m_BufferFormat,m_BufferType,m_BufferAttatchment,multisample);
		}
};
class GBuffer::impl final{
    public:
        GLuint m_fbo;       GLuint m_depth;
		GLuint m_multisampled_fbo; GLuint m_multisampled_depth;
        GLuint m_fbo_bloom; GLuint m_depth_fake;
        std::unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_Buffers;
		std::unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_BuffersMultisampled;
        uint m_Width; uint m_Height;
        void _init(uint w,uint h,uint multisample){
			_destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            glGenFramebuffers(1, &m_fbo);
			Renderer::bindFBO(m_fbo);

			glGenRenderbuffers(1, &m_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

            _constructFramebuffer("BUFFER_DIFFUSE",BUFFER_TYPE_DIFFUSE,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_NORMAL",BUFFER_TYPE_NORMAL,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_MISC",BUFFER_TYPE_MISC,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_POSITION",BUFFER_TYPE_POSITION,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_LIGHTING",BUFFER_TYPE_LIGHTING,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_FREE1",BUFFER_TYPE_FREE1,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_DEPTH",BUFFER_TYPE_DEPTH,m_Width,m_Height,0);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				std::cout << "Main render buffer construction failed!" << std::endl;
			}
			GLint p;
			GLint p1;
			glGetFramebufferParameterivEXT(GL_FRAMEBUFFER,GL_SAMPLES,&p);
			glGetFramebufferParameterivEXT(GL_FRAMEBUFFER,GL_SAMPLE_BUFFERS,&p1);

			std::cout << "Main Framebuffer GL_SAMPLES: " << p << std::endl;
			std::cout << "Main Framebuffer GL_SAMPLE_BUFFERS: " << p1 << std::endl;

			Renderer::bindFBO(0);


			// second framebuffer
			if(multisample != 0){
				glEnable(GL_MULTISAMPLE);

				glGenFramebuffers(1, &m_multisampled_fbo);
				Renderer::bindFBO(m_multisampled_fbo);

				glGenRenderbuffers(1, &m_multisampled_depth);
				glBindRenderbuffer(GL_RENDERBUFFER, m_multisampled_depth);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample, GL_DEPTH_COMPONENT, m_Width, m_Height);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_multisampled_depth);

				_constructFramebuffer("BUFFER_DIFFUSE_INTERMEDIATE",BUFFER_TYPE_DIFFUSE,m_Width,m_Height,multisample);
				_constructFramebuffer("BUFFER_NORMAL_INTERMEDIATE",BUFFER_TYPE_NORMAL,m_Width,m_Height,multisample);
				_constructFramebuffer("BUFFER_MISC_INTERMEDIATE",BUFFER_TYPE_MISC,m_Width,m_Height,multisample);
				_constructFramebuffer("BUFFER_POSITION_INTERMEDIATE",BUFFER_TYPE_POSITION,m_Width,m_Height,multisample);
				//_constructFramebuffer("BUFFER_LIGHTING_INTERMEDIATE",BUFFER_TYPE_LIGHTING,m_Width,m_Height,multisample);
				_constructFramebuffer("BUFFER_FREE1_INTERMEDIATE",BUFFER_TYPE_FREE1,m_Width,m_Height,multisample);

				if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
					std::cout << "multisampled render buffer construction failed!" << std::endl;
				}

				GLint p;
				GLint p1;
				glGetFramebufferParameterivEXT(GL_FRAMEBUFFER,GL_SAMPLES,&p);
				glGetFramebufferParameterivEXT(GL_FRAMEBUFFER,GL_SAMPLE_BUFFERS,&p1);

				std::cout << "multisampled Framebuffer GL_SAMPLES: " << p << std::endl;
				std::cout << "multisampled Framebuffer GL_SAMPLE_BUFFERS: " << p1 << std::endl;

				Renderer::bindFBO(0);
			}


            glGenFramebuffers(1, &m_fbo_bloom);
			Renderer::bindFBO(m_fbo_bloom);

            glGenRenderbuffers(1, &m_depth_fake);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);

            _constructFramebuffer("BUFFER_BLOOM",BUFFER_TYPE_BLOOM,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_FREE2",BUFFER_TYPE_FREE2,m_Width,m_Height,0);
            _constructFramebuffer("BUFFER_GODSRAYS",BUFFER_TYPE_GODSRAYS,m_Width,m_Height,0);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				std::cout << "smaller render buffer construction failed!" << std::endl;
			}

            Renderer::bindFBO(0);
        }
        void _constructFramebuffer(std::string n,uint t,uint w,uint h,uint multisample){
            TextureBuffer* tbo = nullptr;
			tbo = new TextureBuffer(n,GBUFFER_TYPES[t],GBUFFER_PIXEL_TYPES[t],GBUFFER_FLOAT_TYPES[t],GBUFFER_ATTACHMENT_TYPES[t],w,h,multisample,GBUFFER_DIVISIBLES[t]);
            boost::weak_ptr<TextureBuffer> ptr = boost::dynamic_pointer_cast<TextureBuffer>(Resources::getTexturePtr(tbo->name()));
			if(multisample == 0)
				m_Buffers.emplace(t,ptr);
			else
				m_BuffersMultisampled.emplace(t,ptr);
        }
		void _blitToIntermediates(){
			glEnable(GL_TEXTURE_2D);

			Renderer::bindReadFBO(m_multisampled_fbo);
			glReadBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_DIFFUSE]);
			Renderer::bindDrawFBO(m_fbo);
			glDrawBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_DIFFUSE]);
			glBlitFramebuffer(0, 0,m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		    GLenum blit_error = GL_NO_ERROR;
			blit_error = glGetError();
			if (blit_error != GL_NO_ERROR){
				std::cout << "BlitFramebuffer Diffuse failed with error: "  << gluErrorString(blit_error) << std::endl;
			}
			Renderer::bindFBO(0);

			Renderer::bindReadFBO(m_multisampled_fbo);
			glReadBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_NORMAL]);
			Renderer::bindDrawFBO(m_fbo);
			glDrawBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_NORMAL]);
			glBlitFramebuffer(0, 0,m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
		     blit_error = GL_NO_ERROR;
			blit_error = glGetError();
			if (blit_error != GL_NO_ERROR){
				std::cout << "BlitFramebuffer Normal failed with error: "  << gluErrorString(blit_error) << std::endl;
			}
			Renderer::bindFBO(0);

			Renderer::bindReadFBO(m_multisampled_fbo);
			glReadBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_MISC]);
			Renderer::bindDrawFBO(m_fbo);
			glDrawBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_MISC]);
			glBlitFramebuffer(0, 0,m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
		     blit_error = GL_NO_ERROR;
			blit_error = glGetError();
			if (blit_error != GL_NO_ERROR){
				std::cout << "BlitFramebuffer Misc failed with error: "  << gluErrorString(blit_error) << std::endl;
			}
			Renderer::bindFBO(0);

			Renderer::bindReadFBO(m_multisampled_fbo);
			glReadBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_POSITION]);
			Renderer::bindDrawFBO(m_fbo);
			glDrawBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_POSITION]);
			glBlitFramebuffer(0, 0,m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
		     blit_error = GL_NO_ERROR;
			blit_error = glGetError();
			if (blit_error != GL_NO_ERROR){
				std::cout << "BlitFramebuffer Position failed with error: "  << gluErrorString(blit_error) << std::endl;
			}
			Renderer::bindFBO(0);

			Renderer::bindReadFBO(m_multisampled_fbo);
			glReadBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_FREE1]);
			Renderer::bindDrawFBO(m_fbo);
			glDrawBuffer(GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_FREE1]);
			glBlitFramebuffer(0, 0,m_Width, m_Height, 0, 0, m_Width, m_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
		     blit_error = GL_NO_ERROR;
			blit_error = glGetError();
			if (blit_error != GL_NO_ERROR){
				std::cout << "BlitFramebuffer Free1 failed with error: "  << gluErrorString(blit_error) << std::endl;
			}
			Renderer::bindFBO(0);
		}
        void _destruct(){
            for(auto buffer:m_Buffers){             buffer.second.reset(); }
			for(auto buffer:m_BuffersMultisampled){ buffer.second.reset(); }
			m_Buffers.clear();
			m_BuffersMultisampled.clear();
			m_Width = m_Height = 0;

            Renderer::bindFBO(m_fbo);
            glDeleteRenderbuffers(1, &m_depth);
            glDeleteFramebuffers(1, &m_fbo);
            Renderer::bindFBO(0);

			Renderer::bindFBO(m_multisampled_fbo);
			glDeleteRenderbuffers(1, &m_multisampled_depth);
            glDeleteFramebuffers(1, &m_multisampled_fbo);
            Renderer::bindFBO(0);

			Renderer::bindFBO(m_fbo_bloom);
            glDeleteRenderbuffers(1, &m_depth_fake);
            glDeleteFramebuffers(1, &m_fbo_bloom);
			Renderer::bindFBO(0);
        }
        void _start(std::vector<uint>& types,std::string& channels,bool multisampled,bool first_fbo){
            if(first_fbo){
				if(multisampled){
					Renderer::bindReadFBO(m_multisampled_fbo);
					Renderer::bindDrawFBO(m_multisampled_fbo);
				}
				else{
					Renderer::bindReadFBO(m_fbo);
					Renderer::bindDrawFBO(m_fbo);
				}
			}
			else{
				Renderer::bindReadFBO(m_fbo_bloom);
				Renderer::bindDrawFBO(m_fbo_bloom);
			}
            bool r,g,b,a;
            if(channels.find("R") != std::string::npos) r=true; else r=false;
            if(channels.find("G") != std::string::npos) g=true; else g=false;
            if(channels.find("B") != std::string::npos) b=true; else b=false;
            if(channels.find("A") != std::string::npos) a=true; else a=false;
            glColorMask(r,g,b,a);
            glDrawBuffers(types.size(), &types[0]);        // Specify what to render an start acquiring
            glClear(GL_COLOR_BUFFER_BIT);                  // Clear the render targets
        }
        void _start(uint t1,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _start(uint t1,uint t2,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _start(uint t1,uint t2,uint t3,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,uint t6,std::string& c,bool m,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t6).lock().get()->attatchment());
            _start(t,c,m,f);
        }
        void _stop(){
            //glBindFramebuffer(GL_FRAMEBUFFER, 0);
			Renderer::bindFBO(0);
            glColorMask(1,1,1,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(std::string name,int internalformat, int format, int type, int attatchment,uint width,uint height,uint multisample,float divisor):Texture(name,width,height),m_i(new impl()){
    m_i->_init(internalformat,format,type,attatchment,width,height,divisor,multisample,this);
}
TextureBuffer::~TextureBuffer(){
}
const float TextureBuffer::sizeScalar() const{ return m_i->m_SizeScalar; }
const int TextureBuffer::attatchment() const{ return m_i->m_BufferAttatchment; }
GBuffer::GBuffer(uint width,uint height,uint multisample):m_i(new impl()){	
    m_i->_init(width,height,multisample);
}
GBuffer::~GBuffer(){
    m_i->_destruct();
}
void GBuffer::blitToIntermediates(){
	m_i->_blitToIntermediates();
}
void GBuffer::resize(uint width, uint height){
	stop();
	m_i->_init(width,height,Renderer::Detail::RendererInfo::GeneralInfo::multisample_level);
}
void GBuffer::start(std::vector<uint>& types,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(types,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,uint type1,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,type1,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,type1,type2,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,type1,type2,type3,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,channels,multiSampled,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string channels,bool multiSampled,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,type5,channels,multiSampled,first_fbo);}
void GBuffer::stop(){m_i->_stop();}
const std::unordered_map<uint,boost::weak_ptr<TextureBuffer>>& GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* GBuffer::getTexture(uint type){ return m_i->m_Buffers.at(type).lock().get();}
Texture* GBuffer::getTextureMultisampled(uint type){ return m_i->m_BuffersMultisampled.at(type).lock().get();}

GLuint& GBuffer::getMainFBO(){ return m_i->m_fbo; }
GLuint& GBuffer::getSmallFBO(){ return m_i->m_fbo_bloom; }