#include <boost/make_shared.hpp>

#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_Resources.h"
#include "Engine_Sounds.h"
#include "Engine_Window.h"
#include "ShaderProgram.h"
#include "Object.h"
#include "Light.h"
#include "ObjectDynamic.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Texture.h"
#include "Font.h"
#include "Scene.h"

#include <SFML/Graphics.hpp>
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

epriv::ResourceManager* resourceManager;

namespace Engine{
	namespace epriv{
		struct HandleEntry final{
			uint32 nextFreeIndex : 12;
			uint32 counter : 15;
			uint32 active : 1;
			uint32 endOfList : 1;
			EngineResource* resource;
			HandleEntry(){
				counter = 1; nextFreeIndex, active, endOfList = 0; resource = nullptr;
			}
			explicit HandleEntry(uint32 _nextFreeIndex){
				nextFreeIndex = _nextFreeIndex; counter = 1; active, endOfList = 0; resource = nullptr;
			}
		};
	};
};

class epriv::ResourceManager::impl final{
    public:
		//TODO: convert to this resource system --------------------------------------------

		//http://gamesfromwithin.com/managing-data-relationships

		static const uint MAX_ENTRIES = 8192;
		HandleEntry m_Resources[MAX_ENTRIES];
		int m_activeEntryCount;
		uint32 m_firstFreeEntry;

		//create seperate containers for each resource type (mesh material etc)? vector.resize(8192) ?
		//
		// then store the object (as a pointer) to the vector's index corresponding to the handle id
		//    OR
		// store the object as it's handle ID to the vector's index corresponding to the handle id
		// then retrieve the actual resource object via m_Resources[ vector.at(id) ] and cast it to it's actual type?
		// this will come with a performance cost

		//experiment making some of these pointers references when passed around...

		void _Reset(){
			m_activeEntryCount = 0;
			m_firstFreeEntry = 0;
			for (int i = 0; i < MAX_ENTRIES - 1; ++i){
				// possibly delete the resouce here?
				m_Resources[i] = HandleEntry(i + 1);
			}
			m_Resources[MAX_ENTRIES - 1] = HandleEntry();
			m_Resources[MAX_ENTRIES - 1].endOfList = true;
		}
		Handle _Add(EngineResource*& p, ResourceType::Type& type){ //remove the pointer reference here?
			const int newIndex = m_firstFreeEntry;
			if(newIndex >= MAX_ENTRIES) return Handle(); //null handle
			m_firstFreeEntry = m_Resources[newIndex].nextFreeIndex;
			m_Resources[newIndex].nextFreeIndex = 0;
			++m_Resources[newIndex].counter;
			if (m_Resources[newIndex].counter == 0){
				m_Resources[newIndex].counter = 1;
			}
			m_Resources[newIndex].active = true;
			m_Resources[newIndex].resource = p;
			++m_activeEntryCount;
			return Handle (newIndex, m_Resources[newIndex].counter, type);
		}
		void _Visualize(){
			std::cout << "--------- Visualizing Resource Array ---------" << std::endl;
			for (int i = 0; i < MAX_ENTRIES - 1; ++i){
				HandleEntry& e = m_Resources[i];

				if(e.resource){
					std::cout << i << ": Active: " << e.active << " , Counter: " << e.counter << " , Name: " << e.resource->name() << std::endl;
				}
			}
			std::cout << "----------------------------------------------" << std::endl;
		}
		void _Update(Handle& h, EngineResource* p){
			const int index = h.index;
			if(m_Resources[index].counter == h.counter && m_Resources[index].active == true){
				m_Resources[index].resource = p;
			}
			else{
				std::cout << "Error: could not update ID: " << index << " , to resource: " << p->name() << std::endl;
			}
		}
		void _Remove(const Handle& h){
			const uint32 index = h.index;
			if(m_Resources[index].counter == h.counter && m_Resources[index].active == true){
				m_Resources[index].nextFreeIndex = m_firstFreeEntry;
				m_Resources[index].active = false;
				m_firstFreeEntry = index;
				--m_activeEntryCount;
			}
			else{
				std::cout << "Error: could not remove ID: " << index << " , resource: " << m_Resources[index].resource->name() << std::endl;
			}
		}
		EngineResource* _Get(Handle& h) const{
			EngineResource* p = nullptr;
			if (!_Get(h, p)) return nullptr;
			return p;
		}
		bool _Get(const Handle& h, EngineResource*& out) const{
			const int index = h.index;
			if (m_Resources[index].counter != h.counter || m_Resources[index].active == false)
				return false;
			out = m_Resources[index].resource;
			return true;
		}
		template<typename T> inline bool _GetAs(Handle& h, T*& out) const {
			EngineResource* _void;
			const bool rv = _Get(h,_void);
			out = (T*)_void; //use union_cast ? was in the original source
			return rv;
		}
		template<typename T> inline T* _GetAsFast(Handle& h) const {
			const int index = h.index;
			return (T*)m_Resources[index].resource;
		}
		//-----------------------------------------------------------------------------------------------


