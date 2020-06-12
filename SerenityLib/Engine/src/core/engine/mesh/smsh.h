#pragma once
#ifndef ENGINE_MESH_SMSH_H
#define ENGINE_MESH_SMSH_H

class Mesh;
namespace Engine::priv {
    class MeshSkeleton;
}

#include <core/engine/utils/Utils.h>
#include <core/engine/mesh/VertexData.h>

class SMSH_AttributeDataType final { public: enum Type : unsigned int {
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
};};
class SMSH_IndiceDataType final { public: enum Type : unsigned int {
    Unsigned_Byte   = GL_UNSIGNED_BYTE,
    Unsigned_Short  = GL_UNSIGNED_SHORT,
    Unsigned_Int    = GL_UNSIGNED_INT,
};};
class SMSH_AttributeComponentSize final{ public: enum Type : unsigned int {
    Unknown = 0,
    _1      = 1,
    _2      = 2,
    _3      = 3,
    _4      = 4,
    BGRA    = GL_BGRA,
};};

class SMSH_InterleavingType final { public: enum Type : unsigned char {
    Interleaved    = VertexAttributeLayout::Interleaved,
    NonInterleaved = VertexAttributeLayout::NonInterleaved,
};};

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
    unsigned char m_InterleavingType   = static_cast<unsigned char>(SMSH_InterleavingType::Interleaved);
    unsigned int  m_AttributeCount     = 0;
    unsigned int  m_UserDataCount      = 2;
    unsigned int  m_IndiceCount        = 0;
    unsigned int  m_IndiceDataTypeSize = static_cast<unsigned int>(SMSH_IndiceDataType::Unsigned_Short);
    unsigned int  m_NumberOfBones      = 0U;
};

struct SMSH_AttributeNoBuffer {
    unsigned char            m_Normalized              = 0;
    unsigned int             m_Stride                  = 0;
    unsigned int             m_AttributeType           = SMSH_AttributeDataType::Float;
    unsigned int             m_SizeOfAttribute         = 0;
    unsigned int             m_AttributeBufferSize     = 0;
    unsigned int             m_AttributeComponentCount = SMSH_AttributeComponentSize::_4;
    unsigned int             m_Offset                  = 0;

    SMSH_AttributeNoBuffer() {}
    SMSH_AttributeNoBuffer(SMSH_AttributeDataType::Type type, unsigned int componentCount) {
        m_AttributeType           = static_cast<unsigned short>(type);
        m_AttributeComponentCount = componentCount;
    }
};

struct SMSH_Attribute final : public SMSH_AttributeNoBuffer {
    const uint8_t*  m_AttributeBuffer = nullptr;

    SMSH_Attribute(unsigned int sizeOfAttr, unsigned int componentCount) : SMSH_AttributeNoBuffer(static_cast<SMSH_AttributeDataType::Type>(m_AttributeType), componentCount){
        m_SizeOfAttribute = sizeOfAttr;
    }
    SMSH_Attribute(SMSH_AttributeDataType::Type type, unsigned int componentCount) : SMSH_Attribute(0, componentCount){
        m_AttributeType   = type;
    }
    void setBuffer(const uint8_t* attrDataBuffer, unsigned int attrDataBufferSize) {
        m_AttributeBuffer     = attrDataBuffer;
        m_AttributeBufferSize = attrDataBufferSize;
    }
    template<class Stream> bool write(Stream& stream) {
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

        for (size_t i = 0; i < m_AttributeBufferSize; ++i) {
            writeBigEndian(stream, m_AttributeBuffer[i], 1U); //writing as uint8_t's
        }
        return true;
    }
};

class SMSH_File final {
    public:
        static void LoadFile(Mesh* mesh, const char* filename);
        static void SaveFile(const char* filename, Mesh& mesh);
};

#endif