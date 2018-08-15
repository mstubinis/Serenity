#include <boost/make_shared.hpp>
#include "Engine.h"
#include "Engine_ThreadManager.h"
#include "Engine_Time.h"
#include "Engine_Resources.h"
#include "Engine_ObjectPool.h"
#include "Engine_BuiltInResources.h"
#include "Engine_Sounds.h"
#include "Engine_Window.h"
#include "Light.h"
#include "Skybox.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Material.h"
#include "Camera.h"
#include "Texture.h"
#include "Font.h"
#include "Scene.h"

#include <SFML/Graphics.hpp>

using namespace Engine;
using namespace std;

template<class V,class S>S _incrementName(unordered_map<S,V>& m,const S n){S r=n;if(m.size()>0){uint c=0;while(m.count(r)){r=n+" "+boost::lexical_cast<S>(c);c++;}}return r;}
template<class V,class S>S _incrementName(map<S,V>& m,const S n){S r=n;if(m.size()>0){uint c=0;while(m.count(r)){r=n+" "+boost::lexical_cast<S>(c);c++;}}return r;}
template<class V, class O,class S>void _addToContainer(map<S,V>& m,const S& n,O& o){if(m.size()>0&&m.count(n)){o.reset();return;}m.emplace(n,o);}
template<class V, class O,class S>void _addToContainer(unordered_map<S,V>& m,const S& n,O& o){if(m.size()>0&&m.count(n)){o.reset();return;}m.emplace(n,o);}
template<class V,class S>void* _getFromContainer(map<S,V>& m,const S& n){if(!m.count(n))return nullptr;return m.at(n).get();}
template<class V,class S>void* _getFromContainer(unordered_map<S,V>& m,const S& n){if(!m.count(n))return nullptr;return m.at(n).get();}
template<class V,class S>void _removeFromContainer(map<S,V>& m,const S& n){if(m.size()>0&&m.count(n)){m.at(n).reset();m.erase(n);}}
template<class V,class S>void _removeFromContainer(unordered_map<S,V>& m,const S& n){if(m.size()>0&&m.count(n)){m.at(n).reset();m.erase(n);}}

epriv::ResourceManager* resourceManager;

class epriv::ResourceManager::impl final{
    public:
        //http://gamesfromwithin.com/managing-data-relationships
        ObjectPool<EngineResource>*                    m_Resources;
        Engine_Window*                                 m_Window;
        Scene*                                         m_CurrentScene;
        bool                                           m_DynamicMemory;
        vector<MeshInstance*>                          m_MeshInstances;
        unordered_map<string,boost::shared_ptr<Scene>> m_Scenes;
        void _init(const char* name,const uint& width,const uint& height){
            m_CurrentScene = nullptr;
            m_DynamicMemory = false;
            m_Resources = new ObjectPool<EngineResource>(32768);
        }
        void _postInit(const char* name,uint width,uint height){
            m_Window = new Engine_Window(name,width,height);
        }
        void _destruct(){
            for (auto it:m_MeshInstances)                                 SAFE_DELETE(it);
            for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )  it->second.reset();
            SAFE_DELETE(m_Resources);
            SAFE_DELETE(m_Window);
        }
};
epriv::ResourceManager::ResourceManager(const char* name,uint width,uint height):m_i(new impl){
    m_i->_init(name,width,height);
	resourceManager = this;
}
epriv::ResourceManager::~ResourceManager(){
    m_i->_destruct();
}
void epriv::ResourceManager::_init(const char* n,uint w,uint h){
    m_i->_postInit(n,w,h);
}

string Engine::Data::reportTime(){
    return epriv::Core::m_Engine->m_TimeManager->reportTime();
}
float& Engine::Resources::dt(){ return epriv::Core::m_Engine->m_TimeManager->dt(); }
Scene* Engine::Resources::getCurrentScene(){ return resourceManager->m_i->m_CurrentScene; }

bool epriv::ResourceManager::_hasScene(string n){ if(resourceManager->m_i->m_Scenes.count(n)) return true; return false; }
Texture* epriv::ResourceManager::_hasTexture(string n){
	for(uint i = 0; i < resourceManager->m_i->m_Resources->maxEntries(); ++i){
		EngineResource* r = resourceManager->m_i->m_Resources->getAsFast<EngineResource>(i);
		if(r){ Texture* t = dynamic_cast<Texture*>(r); if(t && t->name() == n){ return t; } }
	}
	return 0;
}
void epriv::ResourceManager::_addScene(Scene* s){
    _addToContainer(resourceManager->m_i->m_Scenes,s->name(),boost::shared_ptr<Scene>(s));
}
void epriv::ResourceManager::_addMeshInstance(MeshInstance* m){
    m_i->m_MeshInstances.push_back(m);
}
string epriv::ResourceManager::_buildSceneName(string n){return _incrementName(resourceManager->m_i->m_Scenes,n);}

uint epriv::ResourceManager::_numScenes(){return resourceManager->m_i->m_Scenes.size();}

void Resources::Settings::enableDynamicMemory(bool b){ resourceManager->m_i->m_DynamicMemory = b; }
void Resources::Settings::disableDynamicMemory(){ resourceManager->m_i->m_DynamicMemory = false; }

Engine_Window* Resources::getWindow(){ return resourceManager->m_i->m_Window; }
glm::uvec2 Resources::getWindowSize(){ return resourceManager->m_i->m_Window->getSize(); }

Scene* Resources::getScene(string n){return (Scene*)(_getFromContainer(resourceManager->m_i->m_Scenes,n));}

