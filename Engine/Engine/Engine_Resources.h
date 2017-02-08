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
class RenderedItem;

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for deleting a pointer

namespace Engine{
    namespace Resources{
        namespace Detail{
            class ResourceManagement final{
                public:
                    template<class K,class V,class H,class F,class S> static S _incrementName(std::unordered_map<K,V,H,F>& m,const S n){
                        S r = n;if(m.size() > 0){uint c = 0;while(m.count(r)){r = n + " " + boost::lexical_cast<S>(c);c++;}}return r;
                    }
                    template<class K,class V,class H,class F,class S> static S _incrementName(std::map<K,V,H,F>& m,const S n){
                        S r = n;if(m.size() > 0){uint c = 0;while(m.count(r)){r = n + " " + boost::lexical_cast<S>(c);c++;}}return r;
                    }
                    template<class K,class V, class O,class H,class F,class S> static void _addToContainer(std::map<K,V,H,F>& m,const S& n,O& o){
                        K k(o.get());if(m.size() > 0 && m.count(k)){o.reset();return;}m.emplace(k,o);
                    }
                    template<class K,class V, class O,class H,class F,class S> static void _addToContainer(std::unordered_map<K,V,H,F>& m,const S& n,O& o){
                        K k(o.get());if(m.size() > 0 && m.count(k)){o.reset();return;}m.emplace(k,o);
                    }
                    template<class K,class V,class H,class F,class S> static void* _getFromContainer(std::map<K,V,H,F>& m,const S& n){
                        if(!m.count(n))return nullptr; return m.at(n).get();
                    }
                    template<class K,class V,class H,class F,class S> static void* _getFromContainer(std::unordered_map<K,V,H,F>& m,const S& n){
                        if(!m.count(n)) return nullptr; return m.at(n).get();
                    }
                    template<class K,class V,class H,class F,class S> static void _removeFromContainer(std::map<K,V,H,F>& m,const S& n){
                        if (m.size() > 0 && m.count(n)){m.at(n).reset();m.erase(n);}
                    }
                    template<class K,class V,class H,class F,class S> static void _removeFromContainer(std::unordered_map<K,V,H,F>& m,const S& n){
                        if (m.size() > 0 && m.count(n)){m.at(n).reset();m.erase(n);}
                    }
                    static Scene* m_CurrentScene;

                    static float m_DeltaTime;
					static float m_ApplicationTime;

                    static boost::weak_ptr<Camera> m_ActiveCamera;

                    static Engine_Window* m_Window;

                    static std::unordered_map<skey,boost::shared_ptr<RenderedItem>,skh,skef> m_RenderedItems;
                    static std::unordered_map<skey,boost::shared_ptr<Scene>,skh,skef> m_Scenes;
                    static std::unordered_map<skey,boost::shared_ptr<SoundEffectBasic>,skh,skef> m_Sounds;
                    static std::unordered_map<skey,boost::shared_ptr<Object>,skh,skef> m_Objects;
                    static std::unordered_map<skey,boost::shared_ptr<Camera>,skh,skef> m_Cameras;
                    static std::unordered_map<skey,boost::shared_ptr<Font>,skh,skef> m_Fonts;
                    static std::unordered_map<skey,boost::shared_ptr<Mesh>,skh,skef> m_Meshes;
                    static std::unordered_map<skey,boost::shared_ptr<Texture>,skh,skef> m_Textures;
                    static std::unordered_map<skey,boost::shared_ptr<Material>,skh,skef> m_Materials;
                    static std::unordered_map<skey,boost::shared_ptr<Shader>,skh,skef> m_Shaders;
                    static std::unordered_map<skey,boost::shared_ptr<ShaderP>,skh,skef> m_ShaderPrograms;

                    static void destruct();
            };
        };
        static Scene* getCurrentScene(){ return Detail::ResourceManagement::m_CurrentScene; }
        void setCurrentScene(Scene* s);
        void setCurrentScene(std::string s);

        static float getDeltaTime(){ return Detail::ResourceManagement::m_DeltaTime; }
        static float dt(){ return Detail::ResourceManagement::m_DeltaTime; }
		static float applicationTime(){ return Detail::ResourceManagement::m_ApplicationTime; }

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

        boost::shared_ptr<Object>& getObjectPtr(std::string);
        boost::shared_ptr<Camera>& getCameraPtr(std::string);
        boost::shared_ptr<Texture>& getTexturePtr(std::string);

        Object* getObject(std::string n);
        Camera* getCamera(std::string n);
        Font* getFont(std::string n);
        Texture* getTexture(std::string n);
        Mesh* getMesh(std::string n);
        Material* getMaterial(std::string n);
        Shader* getShader(std::string n);
        ShaderP* getShaderProgram(std::string n);
        RenderedItem* getRenderedItem(std::string n);

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

        void addSound(std::string name, std::string file,bool asEffect = true);
        void addSoundAsEffect(std::string name, std::string file);
        void addSoundAsMusic(std::string name, std::string file);

        void removeSound(std::string name);

        void initResources();
        void initRenderingContexts();
        void cleanupRenderingContexts();
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