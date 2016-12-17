#pragma once
#ifndef ENGINE_ENGINE_RESOURCES_H
#define ENGINE_ENGINE_RESOURCES_H

#include <unordered_map>
#include <map>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "Engine_Physics.h"

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
class ShaderP;
class Object;
class SunLight;
class SoundEffectBasic;
class SoundEffect;
class SoundMusic;
struct ParticleInfo;

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for releasing a pointer
//template<typename T> void SAFE_DELETE(T*& p){
//    delete p;
//    p = nullptr;
//}

template<class K, class V> std::string incrementName(std::unordered_map<K,V>& m, std::string n){
    std::string r = n;if(m.size() > 0){uint c = 0;while(m.count(r)){r = n + " " + boost::lexical_cast<std::string>(c);c++;}}return r;
}
template<class K, class V> std::string incrementName(std::map<K,V>& m, std::string n){
    std::string r = n;if(m.size() > 0){uint c = 0;while(m.count(r)){r = n + " " + boost::lexical_cast<std::string>(c);c++;}}return r;
}

namespace Engine{
    namespace Resources{
        namespace Detail{
            class ResourceManagement final{
                public:
                    static ENGINE_RENDERING_API m_RenderingAPI;
                    static std::unordered_map<std::string,boost::shared_ptr<Scene>> m_Scenes;
                    static Scene* m_CurrentScene;

                    static float m_DeltaTime;

                    static boost::weak_ptr<Camera> m_ActiveCamera;

                    static Engine_Window* m_Window;

                    static std::unordered_map<std::string,boost::shared_ptr<SoundEffectBasic>> m_Sounds;
                    static std::unordered_map<std::string,boost::shared_ptr<Object>> m_Objects;
                    static std::unordered_map<std::string,boost::shared_ptr<Camera>> m_Cameras;
                    static std::unordered_map<std::string,boost::shared_ptr<Font>> m_Fonts;
                    static std::unordered_map<std::string,boost::shared_ptr<Mesh>> m_Meshes;
                    static std::unordered_map<std::string,boost::shared_ptr<Texture>> m_Textures;
                    static std::unordered_map<std::string,boost::shared_ptr<Material>> m_Materials;
                    static std::unordered_map<std::string,boost::shared_ptr<ShaderP>> m_Shaders;
                    static std::unordered_map<std::string,boost::shared_ptr<ParticleInfo>> m_ParticleInfos;

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

        static Scene* getScene(std::string n){ 
            if(!Detail::ResourceManagement::m_Scenes.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Scenes[n].get(); 
        }
        static SoundEffectBasic* getSound(std::string n){ 
            if(!Detail::ResourceManagement::m_Sounds.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Sounds[n].get(); 
        }
        SoundMusic* getSoundAsMusic(std::string n);
        SoundEffect* getSoundAsEffect(std::string n);

        static boost::shared_ptr<Object>& getObjectPtr(std::string n){ 
            return Detail::ResourceManagement::m_Objects[n]; 
        }
        static boost::shared_ptr<Camera>& getCameraPtr(std::string n){ 
            return Detail::ResourceManagement::m_Cameras[n]; 
        }
        static Object* getObject(std::string n){
            if(!Detail::ResourceManagement::m_Objects.count(n))
                return nullptr;
            return getObjectPtr(n).get(); 
        }
        static Camera* getCamera(std::string n){
            if(!Detail::ResourceManagement::m_Cameras.count(n))
                return nullptr;
            return getCameraPtr(n).get(); 
        }
        static Font* getFont(std::string n){
            if(!Detail::ResourceManagement::m_Fonts.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Fonts[n].get(); 
        }
        static Texture* getTexture(std::string n){
            if(!Detail::ResourceManagement::m_Textures.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Textures[n].get(); 
        }
        static Mesh* getMesh(std::string n){
            if(!Detail::ResourceManagement::m_Meshes.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Meshes[n].get(); 
        }
        static Material* getMaterial(std::string n){ 
            if(!Detail::ResourceManagement::m_Materials.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_Materials[n].get(); 
        }
        static ParticleInfo* getParticleInfo(std::string n){ 
            if(!Detail::ResourceManagement::m_ParticleInfos.count(n))
                return nullptr;
            return Detail::ResourceManagement::m_ParticleInfos[n].get(); 
        }
        static ShaderP* getShader(std::string n){ 
            if(!Detail::ResourceManagement::m_Shaders.count(n)) return nullptr;
            return Detail::ResourceManagement::m_Shaders[n].get(); 
        }

        void addMesh(std::string name,std::string file, COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL,bool fromFile=true);
        void addMesh(std::string file, COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL);
        void addMesh(std::string name, float x, float y, float w, float h);
		void addMesh(std::string name, std::unordered_map<std::string,float>& grid, uint width, uint length);
        void removeMesh(std::string name);

        void addMaterial(std::string name, std::string diffuse, std::string normal = "", std::string glow = "",std::string specular = "");
        void addMaterial(std::string name, Texture* diffuse, Texture* normal = nullptr, Texture* glow = nullptr,Texture* specular = nullptr);
        void removeMaterial(std::string name);

        void addShader(std::string name, std::string vertexShader, std::string fragmentShader, bool fromFile = true);

        void addParticleInfo(std::string name, std::string material);
        void addParticleInfo(std::string name, Material* diffuse);
        void removeParticleInfo(std::string name);

        void addSound(std::string name, std::string file,bool asEffect = true);
        void addSoundAsEffect(std::string name, std::string file);
        void addSoundAsMusic(std::string name, std::string file);

        void removeSound(std::string name);

        void initResources();
        void initRenderingContexts(unsigned int api);
        void cleanupRenderingContexts(unsigned int api);
    };
    //TODO: Move this somewhere else
    template<typename T>
    static std::string convertNumToNumWithCommas(T n){
        std::string numWithCommas = std::to_string(n);
        int insertPosition = numWithCommas.length() - 3;
        while (insertPosition > 0) {
            numWithCommas.insert(insertPosition, ",");
            insertPosition-=3;
        }
        return numWithCommas;
    }
};
#endif