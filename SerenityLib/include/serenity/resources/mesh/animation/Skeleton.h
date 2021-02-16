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
};
#include <serenity/resources/mesh/animation/AnimationData.h>
#include <serenity/utils/Utils.h>
#include <serenity/system/Macros.h>

namespace Engine::priv {
    class MeshSkeleton final {
        friend class  Mesh;
        friend struct MeshCPUData;
        friend class  SMSH_File;
        friend class  Engine::priv::MeshLoader;
        friend class  Engine::priv::AnimationData;
        friend class  Engine::priv::PublicMesh;
        friend class  Engine::priv::ModelInstanceAnimation;
        private:
            Engine::unordered_string_map<std::string, uint16_t>       m_BoneMapping;   // maps a bone name to its index
            //Engine::unordered_string_set<std::string> m_BoneMapping;

            Engine::unordered_string_map<std::string, AnimationData>  m_AnimationData; //maps an animation name to its data
            glm::mat4                                                 m_GlobalInverseTransform = glm::mat4(1.0f);
            std::vector<BoneInfo>                                     m_BoneInfo;

            void clear() noexcept {
                m_BoneMapping.clear();
                m_GlobalInverseTransform = glm::mat4(1.0f);
            }
        public:
            MeshSkeleton() = default;
            MeshSkeleton(const aiMesh&, const aiScene&, MeshRequest&, MeshNodeData*, Engine::priv::MeshImportedData&);

            MeshSkeleton(const MeshSkeleton&)            = delete;
            MeshSkeleton& operator=(const MeshSkeleton&) = delete;
            MeshSkeleton(MeshSkeleton&&) noexcept        = default;
            MeshSkeleton& operator=(MeshSkeleton&&)      = default;

            [[nodiscard]] inline Engine::unordered_string_map<std::string, AnimationData>& getAnimationData() noexcept { 
                return m_AnimationData; 
            }
            [[nodiscard]] inline uint16_t numBones() const noexcept {
                return (uint16_t)m_BoneInfo.size();
            }
            [[nodiscard]] inline uint32_t numAnimations() const noexcept {
                return (uint32_t)m_AnimationData.size();
            }
            [[nodiscard]] inline bool hasBone(std::string_view boneName) const noexcept { 
                return m_BoneMapping.contains(boneName); 
            }
            [[nodiscard]] inline uint16_t getBoneIndex(std::string_view boneName) const noexcept {
                return m_BoneMapping.find(boneName)->second;
            }
            [[nodiscard]] inline bool hasAnimation(std::string_view animName) const noexcept {
                return m_AnimationData.contains(animName);
            }
            inline void setBoneOffsetMatrix(uint16_t boneIndex, glm::mat4&& matrix) noexcept {
                m_BoneInfo[boneIndex].BoneOffset = std::move(matrix);
            }
            void addBoneMapping(std::string_view boneName, uint16_t boneIndex) {
                if (hasBone(boneName)) {
                    return;
                }
                m_BoneMapping.emplace(std::piecewise_construct, std::forward_as_tuple(boneName), std::forward_as_tuple(boneIndex));
            }
            void addAnimation(std::string_view animName, MeshNodeData& nodeData, const aiAnimation& anim, MeshNodeData& filledNodes) {
                if (hasAnimation(animName)) {
                    return;
                }
                m_AnimationData.emplace(std::piecewise_construct, std::forward_as_tuple(animName), std::forward_as_tuple(nodeData, *this, anim, filledNodes));
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