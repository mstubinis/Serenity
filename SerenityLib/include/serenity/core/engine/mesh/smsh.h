#pragma once
#ifndef ENGINE_MESH_SMSH_H
#define ENGINE_MESH_SMSH_H

class Mesh;
struct MeshCPUData;
namespace Engine::priv {
    class MeshSkeleton;
}

#include <serenity/core/engine/utils/Utils.h>
#include <serenity/core/engine/mesh/VertexData.h>

enum class SMSH_AttributeDataType : unsigned int {
    Float                        = GL_FLOAT,
    Byte                         = GL_BYTE,
    Unsigned_Byte                = GL_UNSIGNED_BYTE,
    Short                        = GL_SHORT,
    Unsigned_Short               = GL_UNSIGNED_SHORT,
    Int                          = GL_INT,
    Unsigned_Int                 = GL_UNSIGNED_INT,
    Half_Float                   = GL_HALF_FLOAT,
    Double                       = GL_DOUBLE,
    Fixed                        = GL_FIXED,
    UNSIGNED_INT_2_10_10_10_REV  = GL_UNSIGNED_INT_2_10_10_10_REV,
    UNSIGNED_INT_10F_11F_11F_REV = GL_UNSIGNED_INT_10F_11F_11F_REV,
    INT_2_10_10_10_REV           = GL_INT_2_10_10_10_REV,
};
enum class SMSH_IndiceDataType : unsigned int {
    Unsigned_Byte   = GL_UNSIGNED_BYTE,
    Unsigned_Short  = GL_UNSIGNED_SHORT,
    Unsigned_Int    = GL_UNSIGNED_INT,
};
enum class SMSH_AttributeComponentSize : unsigned int {
    Unknown = 0,
    _1      = 1,
    _2      = 2,
    _3      = 3,
    _4      = 4,
    BGRA    = GL_BGRA,
};
enum class SMSH_InterleavingType : unsigned char {
    Interleaved    = (unsigned char)VertexAttributeLayout::Interleaved,
    NonInterleaved = (unsigned char)VertexAttributeLayout::NonInterleaved,
};

/*
smsh is a file extension for this engine that holds mesh / geometry data.

metadata structure:
    - unsigned char: interleaving type
    - unsigned 32 int: attribute count
    - unsigned 32 int: indices count
    - unsigned 32 int: indice data type
    - unsigned 32 int: how many user data (every user data is a 32 bit int) (default to 2)
    - unsigned 32 numBones

    then for each user data
        - unsigned 32 int

    then for each attribute:
        - bool: normalized?
        - unsigned 32 int stride
        - unsigned 32 int enum type : 0 = float, 1 - int, 2 - unsigned int, 3 - short, 4 - unsigned short, etc... make an enum class to represent this
        - unsigned 32 int sizeof attribute (example, 96 for 3 float vector, 32 for 2 half float vector, 32 for 1 packed unsigned int normal, etc)
        - unsigned 32 int size of component (3 for x,y,z | 4 for rgba, 2 for uv, etc)
        - unsigned 32 int attribute data buffer size in bytes
        - unsigned 32 int offset
        - actual data buffer

    then bone / animation data:
        - global inverse transform (glm::mat4)

        - bone mapping (map of std::string keys => unsigned int value)
            for each bone item in map:
                unsigned 32 int: value in map
                unsigned 16 int: length of string key
                buffer of chars to form the string

        - bone info (vector of bone infos, which are just 2 mat4's : boneOffset and finalTransform)

        - unsigned 32 int: animation count
        - animation data (map) std::string anim_name => {
            ticksPerSecond (float),
            durationInTicks (float),
            unsigned 16 int: length of string key
            buffer of chars to form the string key

            
            keyframeData (map, string => custom class)
                unsigned 16 int: length of string key below
                buffer of chars to form the string key below
                unsigned 32 int : number of position keys
                unsigned 32 int : number of rotation keys
                unsigned 32 int : number of scale keys


                vector of positionKeys (glm::vec3 & float),
                    3 half floats for position, 1 32 bit float for time

                vector of rotationKeys (aiQuaternion & float),
                    4 half floats for quaternion, 1 32 bit float for time
                vector of scaleKeys (glm::vec3 & float),
                    3 half floats for scale, 1 32 bit float for time

        }
        -node heirarchy:
        : will store all nodes in a vector and then sort them by name
        : will then create a second 2d vector describing parent => child relationship using indices into the sorted vector
        : will then write out the nodes to file in 2 passes:
           - 1st pass: all data except children
           - 2nd pass: children

        - Node {
            - parentNode
            - name
            - glm::mat4 transform
            - vector of children
        }

*/

