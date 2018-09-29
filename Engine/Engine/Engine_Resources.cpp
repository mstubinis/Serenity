#include <boost/make_shared.hpp>
#include "Engine.h"
#include "Engine_ThreadManager.h"
#include "Engine_Time.h"
#include "Engine_Resources.h"
#include "Engine_ObjectPool.h"
#include "Engine_BuiltInResources.h"
#include "Engine_EventDispatcher.h"
#include "Engine_Sounds.h"
#include "Engine_Window.h"
#include "Skybox.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Material.h"
#include "Camera.h"
#include "Texture.h"
#include "Font.h"
#include "Scene.h"

#include <iostream>

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

epriv::ResourceManager::impl* resourceManager;

class epriv::ResourceManager::impl final{
    public:
        //http://gamesfromwithin.com/managing-data-relationships
        ObjectPool<EngineResource>*                    m_Resources;
        Engine_Window*                                 m_Window;
        Scene*                                         m_CurrentScene;
        bool                                           m_DynamicMemory;
        unordered_map<string,boost::shared_ptr<Scene>> m_Scenes;
        void _init(const char* name,const uint& width,const uint& height){
            m_CurrentScene = nullptr;
            m_Window = nullptr;
            m_DynamicMemory = false;
            m_Resources = new ObjectPool<EngineResource>(32768);
        }
        void _postInit(const char* name,uint width,uint height){
            m_Window = new Engine_Window(name,width,height);
        }
        void _destruct(){
            SAFE_DELETE(m_Resources);
            SAFE_DELETE(m_Window);
        }
};
Handle::Handle() { index = 0; counter = 0; type = 0; }
Handle::Handle(uint32 _index, uint32 _counter, uint32 _type) { index = _index; counter = _counter; type = _type; }
inline Handle::operator uint32() const { return type << 27 | counter << 12 | index; }
const bool Handle::null() const { if (type == ResourceType::Empty) return true; return false; }
const EngineResource* Handle::get() const {
    if (null()) return nullptr;
    return resourceManager->m_Resources->getAsFast<EngineResource>(index);
}
inline const EngineResource* Handle::operator ->() const { return get(); }

epriv::ResourceManager::ResourceManager(const char* name,uint width,uint height):m_i(new impl){
    m_i->_init(name,width,height);
    resourceManager = m_i.get();
}
epriv::ResourceManager::~ResourceManager(){ m_i->_destruct(); }
void epriv::ResourceManager::_init(const char* n,uint w,uint h){ m_i->_postInit(n,w,h); }

string Engine::Data::reportTime(){
    return epriv::Core::m_Engine->m_TimeManager->reportTime();
}
const double Engine::Resources::dt(){ return epriv::Core::m_Engine->m_TimeManager->dt(); }
Scene* Engine::Resources::getCurrentScene(){ return resourceManager->m_CurrentScene; }

bool epriv::ResourceManager::_hasScene(string n){ if(m_i->m_Scenes.count(n)) return true; return false; }
Texture* epriv::ResourceManager::_hasTexture(string n){
    auto& resourcePool = *(m_i->m_Resources);
    for(uint i = 0; i < resourcePool.maxEntries(); ++i){
        EngineResource* r = resourcePool.getAsFast<EngineResource>(i+1);
        if(r){ Texture* t = dynamic_cast<Texture*>(r); if(t && t->name() == n){ return t; } }
    }
    return 0;
}
void epriv::ResourceManager::_addScene(Scene* s){
    boost::shared_ptr<Scene> ptr(s);
    _addToContainer(m_i->m_Scenes,s->name(), ptr);
}
string epriv::ResourceManager::_buildSceneName(string n){return _incrementName(m_i->m_Scenes,n);}

uint epriv::ResourceManager::_numScenes(){return m_i->m_Scenes.size();}

void Resources::Settings::enableDynamicMemory(bool b){ resourceManager->m_DynamicMemory = b; }
void Resources::Settings::disableDynamicMemory(){ resourceManager->m_DynamicMemory = false; }

Engine_Window* Resources::getWindow(){ return resourceManager->m_Window; }
glm::uvec2 Resources::getWindowSize(){ return resourceManager->m_Window->getSize(); }

Scene* Resources::getScene(string n){return (Scene*)(_getFromContainer(resourceManager->m_Scenes,n));}

