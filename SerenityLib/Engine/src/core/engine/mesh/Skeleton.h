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
    class MeshSkeleton final : public Engine::NonCopyable, public Engine::NonMoveable {
        friend class  Mesh;
        friend class  SMSH_File;
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend struct Engine::priv::DefaultMeshBindFunctor;
        friend struct Engine::priv::DefaultMeshUnbindFunctor;
        friend class  Engine::priv::InternalMeshPublicInterface;
        friend class  Engine::priv::ModelInstanceAnimation;
        private:
            unsigned int                                    m_NumBones                 = 0U;
            std::vector<BoneInfo>                           m_BoneInfo;
            std::unordered_map<std::string, unsigned int>   m_BoneMapping;             // maps a bone name to its index
            std::unordered_map<std::string, AnimationData>  m_AnimationData;
            glm::mat4                                       m_GlobalInverseTransform   = glm::mat4(1.0f);

            void clear();
        public:
            MeshSkeleton();
            ~MeshSkeleton();

            unsigned int numBones() const;
    };
};
#endif