struct SMSH_Fileheader final {
    uint8_t   m_InterleavingType   = (uint8_t)SMSH_InterleavingType::Interleaved;
    uint32_t  m_AttributeCount     = 0;
    uint32_t  m_UserDataCount      = 2;
    uint32_t  m_IndiceCount        = 0;
    uint32_t  m_IndiceDataTypeSize = (uint32_t)SMSH_IndiceDataType::Unsigned_Short;
    uint32_t  m_NumberOfBones      = 0U;

    SMSH_Fileheader() = default;
    SMSH_Fileheader(MeshCPUData& cpuData);
};

struct SMSH_AttributeNoBuffer {
    uint8_t     m_Normalized              = 0;
    uint32_t    m_Stride                  = 0;
    uint32_t    m_AttributeType           = (uint32_t)SMSH_AttributeDataType::Float;
    uint32_t    m_SizeOfAttribute         = 0;
    uint32_t    m_AttributeBufferSize     = 0;
    uint32_t    m_AttributeComponentCount = (uint32_t)SMSH_AttributeComponentSize::_4;
    uint32_t    m_Offset                  = 0;

    SMSH_AttributeNoBuffer() = default;

    SMSH_AttributeNoBuffer(SMSH_AttributeDataType type, uint32_t componentCount, uint32_t offset, uint32_t normalized, uint32_t stride, uint32_t sizeOfAttr, uint32_t attributeBufferSize) 
        : m_AttributeType((uint32_t)type)
        , m_AttributeComponentCount(componentCount)
        , m_Offset(offset)
        , m_Normalized(normalized)
        , m_Stride(stride)
        , m_SizeOfAttribute(sizeOfAttr)
        , m_AttributeBufferSize(attributeBufferSize)
    {}
};

struct SMSH_Attribute final : public SMSH_AttributeNoBuffer {
    const uint8_t*  m_AttributeBuffer = nullptr;

    SMSH_Attribute(SMSH_AttributeDataType type, SMSH_AttributeComponentSize componentCount, uint32_t offset, uint32_t normalized, uint32_t stride, uint32_t sizeOfAttr, uint32_t attributeBufferSize)
        : SMSH_AttributeNoBuffer{ type, (uint32_t)componentCount, offset, normalized, stride, sizeOfAttr, attributeBufferSize }
    {}


    void setBuffer(const uint8_t* attrDataBuffer, uint32_t attrDataBufferSize) {
        m_AttributeBuffer     = attrDataBuffer;
        m_AttributeBufferSize = attrDataBufferSize;
    }
    template<class STREAM> bool write(STREAM& stream) {
        if (m_AttributeBufferSize == 0 || !m_AttributeBuffer) {
            return false;
        }
        writeBigEndian(stream, m_Normalized,               1U);
        writeBigEndian(stream, m_Stride,                   4U);
        writeBigEndian(stream, m_AttributeType,            4U);
        writeBigEndian(stream, m_SizeOfAttribute,          4U);
        writeBigEndian(stream, m_AttributeComponentCount,  4U);
        writeBigEndian(stream, m_AttributeBufferSize,      4U);
        writeBigEndian(stream, m_Offset,                   4U);

        for (uint32_t i = 0; i < m_AttributeBufferSize; ++i) {
            writeBigEndian(stream, m_AttributeBuffer[i], 1U); //writing as uint8_t's
        }
        return true;
    }
};

class SMSH_File final {
    public:
        static void LoadFile(const char* filename, MeshCPUData& cpuData);
        static void SaveFile(const char* filename, MeshCPUData& cpuData);
};

#endif