void Resources::getShader(Handle& h,Shader*& p){ resourceManager->m_Resources->getAs(h,p); }
Shader* Resources::getShader(Handle& h){ Shader* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getSoundData(Handle& h,SoundData*& p){ resourceManager->m_Resources->getAs(h,p); }
SoundData* Resources::getSoundData(Handle& h){ SoundData* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getCamera(Handle& h,Camera*& p){ resourceManager->m_Resources->getAs(h,p); }
Camera* Resources::getCamera(Handle& h){ Camera* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getFont(Handle& h,Font*& p){ resourceManager->m_Resources->getAs(h,p); }
Font* Resources::getFont(Handle& h){ Font* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getTexture(Handle& h,Texture*& p){ resourceManager->m_Resources->getAs(h,p); }
Texture* Resources::getTexture(Handle& h){ Texture* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getMesh(Handle& h,Mesh*& p){ resourceManager->m_Resources->getAs(h,p); }
Mesh* Resources::getMesh(Handle& h){ Mesh* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getMaterial(Handle& h,Material*& p){ resourceManager->m_Resources->getAs(h,p); }
Material* Resources::getMaterial(Handle& h){ Material* p; resourceManager->m_Resources->getAs(h,p); return p; }
void Resources::getShaderProgram(Handle& h,ShaderP*& p){ resourceManager->m_Resources->getAs(h,p); }
ShaderP* Resources::getShaderProgram(Handle& h){ ShaderP* p; resourceManager->m_Resources->getAs(h,p); return p; }


Handle Resources::addFont(string filename){
    return resourceManager->m_Resources->add(new Font(filename),ResourceType::Font);
}
Handle Resources::addMesh(string f, bool b,float threshhold){
    return resourceManager->m_Resources->add(new Mesh(f,b,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n,float x,float y,float w,float h,float threshhold){
    return resourceManager->m_Resources->add(new Mesh(n,x,y,w,h,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n,float w,float h,float threshhold){
    return resourceManager->m_Resources->add(new Mesh(n,w,h,threshhold),ResourceType::Mesh);
}
Handle Resources::addMesh(string n, unordered_map<string,float>& g, uint w, uint l,float threshhold){
    return resourceManager->m_Resources->add(new Mesh(n,g,w,l,threshhold),ResourceType::Mesh);
}
Handle Resources::addMeshAsync(string f, bool b,float threshhold){
    Mesh* mesh = new Mesh(f,b,threshhold,false);
    auto ref = std::ref(*mesh);
    auto job = boost::bind(&InternalMeshPublicInterface::LoadCPU, ref);
    auto cbk = boost::bind(&InternalMeshPublicInterface::LoadGPU, ref);
    Engine::epriv::threading::addJobWithPostCallback(job,cbk);
    return resourceManager->m_Resources->add(mesh, ResourceType::Mesh);
}


Handle epriv::ResourceManager::_addTexture(Texture* t){ return resourceManager->m_Resources->add(t,ResourceType::Texture); }
Handle Resources::addTexture(string file,ImageInternalFormat::Format internFormat,bool mipmaps){
    return resourceManager->m_Resources->add(new Texture(file,mipmaps,internFormat),ResourceType::Texture);
}

Handle Resources::addMaterial(string name, string diffuse, string normal,string glow, string specular){
    Material* material = new Material(name,diffuse,normal,glow,specular);
    return resourceManager->m_Resources->add(material,ResourceType::Material);
}
Handle Resources::addMaterial(string name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular){
    Material* material = new Material(name,diffuse,normal,glow,specular);
    return resourceManager->m_Resources->add(material,ResourceType::Material);
}

Handle Resources::addShader(string fileOrData, ShaderType::Type type, bool fromFile){
    return resourceManager->m_Resources->add(new Shader(fileOrData,type,fromFile),ResourceType::Shader);
}

Handle Resources::addShaderProgram(string n, Shader* v, Shader* f){
    return resourceManager->m_Resources->add(new ShaderP(n,v,f),ResourceType::ShaderProgram);
}
Handle Resources::addShaderProgram(string n, Handle& v, Handle& f){
    Shader* vS = 0; resourceManager->m_Resources->getAs(v,vS);
    Shader* fS = 0; resourceManager->m_Resources->getAs(f,fS);
    return resourceManager->m_Resources->add(new ShaderP(n,vS,fS),ResourceType::ShaderProgram);
}

Handle Resources::addSoundData(string file,string n,bool music){
    return resourceManager->m_Resources->add(new SoundData(file,music),ResourceType::SoundData);
}

void Resources::setCurrentScene(Scene* newScene){
    Scene* oldScene = resourceManager->m_CurrentScene;

    epriv::EventSceneChanged e; e.oldScene = oldScene; e.newScene = newScene;
    Event ev; ev.eventSceneChanged = e; ev.type = EventType::SceneChanged;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::SceneChanged,ev);
    
    if(!oldScene){
        cout << "---- Initial scene set to: " << newScene->name() << endl;
        resourceManager->m_CurrentScene = newScene;
        epriv::Core::m_Engine->m_ComponentManager->_sceneSwap(nullptr, newScene);       
        return;
    }
    if(oldScene != newScene){
        cout << "---- Scene Change started (" << oldScene->name() << ") to (" << newScene->name() << ") ----" << endl;
        if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory){
            //mark game object resources to minus use count
        }
        resourceManager->m_CurrentScene = newScene;
        epriv::Core::m_Engine->m_ComponentManager->_sceneSwap(oldScene, newScene);
        if(resourceManager->m_DynamicMemory){
            //mark game object resources to add use count
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(string s){Resources::setCurrentScene((Scene*)(_getFromContainer(resourceManager->m_Scenes,s)));}
