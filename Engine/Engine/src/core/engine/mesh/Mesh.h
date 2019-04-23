#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <core/engine/mesh/VertexData.h>
#include <core/engine/mesh/MeshIncludes.h>

#include "core/engine/BindableResource.h"
#include "core/engine/events/Engine_EventObject.h"
#include "core/engine/Engine_Physics.h"


#include <map>
#include <unordered_map>

#include <assimp/scene.h>

typedef unsigned short ushort;

class  MeshInstance;
class  Mesh;

namespace Engine{
    namespace epriv{
        class  MeshLoader;
        class  MeshSkeleton;
        class  MeshCollisionFactory;
        struct DefaultMeshBindFunctor;
        struct DefaultMeshUnbindFunctor;
        struct BoneNode;
        class  AnimationData;
        struct InternalMeshRequestPublicInterface;
        struct InternalMeshPublicInterface final {
            static void LoadCPU(Mesh&);
            static void LoadGPU(Mesh&);
            static void UnloadCPU(Mesh&);
            static void UnloadGPU(Mesh&);
            static bool SupportsInstancing();
            static btCollisionShape* BuildCollision(Mesh*, CollisionType::Type);
        };
    };
};

class Mesh final: public BindableResource, public EventObserver{
    friend struct ::Engine::epriv::InternalMeshPublicInterface;
    friend struct ::Engine::epriv::InternalMeshRequestPublicInterface;
    friend struct ::Engine::epriv::DefaultMeshBindFunctor;
    friend struct ::Engine::epriv::DefaultMeshUnbindFunctor;
    friend class  ::Engine::epriv::AnimationData;
    friend class  ::Engine::epriv::MeshSkeleton;
    friend class  ::Engine::epriv::MeshLoader;
    friend class  ::Engine::epriv::MeshCollisionFactory;
    private:
        VertexData*                            m_VertexData;
        Engine::epriv::MeshCollisionFactory*   m_CollisionFactory;
        Engine::epriv::MeshSkeleton*           m_Skeleton;
        std::string                            m_File;
        glm::vec3                              m_radiusBox;
        float                                  m_radius;
        float                                  m_threshold;

        void init_blank();

        void unload_cpu();
        void unload_gpu();

        void load_cpu();
        void load_gpu();

        void finalize_vertex_data(Engine::epriv::MeshImportedData& data);
        void triangulate_component_indices(Engine::epriv::MeshImportedData& data, std::vector<std::vector<uint>>& indices, unsigned char flags);
        void calculate_radius();
    public:
        static Mesh *FontPlane, *Plane, *Cube; //loaded in renderer

        Mesh();
        Mesh(VertexData*, const std::string& name, float threshold = 0.0005f);
        Mesh(std::string name,float width, float height,float threshold); //plane
        Mesh(std::string fileOrData, float threshold = 0.0005f); //file or data
        ~Mesh();

        std::unordered_map<std::string, Engine::epriv::AnimationData>& animationData();
        const glm::vec3& getRadiusBox() const;
        const float getRadius() const;
        const VertexData& getVertexStructure() const;

        void onEvent(const Event& e);

        void load();
        void unload();

        template<typename T> void modifyVertices(uint attributeIndex, std::vector<T>& modifications, MeshModifyFlags::Flag _flags = MeshModifyFlags::Default) {
            auto& data = const_cast<VertexData&>(getVertexStructure());
            if (_flags & MeshModifyFlags::Orphan)
                data.setData<T>(attributeIndex, modifications, true, true);
            else
                data.setData<T>(attributeIndex, modifications, true, false);
        }
        void modifyIndices(std::vector<ushort>& modifiedIndices, MeshModifyFlags::Flag _flags = MeshModifyFlags::Default) {
            auto& data = const_cast<VertexData&>(getVertexStructure());
            if(_flags & MeshModifyFlags::Orphan)
                data.setDataIndices(modifiedIndices, true, true);
            else
                data.setDataIndices(modifiedIndices, true, false);
        }
        void render(bool instancing = true, MeshDrawMode::Mode = MeshDrawMode::Triangles);
        void playAnimation(std::vector<glm::mat4>&,const std::string& animationName,float time);
};
#endif