		Engine_Window* m_Window;
        Scene* m_CurrentScene;
		bool m_DynamicMemory;

        unordered_map<string,boost::shared_ptr<MeshInstance>> m_MeshInstances;
        unordered_map<string,boost::shared_ptr<Font>> m_Fonts;
        unordered_map<string,boost::shared_ptr<Mesh>> m_Meshes;
        unordered_map<string,boost::shared_ptr<Texture>> m_Textures;
        unordered_map<string,boost::shared_ptr<Material>> m_Materials;
        unordered_map<string,boost::shared_ptr<ShaderP>> m_ShaderPrograms;

        unordered_map<string,boost::shared_ptr<Scene>> m_Scenes;
        unordered_map<string,boost::shared_ptr<Object>> m_Objects;
        unordered_map<string,boost::shared_ptr<Camera>> m_Cameras;

		void _init(const char* name,const uint& width,const uint& height){
			m_CurrentScene = nullptr;
			m_DynamicMemory = false;
			_Reset();//this is needed
		}
		void _postInit(const char* name,uint width,uint height){
			m_Window = new Engine_Window(name,width,height);

			//add a basic cube mesh
			#pragma region MeshData
			string cubeMesh =  
				"v 1.0 -1.0 -1.0\n"
				"v 1.0 -1.0 1.0\n"
				"v -1.0 -1.0 1.0\n"
				"v -1.0 -1.0 -1.0\n"
				"v 1.0 1.0 -1.0\n"
				"v 1.0 1.0 1.0\n"
				"v -1.0 1.0 1.0\n"
				"v -1.0 1.0 -1.0\n"
				"vt 1.0 0.0\n"
				"vt 0.0 1.0\n"
				"vt 0.0 0.0\n"
				"vt 1.0 0.0\n"
				"vt 0.0 1.0\n"
				"vt 0.0 0.0\n"
				"vt 1.0 0.0\n"
				"vt 0.0 1.0\n"
				"vt 1.0 0.0\n"
				"vt 0.0 1.0\n"
				"vt 0.0 0.0\n"
				"vt 0.0 0.0\n"
				"vt 1.0 1.0\n"
				"vt 1.0 0.0\n"
				"vt 0.0 1.0\n"
				"vt 1.0 1.0\n"
				"vt 1.0 1.0\n"
				"vt 1.0 1.0\n"
				"vt 1.0 0.0\n"
				"vt 1.0 1.0\n"
				"vn 0.0 -1.0 0.0\n"
				"vn 0.0 1.0 0.0\n"
				"vn 1.0 -0.0 0.0\n"
				"vn 0.0 -0.0 1.0\n"
				"vn -1.0 -0.0 -0.0\n"
				"vn 0.0 0.0 -1.0\n"
				"f 2/1/1 4/2/1 1/3/1\n"
				"f 8/4/2 6/5/2 5/6/2\n"
				"f 5/7/3 2/8/3 1/3/3\n"
				"f 6/9/4 3/10/4 2/11/4\n"
				"f 3/12/5 8/13/5 4/2/5\n"
				"f 1/14/6 8/15/6 5/6/6\n"
				"f 2/1/1 3/16/1 4/2/1\n"
				"f 8/4/2 7/17/2 6/5/2\n"
				"f 5/7/3 6/18/3 2/8/3\n"
				"f 6/9/4 7/17/4 3/10/4\n"
				"f 3/12/5 7/19/5 8/13/5\n"
				"f 1/14/6 4/20/6 8/15/6";
			#pragma endregion
			Resources::addMesh("Cube",cubeMesh,CollisionType::None,false);

			Texture* brdfCook = new Texture("BRDFCookTorrance",512,512,ImageInternalFormat::RG16F,ImagePixelFormat::RG,ImagePixelType::FLOAT,GL_TEXTURE_2D,1.0f);
			brdfCook->setWrapping(TextureWrap::ClampToEdge);

			Resources::addMesh("Plane",1.0f,1.0f);
		}
		void _destruct(){
			for (auto it = m_MeshInstances.begin();it != m_MeshInstances.end(); ++it )   it->second.reset();
			for (auto it = m_Meshes.begin();it != m_Meshes.end(); ++it )                 it->second.reset();
			for (auto it = m_Textures.begin();it != m_Textures.end(); ++it )             it->second.reset();
			for (auto it = m_Fonts.begin();it != m_Fonts.end(); ++it )                   it->second.reset();
			for (auto it = m_Materials.begin();it != m_Materials.end(); ++it )           it->second.reset();
			for (auto it = m_ShaderPrograms.begin();it != m_ShaderPrograms.end(); ++it ) it->second.reset();
			for (auto it = m_Objects.begin();it != m_Objects.end(); ++it )               it->second.reset();
			for (auto it = m_Cameras.begin();it != m_Cameras.end(); ++it )               it->second.reset();
			for (auto it = m_Scenes.begin();it != m_Scenes.end(); ++it )                 it->second.reset();

			_Visualize(); //remove this eventually

			for(uint i = 0; i < MAX_ENTRIES; ++i){
				SAFE_DELETE(m_Resources[i].resource);
			}

			SAFE_DELETE(m_Window);
		}
};
epriv::ResourceManager::ResourceManager(const char* name,uint width,uint height):m_i(new impl){
	m_i->_init(name,width,height);
}
epriv::ResourceManager::~ResourceManager(){
	m_i->_destruct();
}
void epriv::ResourceManager::_init(const char* n,uint w,uint h){
	resourceManager = epriv::Core::m_Engine->m_ResourceManager;
	m_i->_postInit(n,w,h);
}



