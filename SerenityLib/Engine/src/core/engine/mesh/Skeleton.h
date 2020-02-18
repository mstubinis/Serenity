#pragma once
#ifndef ENGINE_MESH_SKELETON_INCLUDE_GUARD
#define ENGINE_MESH_SKELETON_INCLUDE_GUARD

class  Mesh;
namespace Engine::priv {
    struct InternalMeshPublicInterface;
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
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend struct Engine::priv::DefaultMeshBindFunctor;
        friend struct Engine::priv::DefaultMeshUnbindFunctor;
        friend struct Engine::priv::InternalMeshPublicInterface;
        friend class  Engine::priv::ModelInstanceAnimation;
        private:
            BoneNode*                                       m_RootNode;
            unsigned int                                    m_NumBones;
            std::vector<BoneInfo>                           m_BoneInfo;
            std::vector<glm::vec4>                          m_BoneIDs, m_BoneWeights;
            std::unordered_map<std::string, unsigned int>   m_BoneMapping; // maps a bone name to its index
            std::unordered_map<std::string, AnimationData>  m_AnimationData;
            glm::mat4                                       m_GlobalInverseTransform;

            //void fill(const Engine::priv::MeshImportedData& data);
            void populateCleanupMap(BoneNode* node, std::unordered_map<std::string, BoneNode*>& _map);
            void cleanup();
            void clear();

        public:
            MeshSkeleton();
            ~MeshSkeleton();

            const unsigned int& numBones() const;
    };
};

#endif