#pragma once
#ifndef ENGINE_MESH_SKELETON_H
#define ENGINE_MESH_SKELETON_H

class  Mesh;
class  SMSH_File;
struct MeshCPUData;
struct MeshNodeData;
struct MeshRequest;
struct MeshRequestPart;
namespace Engine::priv {
    class  PublicMesh;
    class  MeshImportedData;
    class  MeshLoader;
    class  ModelInstanceAnimation;
    class  ModelInstanceAnimationContainer;
};

#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/system/Macros.h>
#include <serenity/utils/Utils.h>

namespace Engine::priv {
    class MeshSkeleton final {
        friend class  ::Mesh;
        friend struct ::MeshCPUData;
        friend class  ::SMSH_File;
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend class  Engine::priv::PublicMesh;
        friend class  Engine::priv::ModelInstanceAnimation;
        friend class  Engine::priv::ModelInstanceAnimationContainer;
        public:
            using AnimationDataMap = Engine::unordered_string_map<std::string, int32_t>;
        private:
            AnimationDataMap            m_AnimationMapping; //maps an animation name to its data Index
            glm::mat4                   m_GlobalInverseTransform = glm::mat4{ 1.0f };
            std::vector<glm::mat4>      m_BoneOffsets;
            std::vector<AnimationData>  m_AnimationData; //animations for this skeleton


            template<class ... ARGS>
            int internal_add_animation_impl(std::string_view animName, ARGS&&... args) {
                if (hasAnimation(animName)) {
                    return -1;
                }
                const int32_t index = static_cast<int32_t>(m_AnimationData.size());
                m_AnimationMapping.emplace(std::piecewise_construct, std::forward_as_tuple(animName), std::forward_as_tuple(index));
                m_AnimationData.emplace_back(std::forward<ARGS>(args)...);
                ENGINE_LOG("Added animation: " << animName);
                return index;
            }
        public:
            MeshSkeleton() = default;
            MeshSkeleton(const aiMesh&, const aiScene&, MeshRequestPart&, Engine::priv::MeshImportedData&);

            MeshSkeleton(const MeshSkeleton&)            = delete;
            MeshSkeleton& operator=(const MeshSkeleton&) = delete;
            MeshSkeleton(MeshSkeleton&&) noexcept        = default;
            MeshSkeleton& operator=(MeshSkeleton&&)      = default;

            [[nodiscard]] inline AnimationDataMap& getAnimationData() noexcept { return m_AnimationMapping; }
            [[nodiscard]] inline size_t numBones() const noexcept { return m_BoneOffsets.size(); }
            [[nodiscard]] inline const std::vector<glm::mat4>& getBoneOffsets() const noexcept { return m_BoneOffsets; }
            [[nodiscard]] inline size_t numAnimations() const noexcept { return m_AnimationData.size(); }
            [[nodiscard]] inline bool hasAnimation(std::string_view animName) const noexcept { return m_AnimationMapping.contains(animName); }
            [[nodiscard]] int32_t getAnimationIndex(std::string_view animName) const noexcept { auto it = m_AnimationMapping.find(animName); return (it == m_AnimationMapping.end()) ? -1 : it->second; }

            inline void setBoneOffsetMatrix(uint16_t boneIdx, glm::mat4&& matrix) noexcept { m_BoneOffsets[boneIdx] = std::move(matrix); }

            int addAnimation(std::string_view animName, const aiAnimation& anim, MeshRequestPart& part) { return internal_add_animation_impl(animName, anim, part); }
            int addAnimation(std::string_view animName, AnimationData&& animationData) { return internal_add_animation_impl(animName, std::move(animationData)); }
    };
};
#endif