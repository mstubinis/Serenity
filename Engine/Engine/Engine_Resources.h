#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include "Engine_Physics.h"
#include "ShaderProgram.h"

#include <unordered_map>
#include <map>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

enum ENGINE_RENDERING_API{
    ENGINE_RENDERING_API_OPENGL,
    ENGINE_RENDERING_API_DIRECTX
};

typedef unsigned int uint;

class Engine_Window;
class Scene;
class Font;
class Texture;
class Camera;
class Mesh;
class Material;
class Object;
class SunLight;
class SoundEffectBasic;
class SoundEffect;
class SoundMusic;

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for releasing a pointer

class StringKey final{
	public:
		std::string* s;
		bool del;
		bool operator==(const StringKey& o) const { return *(s) == *(o.s); }
		bool operator<(const StringKey& o) const { return *(s) < *(o.s); }
		StringKey(std::string _key = ""){ 
			s = new std::string(_key);
			del = true;
		}
		~StringKey(){
			if(del){
				delete s;
				del = false;
			}
		}
		void lock(std::string* ptr){ delete s;s = ptr; del = false; } //the string pointer will be deleted sometime later manually
};
class skh final{
	public: size_t operator() (StringKey const& key) const{
		size_t hash = 0;
		for(size_t i = 0; i < key.s->size(); i++)
			hash += (71*hash + key.s->at(i)) % 5;
		return hash;
     }
};
class skef final{
	public: bool operator() (StringKey const& t1, StringKey const& t2) const{
		return !((*t1.s).compare((*t2.s)));
	}
};

namespace Engine{
    namespace Resources{
        namespace Detail{
            class ResourceManagement final{
                public:
					template<class V> static std::string _incrementName(std::unordered_map<StringKey,V,skh,skef>& m, std::string n){
						std::string r = n;
						if(m.size() > 0){
							uint c = 0;
							StringKey k(r);
							while(m.count(k)){
								r = n + " " + boost::lexical_cast<std::string>(c);c++;
							}
						}
						return r;
					}
					template<class V> static std::string _incrementName(std::map<StringKey,V,skh,skef>& m, std::string n){
						std::string r = n;
						if(m.size() > 0){
							uint c = 0;
							StringKey k(r);
							while(m.count(k)){
								r = n + " " + boost::lexical_cast<std::string>(c);c++;
							}
						}
						return r;
					}

					template<class V, class O> static void _addToContainer(std::map<StringKey,V,skh,skef>& m, std::string& n,O& o){
						StringKey k(n);
						if(m.size() > 0 && m.count(k)){
							o.reset();return;
						}
						k.lock(o->namePtr());
						m.insert(std::pair<StringKey,O>(k,o));
					}
					template<class V, class O> static void _addToContainer(std::unordered_map<StringKey,V,skh,skef>& m, std::string& n,O& o){
						StringKey k(n);
						if(m.size() > 0 && m.count(k)){
							o.reset();return;
						}
						k.lock(o->namePtr());
						m.insert(std::pair<StringKey,O>(k,o));
					}

					template<class V> static void* _getFromContainer(std::map<StringKey,V,skh,skef>& m, std::string& n){
						StringKey k(n);
						if(!m.count(k))
							return nullptr; 
						return m[k].get(); 
					}
					template<class V> static void* _getFromContainer(std::unordered_map<StringKey,V,skh,skef>& m, std::string& n){
						StringKey k(n);
						if(!m.count(k)) 
							return nullptr; 
						return m[k].get(); 
					}

					template<class V> static void _removeFromContainer(std::map<StringKey,V,skh,skef>& m, std::string& n){
						StringKey k(n);
						if (m.size() > 0 && m.count(k)){
							m[k].reset();
							m.erase(k);
						}
					}
					template<class V> static void _removeFromContainer(std::unordered_map<StringKey,V,skh,skef>& m, std::string& n){
						StringKey k(n);
						if (m.size() > 0 && m.count(k)){
							m[k].reset();
							m.erase(k);
						}
					}

                    static ENGINE_RENDERING_API m_RenderingAPI;
                    static Scene* m_CurrentScene;

                    static float m_DeltaTime;

                    static boost::weak_ptr<Camera> m_ActiveCamera;

                    static Engine_Window* m_Window;

