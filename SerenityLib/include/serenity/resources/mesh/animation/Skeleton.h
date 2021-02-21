#pragma once
#ifndef ENGINE_MESH_SKELETON_H
#define ENGINE_MESH_SKELETON_H

class  Mesh;
class  SMSH_File;
struct MeshCPUData;
struct MeshNodeData;
struct MeshRequest;
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
        friend class  Mesh;
        friend struct MeshCPUData;
        friend class  SMSH_File;
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend class  Engine::priv::PublicMesh;
        friend class  Engine::priv::ModelInstanceAnimation;
        friend class  Engine::priv::ModelInstanceAnimationContainer;
        public:
            using AnimationDataMap = Engine::unordered_string_map<std::string, AnimationData>;
        private:
            AnimationDataMap       m_AnimationData; //maps an animation name to its data
            glm::mat4              m_GlobalInverseTransform = glm::mat4(1.0f);
            std::vector<BoneInfo>  m_BoneInfo;

            void clear() noexcept {
                m_GlobalInverseTransform = glm::mat4{ 1.0f };
            }
        public:
            MeshSkeleton() = default;
            MeshSkeleton(const aiMesh&, const aiScene&, MeshRequest&, MeshNodeData*, Engine::priv::MeshImportedData&);

            MeshSkeleton(const MeshSkeleton&)            = delete;
            MeshSkeleton& operator=(const MeshSkeleton&) = delete;
            MeshSkeleton(MeshSkeleton&&) noexcept        = default;
            MeshSkeleton& operator=(MeshSkeleton&&)      = default;

            [[nodiscard]] inline AnimationDataMap& getAnimationData() noexcept { return m_AnimationData; }
            [[nodiscard]] inline uint16_t numBones() const noexcept { return (uint16_t)m_BoneInfo.size(); }
            [[nodiscard]] inline uint32_t numAnimations() const noexcept { return (uint32_t)m_AnimationData.size(); }
            [[nodiscard]] inline bool hasAnimation(std::string_view animName) const noexcept { return m_AnimationData.contains(animName); }
            inline void setBoneOffsetMatrix(uint16_t boneIdx, glm::mat4&& matrix) noexcept { m_BoneInfo[boneIdx].BoneOffset = std::move(matrix); }

            void addAnimation(std::string_view animName, const aiAnimation& anim, MeshRequest& request) {
                if (hasAnimation(animName)) {
                    return;
                }
                m_AnimationData.emplace(std::piecewise_construct, std::forward_as_tuple(animName), std::forward_as_tuple(anim, request));
                ENGINE_PRODUCTION_LOG("Added animation: " << animName)
            }
            void addAnimation(std::string_view animName, AnimationData&& animationData) {
                if (hasAnimation(animName)) {
                    return;
                }
                m_AnimationData.emplace(std::piecewise_construct, std::forward_as_tuple(animName), std::forward_as_tuple(std::move(animationData)));
                ENGINE_PRODUCTION_LOG("Added animation: " << animName)
            }
    };
};
#endif