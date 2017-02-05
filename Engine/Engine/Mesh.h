#pragma once
#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#define GL_TRIANGLES 0x0004

#include "Engine_ResourceBasic.h"
#include "Engine_Physics.h"
#include <unordered_map>
#include <assimp/scene.h>

namespace sf{ class Image; };

class btHeightfieldTerrainShape;
struct ImportedMeshData;
struct BoneInfo;
struct VertexBoneData;
struct aiAnimation;
typedef unsigned int GLuint;
typedef unsigned int uint;
typedef unsigned short ushort;

const uint NUM_VERTEX_DATA = 5;
const uint VERTEX_AMOUNTS[NUM_VERTEX_DATA] = {3,2,3,3,3};

class AnimationData{
	friend class Mesh;
	private:
		Mesh* m_Mesh;
		aiAnimation* m_Animation;
		std::unordered_map<std::string,aiNodeAnim*> m_NodeAnimMap;

		void _ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
		void _BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms);
		void _SetBoneTransform(uint Index, glm::mat4& Transform);
		void _CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		void _CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		void _CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const aiNodeAnim* node);
		uint _FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint _FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		uint _FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

	public:
		AnimationData(Mesh*,uint animationIndex);
		~AnimationData();

		void play(float time);
		
};

class Mesh final: public EngineResource{
	friend class AnimationData;
    private:
        GLuint m_buffers[NUM_VERTEX_DATA]; //0 - position, 1 - uv, 2 - normal, 3 - binormals, 4 - tangents
		GLuint m_elementbuffer;
        Collision* m_Collision;

		//animation data
		std::unordered_map<std::string,uint> m_BoneMapping; // maps a bone name to its index
		uint m_NumBones;
		std::vector<BoneInfo> m_BoneInfo;
		glm::mat4 m_GlobalInverseTransform;
		std::vector<VertexBoneData> m_Bones;
		const aiScene* m_aiScene;
		std::unordered_map<std::string,AnimationData*> m_Animations;

        glm::vec3 m_radiusBox;
        float m_radius;
        std::vector<glm::vec3> m_Points;
        std::vector<glm::vec2> m_UVs;
        std::vector<glm::vec3> m_Normals;
        std::vector<glm::vec3> m_Binormals;
        std::vector<glm::vec3> m_Tangents;
		std::vector<ushort> m_Indices;

		void _loadData(ImportedMeshData&,float threshhold = 0.0005f);
        void _loadFromFile(std::string,COLLISION_TYPE);
        void _loadFromOBJ(std::string,COLLISION_TYPE);
        void _loadFromOBJMemory(std::string,COLLISION_TYPE);
        void _calculateMeshRadius();
    public:
        Mesh(std::string& name,btHeightfieldTerrainShape*);
		Mesh(std::string& name,std::unordered_map<std::string,float>& grid,uint width,uint length);
        Mesh(std::string& name,float width, float height);
        Mesh(std::string& name,float x, float y, float width, float height);
        Mesh(std::string& name,std::string = "",COLLISION_TYPE = COLLISION_TYPE_CONVEXHULL, bool notMemory = true);
        ~Mesh();

        void initRenderingContext();
        void cleanupRenderingContext();

        Collision* getCollision() const { return m_Collision; }
		std::unordered_map<std::string,AnimationData*>& animations(){ return m_Animations; }
        const glm::vec3& getRadiusBox() const { return m_radiusBox; }
        const float getRadius() const { return m_radius; }

        void render(GLuint mode = GL_TRIANGLES);
		void playAnimation(std::string animationName,float time);
};
#endif