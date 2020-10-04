#pragma once
#ifndef ENGINE_MESH_SKELETON_H
#define ENGINE_MESH_SKELETON_H

class  Mesh;
class  SMSH_File;
namespace Engine::priv {
    class  InternalMeshPublicInterface;
    struct DefaultMeshBindFunctor;
    struct DefaultMeshUnbindFunctor;
    struct MeshImportedData;
    class  MeshLoader;
    class  ModelInstanceAnimation;
};
#include <core/engine/mesh/AnimationData.h>

namespace Engine::priv {
    class MeshSkeleton final {
        friend class  Mesh;
        friend class  SMSH_File;
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend struct Engine::priv::DefaultMeshBindFunctor;
        friend struct Engine::priv::DefaultMeshUnbindFunctor;
        friend class  Engine::priv::InternalMeshPublicInterface;
        friend class  Engine::priv::ModelInstanceAnimation;
        private:
            std::unordered_map<std::string, unsigned int>   m_BoneMapping;             // maps a bone name to its index
            std::unordered_map<std::string, AnimationData>  m_AnimationData;
            std::vector<BoneInfo>                           m_BoneInfo;
            glm::mat4                                       m_GlobalInverseTransform = glm::mat4(1.0f);
            unsigned int                                    m_NumBones                 = 0U;

            void clear() noexcept {
                m_NumBones = 0;
                m_BoneMapping.clear();
                m_GlobalInverseTransform = glm::mat4(1.0f);
            }
        public:
            MeshSkeleton() = default;
            MeshSkeleton(const MeshSkeleton& other)            = delete;
            MeshSkeleton& operator=(const MeshSkeleton& other) = delete;
            MeshSkeleton(MeshSkeleton&& other) noexcept        = default;
            MeshSkeleton& operator=(MeshSkeleton&& other)      = default;
            ~MeshSkeleton() = default;

            inline CONSTEXPR unsigned int numBones() const noexcept { return m_NumBones; }
    };
};
#endif