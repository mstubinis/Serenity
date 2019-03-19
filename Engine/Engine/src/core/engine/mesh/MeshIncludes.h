#pragma once
#ifndef ENGINE_MESH_INCLUDES_INCLUDE_GUARD
#define ENGINE_MESH_INCLUDES_INCLUDE_GUARD

#include <core/engine/Engine_Utils.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>

const uint NUM_BONES_PER_VERTEX   = 4;
const uint NUM_MAX_INSTANCES      = 65536;

struct MeshDrawMode {enum Mode {
    Triangles       = GL_TRIANGLES,
    Quads           = GL_QUADS,
    Points          = GL_POINTS,
    Lines           = GL_LINES,
    TriangleStrip   = GL_TRIANGLE_STRIP,
    TriangleFan     = GL_TRIANGLE_FAN,
    QuadStrip       = GL_QUAD_STRIP,
};};
struct MeshModifyFlags {enum Flag {
    Default = 0,
    Orphan  = 1,
};};

namespace Engine {
    namespace epriv {

        struct Vertex final {
            glm::vec3               position;
            glm::vec2               uv;
            glm::vec3               normal;
            glm::vec3               binormal;
            glm::vec3               tangent;
            Vertex() { clear(); }
            ~Vertex() {}
            void clear() { 
                position = normal = binormal = tangent = glm::vec3(0.0f);
                uv = glm::vec2(0.0f); 
            }
        };
        struct Triangle final {
            Vertex v1;
            Vertex v2;
            Vertex v3;
            Triangle() {}
            Triangle(Vertex& _v1, Vertex& _v2, Vertex& _v3) { 
                v1 = _v1; v2 = _v2; v3 = _v3; 
            }
            ~Triangle(){}
        };


        struct VertexBoneData final {
            float     IDs[NUM_BONES_PER_VERTEX];
            float Weights[NUM_BONES_PER_VERTEX];
            VertexBoneData() {
                for (uint i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                    IDs[i]     = 0.0f;
                    Weights[i] = 0.0f;
                }
            }
            void AddBoneData(uint BoneID, float Weight) {
                for (uint i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
                    if (Weights[i] == 0.0f) {
                        IDs[i] = float(BoneID);
                        Weights[i] = Weight;
                        return;
                    }
                }
            }
        };
    };
};


#endif