void Resources::getShader(Handle& h,Shader*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Shader* Resources::getShader(Handle& h){ Shader* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getSoundData(Handle& h,SoundData*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
SoundData* Resources::getSoundData(Handle& h){ SoundData* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getCamera(Handle& h,Camera*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Camera* Resources::getCamera(Handle& h){ Camera* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getFont(Handle& h,Font*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Font* Resources::getFont(Handle& h){ Font* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getTexture(Handle& h,Texture*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Texture* Resources::getTexture(Handle& h){ Texture* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getMesh(Handle& h,Mesh*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Mesh* Resources::getMesh(Handle& h){ Mesh* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getMaterial(Handle& h,Material*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
Material* Resources::getMaterial(Handle& h){ Material* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }
void Resources::getShaderProgram(Handle& h,ShaderP*& p){ resourceManager->m_i->m_Resources->getAs(h,p); }
ShaderP* Resources::getShaderProgram(Handle& h){ ShaderP* p; resourceManager->m_i->m_Resources->getAs(h,p); return p; }


Handle Resources::addFont(string filename){
    return resourceManager->m_i->m_Resources->add(new Font(filename),ResourceType::Font);
}
Handle Resources::addMesh(string f, CollisionType::Type t, bool b,float threshhold){
    return resourceManager->m_i->m_Resources->add(new Mesh(f,t,b,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n,float x,float y,float w,float h,float threshhold){
    return resourceManager->m_i->m_Resources->add(new Mesh(n,x,y,w,h,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n,float w,float h,float threshhold){
    return resourceManager->m_i->m_Resources->add(new Mesh(n,w,h,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n, unordered_map<string,float>& g, uint w, uint l,float threshhold){
    return resourceManager->m_i->m_Resources->add(new Mesh(n,g,w,l,threshhold),ResourceType::Mesh);
}
Handle Resources::addMeshAsync(string f, CollisionType::Type t, bool b,float threshhold){
	Mesh* mesh = new Mesh(f,t,b,threshhold,false);

	boost::function<void()> job = boost::bind(&InternalMeshPublicInterface::LoadCPU, mesh);
	boost::function<void()> cbk = boost::bind(&InternalMeshPublicInterface::LoadGPU, mesh);

	Engine::epriv::threading::addJobWithPostCallback(job,cbk);

    return resourceManager->m_i->m_Resources->add(mesh, ResourceType::Mesh);
}


Handle epriv::ResourceManager::_addTexture(Texture* t){
    return resourceManager->m_i->m_Resources->add(t,ResourceType::Texture);
}

Handle Resources::addTexture(string file,ImageInternalFormat::Format internFormat,bool mipmaps){
	return resourceManager->m_i->m_Resources->add(new Texture(file,mipmaps,internFormat),ResourceType::Texture);
}

Handle Resources::addMaterial(string name, string diffuse, string normal,string glow, string specular,Handle programHandle){
    ShaderP* program = nullptr;
    if(programHandle.null()){
        program = epriv::InternalShaderPrograms::Deferred;
    }
    else{
        program = Resources::getShaderProgram(programHandle);
    }
    Material* material = new Material(name,diffuse,normal,glow,specular);
    program->addMaterial(material);
    return resourceManager->m_i->m_Resources->add(material,ResourceType::Material);
}
Handle Resources::addMaterial(string name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular,ShaderP* program){
    if(!program) program = epriv::InternalShaderPrograms::Deferred;
    Material* material = new Material(name,diffuse,normal,glow,specular);
    program->addMaterial(material);
    return resourceManager->m_i->m_Resources->add(material,ResourceType::Material);
}

Handle Resources::addShader(string fileOrData, ShaderType::Type type, bool fromFile){
    return resourceManager->m_i->m_Resources->add(new Shader(fileOrData,type,fromFile),ResourceType::Shader);
}

Handle Resources::addShaderProgram(string n, Shader* v, Shader* f, ShaderRenderPass::Pass s){
    return resourceManager->m_i->m_Resources->add(new ShaderP(n,v,f,s),ResourceType::ShaderProgram);
}
Handle Resources::addShaderProgram(string n, Handle& v, Handle& f, ShaderRenderPass::Pass s){
    Shader* vS = nullptr; resourceManager->m_i->m_Resources->getAs(v,vS);
    Shader* fS = nullptr; resourceManager->m_i->m_Resources->getAs(f,fS);
    return resourceManager->m_i->m_Resources->add(new ShaderP(n,vS,fS,s),ResourceType::ShaderProgram);
}

Handle Resources::addSoundData(string file,string n,bool music){
    return resourceManager->m_i->m_Resources->add(new SoundData(file,music),ResourceType::SoundData);
}

void Resources::setCurrentScene(Scene* scene){
    if(!resourceManager->m_i->m_CurrentScene){
        epriv::Core::m_Engine->m_ComponentManager->_sceneSwap(nullptr,scene);
        resourceManager->m_i->m_CurrentScene = scene;
        return;
    }
    if(resourceManager->m_i->m_CurrentScene != scene){
        cout << "---- Scene Change started (" << resourceManager->m_i->m_CurrentScene->name() << ") to (" << scene->name() << ") ----" << endl;
        if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory){
            //mark game object resources to minus use count
        }
        epriv::Core::m_Engine->m_ComponentManager->_sceneSwap(resourceManager->m_i->m_CurrentScene,scene);
        resourceManager->m_i->m_CurrentScene = scene;
        if(resourceManager->m_i->m_DynamicMemory){
            //mark game object resources to add use count
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(string s){Resources::setCurrentScene((Scene*)(_getFromContainer(resourceManager->m_i->m_Scenes,s)));}