Handle epriv::ResourceManager::_addResource(EngineResource* r,ResourceType::Type t){
	return resourceManager->m_i->_Add(r,t);
}









string Engine::Data::reportTime(){
	return epriv::Core::m_Engine->m_TimeManager->reportTime();
}
float& Engine::Resources::dt(){ return epriv::Core::m_Engine->m_TimeManager->dt(); }
Scene* Engine::Resources::getCurrentScene(){ return resourceManager->m_i->m_CurrentScene; }

bool epriv::ResourceManager::_hasMaterial(string n){ if(resourceManager->m_i->m_Materials.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasMesh(string n){ if(resourceManager->m_i->m_Meshes.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasTexture(string n){ if(resourceManager->m_i->m_Textures.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasObject(string n){ if(resourceManager->m_i->m_Objects.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasFont(string n){ if(resourceManager->m_i->m_Fonts.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasScene(string n){ if(resourceManager->m_i->m_Scenes.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasMeshInstance(string n){ if(resourceManager->m_i->m_MeshInstances.count(n)) return true; return false; }
bool epriv::ResourceManager::_hasCamera(string n){ if(resourceManager->m_i->m_Cameras.count(n)) return true; return false; }
void epriv::ResourceManager::_addScene(Scene* s){
	_addToContainer(resourceManager->m_i->m_Scenes,s->name(),boost::shared_ptr<Scene>(s));
}
void epriv::ResourceManager::_addCamera(Camera* c){
	_addToContainer(resourceManager->m_i->m_Cameras,c->name(),boost::shared_ptr<Camera>(c));
}
void epriv::ResourceManager::_addFont(Font* f){
	_addToContainer(resourceManager->m_i->m_Fonts,f->name(),boost::shared_ptr<Font>(f));
}
Handle epriv::ResourceManager::_addShader(Shader* s){
	return resourceManager->_addResource(s,ResourceType::Shader);
}
void epriv::ResourceManager::_addTexture(Texture* t){
	_addToContainer(resourceManager->m_i->m_Textures,t->name(),boost::shared_ptr<Texture>(t));
}
void epriv::ResourceManager::_addMaterial(Material* m){
	_addToContainer(resourceManager->m_i->m_Materials,m->name(),boost::shared_ptr<Material>(m));
}
void epriv::ResourceManager::_addObject(Object* o){
	_addToContainer(resourceManager->m_i->m_Objects,o->name(),boost::shared_ptr<Object>(o));
}
void epriv::ResourceManager::_addMeshInstance(MeshInstance* m){
	_addToContainer(resourceManager->m_i->m_MeshInstances,m->name(),boost::shared_ptr<MeshInstance>(m));
}
void epriv::ResourceManager::_addMesh(Mesh* m){
    _addToContainer(resourceManager->m_i->m_Meshes,m->name(),boost::shared_ptr<Mesh>(m));
}
Handle epriv::ResourceManager::_addSoundData(SoundData* s){
    return resourceManager->_addResource(s,ResourceType::SoundData);
}
string epriv::ResourceManager::_buildMeshInstanceName(string n){return _incrementName(resourceManager->m_i->m_MeshInstances,n);}
string epriv::ResourceManager::_buildObjectName(string n){return _incrementName(resourceManager->m_i->m_Objects,n);}
string epriv::ResourceManager::_buildTextureName(string n){return _incrementName(resourceManager->m_i->m_Textures,n);}
string epriv::ResourceManager::_buildFontName(string n){return _incrementName(resourceManager->m_i->m_Fonts,n);}
string epriv::ResourceManager::_buildSceneName(string n){return _incrementName(resourceManager->m_i->m_Scenes,n);}
string epriv::ResourceManager::_buildMeshName(string n){return _incrementName(resourceManager->m_i->m_Meshes,n);}
string epriv::ResourceManager::_buildMaterialName(string n){return _incrementName(resourceManager->m_i->m_Materials,n);}
string epriv::ResourceManager::_buildCameraName(string n){return _incrementName(resourceManager->m_i->m_Cameras,n);}

void epriv::ResourceManager::_remCamera(string n){_removeFromContainer(resourceManager->m_i->m_Cameras,n);}
void epriv::ResourceManager::_remObject(string n){_removeFromContainer(resourceManager->m_i->m_Objects,n);}

void epriv::ResourceManager::_resizeCameras(uint w,uint h){for(auto c:resourceManager->m_i->m_Cameras){c.second.get()->resize(w,h);}}
uint epriv::ResourceManager::_numScenes(){return resourceManager->m_i->m_Scenes.size();}

void Resources::Settings::enableDynamicMemory(bool b){ resourceManager->m_i->m_DynamicMemory = b; }
void Resources::Settings::disableDynamicMemory(){ resourceManager->m_i->m_DynamicMemory = false; }









Engine_Window* Resources::getWindow(){ return resourceManager->m_i->m_Window; }
glm::uvec2 Resources::getWindowSize(){ return resourceManager->m_i->m_Window->getSize(); }

boost::shared_ptr<Object>& Resources::getObjectPtr(string n){return resourceManager->m_i->m_Objects.at(n);}
boost::shared_ptr<Camera>& Resources::getCameraPtr(string n){return resourceManager->m_i->m_Cameras.at(n);}
boost::shared_ptr<Texture>& Resources::getTexturePtr(string n){return resourceManager->m_i->m_Textures.at(n);}

Scene* Resources::getScene(string n){return (Scene*)(_getFromContainer(resourceManager->m_i->m_Scenes,n));}
Object* Resources::getObject(string n){return (Object*)(_getFromContainer(resourceManager->m_i->m_Objects,n));}
Camera* Resources::getCamera(string n){return (Camera*)(_getFromContainer(resourceManager->m_i->m_Cameras,n));}
Font* Resources::getFont(string n){return (Font*)(_getFromContainer(resourceManager->m_i->m_Fonts,n));}
Texture* Resources::getTexture(string n){return (Texture*)(_getFromContainer(resourceManager->m_i->m_Textures,n));}
Mesh* Resources::getMesh(string n){return (Mesh*)(_getFromContainer(resourceManager->m_i->m_Meshes,n));}
Material* Resources::getMaterial(string n){return (Material*)(_getFromContainer(resourceManager->m_i->m_Materials,n));}
ShaderP* Resources::getShaderProgram(string n){return (ShaderP*)(_getFromContainer(resourceManager->m_i->m_ShaderPrograms,n));}
MeshInstance* Resources::getMeshInstance(string n){return (MeshInstance*)(_getFromContainer(resourceManager->m_i->m_MeshInstances,n)); }

void Resources::getShader(Handle& h,Shader*& s){ resourceManager->m_i->_GetAs(h,s); }
Shader* Resources::getShader(Handle& h){ Shader* s; resourceManager->m_i->_GetAs(h,s); return s; }
void Resources::getSoundData(Handle& h,SoundData*& s){ resourceManager->m_i->_GetAs(h,s); }
SoundData* Resources::getSoundData(Handle& h){ SoundData* s; resourceManager->m_i->_GetAs(h,s); return s; }



void Resources::addMesh(string n,string f, CollisionType t, bool b,float threshhold){
    _addToContainer(resourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,f,t,b,threshhold));
}
void Resources::addMesh(string n,float x,float y,float w,float h,float threshhold){
    _addToContainer(resourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,x,y,w,h,threshhold));
}
void Resources::addMesh(string n,float w,float h,float threshhold){
    _addToContainer(resourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,w,h,threshhold));
}
void Resources::addMesh(string f, CollisionType t,float threshhold){string n = f.substr(0, f.size()-4);Resources::addMesh(n,f,t,true,threshhold);}
void Resources::addMesh(string n, unordered_map<string,float>& g, uint w, uint l,float threshhold){
    _addToContainer(resourceManager->m_i->m_Meshes,n,boost::make_shared<Mesh>(n,g,w,l,threshhold));
}

void Resources::addMaterial(string n, string d, string nm , string g, string s,string program){
    _addToContainer(resourceManager->m_i->m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == "") program = "Deferred";
    Resources::getShaderProgram(program)->addMaterial(n);
}
void Resources::addMaterial(string n, Texture* d, Texture* nm, Texture* g, Texture* s,ShaderP* program){
    _addToContainer(resourceManager->m_i->m_Materials,n,boost::make_shared<Material>(n,d,nm,g,s,program));
    if(program == nullptr) program = Resources::getShaderProgram("Deferred");
    program->addMaterial(n);
}

Handle Resources::addShader(string name, string fileOrData, ShaderType::Type type, bool fromFile){
	Shader* shader = new Shader(name,fileOrData,type,fromFile);
	return resourceManager->_addResource(shader,ResourceType::Shader);
}

void Resources::addShaderProgram(string n, Shader* v, Shader* f, ShaderRenderPass::Pass s){
    _addToContainer(resourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,v,f,s));
}
void Resources::addShaderProgram(string n, Handle& v, Handle& f, ShaderRenderPass::Pass s){
	Shader* vS = nullptr; resourceManager->m_i->_GetAs(v,vS);
	Shader* fS = nullptr; resourceManager->m_i->_GetAs(f,fS);
	_addToContainer(resourceManager->m_i->m_ShaderPrograms,n,boost::make_shared<ShaderP>(n,vS,fS,s));
}

Handle Resources::addSoundData(string file,string n,bool music){
	SoundData* soundData = new SoundData(file,music);
	return resourceManager->_addResource(soundData,ResourceType::SoundData);
}

void Resources::removeMesh(string n){_removeFromContainer(resourceManager->m_i->m_Meshes,n);}
void Resources::removeMaterial(string n){_removeFromContainer(resourceManager->m_i->m_Materials,n);}

void Resources::setCurrentScene(Scene* scene){
	if(resourceManager->m_i->m_CurrentScene == nullptr){
		resourceManager->m_i->m_CurrentScene = scene;
		return;
	}
	if(resourceManager->m_i->m_CurrentScene != scene){
        cout << "---- Scene Change started (" << resourceManager->m_i->m_CurrentScene->name() << ") to (" << scene->name() << ") ----" << endl;
        if(epriv::Core::m_Engine->m_ResourceManager->m_i->m_DynamicMemory){
            //mark game object resources to minus use count
            for(auto obj:resourceManager->m_i->m_CurrentScene->objects()){ obj.second->suspend(); }
            for(auto obj:resourceManager->m_i->m_CurrentScene->lights()){ obj.second->suspend(); }
			for(auto obj:resourceManager->m_i->m_CurrentScene->cameras()){ obj.second->suspend(); }
        }
		resourceManager->m_i->m_CurrentScene = scene;
        if(resourceManager->m_i->m_DynamicMemory){
            //mark game object resources to add use count
            for(auto obj:scene->objects()){ obj.second->resume(); }
            for(auto obj:scene->lights()){ obj.second->resume(); }
			for(auto obj:scene->cameras()){ obj.second->resume(); }
        }
        cout << "-------- Scene Change ended --------" << endl;
    }
}
void Resources::setCurrentScene(string s){Resources::setCurrentScene((Scene*)(_getFromContainer(resourceManager->m_i->m_Scenes,s)));}