					static std::unordered_map<StringKey,boost::shared_ptr<Scene>,skh,skef> m_Scenes;
                    static std::unordered_map<StringKey,boost::shared_ptr<SoundEffectBasic>,skh,skef> m_Sounds;
                    static std::unordered_map<StringKey,boost::shared_ptr<Object>,skh,skef> m_Objects;
                    static std::unordered_map<StringKey,boost::shared_ptr<Camera>,skh,skef> m_Cameras;
                    static std::unordered_map<StringKey,boost::shared_ptr<Font>,skh,skef> m_Fonts;
                    static std::unordered_map<StringKey,boost::shared_ptr<Mesh>,skh,skef> m_Meshes;
                    static std::unordered_map<StringKey,boost::shared_ptr<Texture>,skh,skef> m_Textures;
                    static std::unordered_map<StringKey,boost::shared_ptr<Material>,skh,skef> m_Materials;
                    static std::unordered_map<StringKey,boost::shared_ptr<Shader>,skh,skef> m_Shaders;
					static std::unordered_map<StringKey,boost::shared_ptr<ShaderP>,skh,skef> m_ShaderPrograms;

                    static void destruct();
            };
        };
        static Scene* getCurrentScene(){ return Detail::ResourceManagement::m_CurrentScene; }
        void setCurrentScene(Scene* s);
        void setCurrentScene(std::string s);

        static ENGINE_RENDERING_API getAPI(){ return Detail::ResourceManagement::m_RenderingAPI; }

        static float getDeltaTime(){ return Detail::ResourceManagement::m_DeltaTime; }
        static float dt(){ return Detail::ResourceManagement::m_DeltaTime; }

        Engine_Window* getWindow();
        sf::Vector2u getWindowSize();

        Camera* getActiveCamera();
        boost::weak_ptr<Camera>& getActiveCameraPtr();
        void setActiveCamera(Camera* c);
        void setActiveCamera(std::string name);

        Scene* getScene(std::string n);
        SoundEffectBasic* getSound(std::string n);
        SoundMusic* getSoundAsMusic(std::string n);
        SoundEffect* getSoundAsEffect(std::string n);

        boost::shared_ptr<Object>& getObjectPtr(std::string n);
        boost::shared_ptr<Camera>& getCameraPtr(std::string n);
        Object* getObject(std::string n);
        Camera* getCamera(std::string n);
        Font* getFont(std::string n);
        Texture* getTexture(std::string n);
        Mesh* getMesh(std::string n);
        Material* getMaterial(std::string n);
        Shader* getShader(std::string n);
		ShaderP* getShaderProgram(std::string n);

        void addMesh(std::string name,std::string file, COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL,bool fromFile = true);
        void addMesh(std::string file, COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL);
        void addMesh(std::string name, float x, float y, float w, float h);
		void addMesh(std::string name, float w, float h);
		void addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length);
        void removeMesh(std::string name);

        void addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "",std::string shader = "");
        void addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
        void removeMaterial(std::string name);

		void addShader(std::string name, std::string shaderFileOrData, SHADER_TYPE shaderType, bool fromFile = true);

		void addShaderProgram(std::string name, Shader* vertexShader, Shader* fragmentShader, SHADER_PIPELINE_STAGE = SHADER_PIPELINE_STAGE_GEOMETRY);
        void addShaderProgram(std::string name, std::string vertexShader, std::string fragmentShader, SHADER_PIPELINE_STAGE = SHADER_PIPELINE_STAGE_GEOMETRY);

        void addParticleInfo(std::string name, std::string material);
        void addParticleInfo(std::string name, Material* diffuse);
        void removeParticleInfo(std::string name);

        void addSound(std::string name, std::string file,bool asEffect = true);
        void addSoundAsEffect(std::string name, std::string file);
        void addSoundAsMusic(std::string name, std::string file);

        void removeSound(std::string name);

        void initResources();
        void initRenderingContexts(uint api);
        void cleanupRenderingContexts(uint api);
    };
    //TODO: Move this somewhere else
    template<typename T>
    static std::string convertNumToNumWithCommas(T n){
        std::string r = std::to_string(n);
        int insertPosition = r.length() - 3;
        while (insertPosition > 0) {
            r.insert(insertPosition, ",");
            insertPosition-=3;
        }
        return r;
    }
};
#endif