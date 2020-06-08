#include <core/engine/mesh/smsh.h>
#include <core/engine/math/Engine_Math.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>

using namespace std;

VertexData* SMSH_File::LoadFile(const char* filename) {
    VertexData* vertexData = nullptr;

    boost::iostreams::mapped_file_source stream(filename);
    SMSH_Fileheader smsh_header;

    uint32_t blockStart = 0;
    const uint8_t* streamDataBuffer = (uint8_t*)stream.data();

    readBigEndian(smsh_header.m_InterleavingType, streamDataBuffer, 1, blockStart);
    readBigEndian(smsh_header.m_AttributeCount, streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceCount, streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceDataTypeSize, streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_UserDataCount, streamDataBuffer, 4, blockStart);

    vector<unsigned int> userData(smsh_header.m_UserDataCount, 0);
    for (size_t i = 0; i < smsh_header.m_UserDataCount; ++i) {
        readBigEndian(userData[i], streamDataBuffer, 4, blockStart);
    }

    /*
    vector<glm::vec3> temp_pos;
    vector<glm::vec2> temp_uvs;
    vector<GLuint>    temp_norm;
    vector<GLuint>    temp_binorm;
    vector<GLuint>    temp_tang;
    vector<glm::vec4> temp_bID;
    vector<glm::vec4> temp_bW;
    */

    /*
    VertexDataFormat data;
    const size_t stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(GLuint) + sizeof(GLuint) + sizeof(GLuint);
    data.m_InterleavingType = VertexAttributeLayout::Interleaved;
    data.add(3,       GL_FLOAT,              false, stride,  0, sizeof(glm::vec3)); //positions
    data.add(2,       GL_FLOAT,              false, stride, 12, sizeof(glm::vec2)); //uvs
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 20, sizeof(GLuint)); //normals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 24, sizeof(GLuint)); //binormals
    data.add(GL_BGRA, GL_INT_2_10_10_10_REV, true,  stride, 28, sizeof(GLuint)); //tangents
    return data;
    */


    VertexDataFormat vertexDataFormat;
    vertexDataFormat.m_InterleavingType = static_cast<VertexAttributeLayout::Type>(smsh_header.m_InterleavingType);

    vector<SMSH_AttributeNoBuffer> attr_data;
    for (size_t i = 0; i < smsh_header.m_AttributeCount; ++i) {
        attr_data.push_back(SMSH_AttributeNoBuffer());
        auto& attr = attr_data.back();

        readBigEndian(attr.m_Normalized, streamDataBuffer, 1, blockStart);
        readBigEndian(attr.m_Stride, streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_AttributeType, streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_SizeOfAttribute, streamDataBuffer, 4, blockStart);         //(example, 96 for 3 float vector, 32 for 2 half float vector, 32 for 1 packed unsigned int normal, etc)
        readBigEndian(attr.m_AttributeComponentCount, streamDataBuffer, 4, blockStart); //(3 for x,y,z | 4 for rgba, 2 for uv, etc)
        readBigEndian(attr.m_AttributeBufferSize, streamDataBuffer, 4, blockStart);     //size of the whole data buffer
        readBigEndian(attr.m_Offset, streamDataBuffer, 4, blockStart);

        uint8_t* buffer = NEW uint8_t[attr.m_AttributeBufferSize];
        std::memcpy(&buffer[0], &streamDataBuffer[blockStart], attr.m_AttributeBufferSize);
        blockStart += attr.m_AttributeBufferSize;
        
        vertexDataFormat.add(attr.m_AttributeComponentCount, attr.m_AttributeType, static_cast<bool>(attr.m_Normalized), attr.m_Stride, attr.m_Offset, attr.m_SizeOfAttribute);

        vertexData->setData(i, buffer, attr.m_AttributeBufferSize /*may have to be buffer size / size of attribute...*/, false, false);

        delete[] buffer;
    }

    return vertexData;
}

void SMSH_File::SaveFile(const char* filename, VertexData& vertexData) {
    SMSH_Fileheader smsh_header;

    smsh_header.m_InterleavingType       = SMSH_InterleavingType::Interleaved;
    smsh_header.m_AttributeCount         = vertexData.m_Format.m_Attributes.size();
    smsh_header.m_IndiceCount            = vertexData.m_Indices.size();

    if (smsh_header.m_IndiceCount <= 255) {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Byte;
    }else if (smsh_header.m_IndiceCount >= 256 && smsh_header.m_IndiceCount <= 65535) {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Short;
    }else {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Int;
    }
    smsh_header.m_UserDataCount          = 2U;

    std::ofstream stream(filename, ios::binary | ios::out);

    writeBigEndian(stream, smsh_header.m_InterleavingType);
    writeBigEndian(stream, smsh_header.m_AttributeCount);
    writeBigEndian(stream, smsh_header.m_IndiceCount);
    writeBigEndian(stream, smsh_header.m_IndiceDataTypeSize);
    writeBigEndian(stream, smsh_header.m_UserDataCount);

    for (unsigned int i = 0; i < smsh_header.m_UserDataCount; ++i) {
        unsigned int userData = 0; //replace the Zero with whatever userdata you want
        writeBigEndian(stream, userData);
    }

    //positions - 3 half floats
    SMSH_Attribute positions_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_3 };
    positions_attr.m_SizeOfAttribute = sizeof(uint16_t) * 3;
    positions_attr.m_Offset = 0;
    positions_attr.m_Normalized = false;
    const auto& positions = vertexData.getData<glm::vec3>(0);
    vector<uint16_t> positions_packed(positions.size() * 3);
    for (size_t i = 0; i < positions.size(); ++i) {
        Engine::Math::Float16From32(&positions_packed[(i * 4) + 0], positions[i].x);
        Engine::Math::Float16From32(&positions_packed[(i * 4) + 1], positions[i].y);
        Engine::Math::Float16From32(&positions_packed[(i * 4) + 2], positions[i].z);
    }
    positions_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint16_t*>(positions_packed.data())), positions_packed.size());

    //uvs - 2 half floats
    SMSH_Attribute uvs_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_2 };
    uvs_attr.m_SizeOfAttribute = sizeof(uint16_t) * 2;
    uvs_attr.m_Offset = 6;
    uvs_attr.m_Normalized = false;
    const auto& uvs = vertexData.getData<glm::vec2>(1);
    vector<uint16_t> uvs_packed(uvs.size() * 2);
    for (size_t i = 0; i < uvs.size(); ++i) {
        Engine::Math::Float16From32(&uvs_packed[(i * 4) + 0], uvs[i].x);
        Engine::Math::Float16From32(&uvs_packed[(i * 4) + 1], uvs[i].y);
    }
    uvs_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint16_t*>(uvs_packed.data())), uvs_packed.size());


    //normals - 1 packed unsigned 32 int
    SMSH_Attribute normals_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    normals_attr.m_SizeOfAttribute = sizeof(GLuint);
    normals_attr.m_Offset = 10;
    normals_attr.m_Normalized = true;
    const auto& normals = vertexData.getData<GLuint>(2);
    normals_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint32_t*>(normals.data())), normals.size());

    //binormals - 1 packed unsigned 32 int
    SMSH_Attribute binormals_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    binormals_attr.m_SizeOfAttribute = sizeof(GLuint);
    binormals_attr.m_Offset = 14;
    binormals_attr.m_Normalized = true; 
    const auto& binormals = vertexData.getData<GLuint>(3);
    binormals_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint32_t*>(binormals.data())), binormals.size());

    //tangents - 1 packed unsigned 32 int
    SMSH_Attribute tangents_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    tangents_attr.m_SizeOfAttribute = sizeof(GLuint);
    tangents_attr.m_Offset = 18;
    tangents_attr.m_Normalized = true;
    const auto& tangents = vertexData.getData<GLuint>(4);
    tangents_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint32_t*>(tangents.data())), tangents.size());

    //boneids - 4 half floats
    SMSH_Attribute boneids_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4 };
    boneids_attr.m_SizeOfAttribute = sizeof(uint16_t) * 4;
    boneids_attr.m_Offset = 22;
    boneids_attr.m_Normalized = false;
    const auto& boneids = vertexData.getData<glm::vec4>(5);
    vector<uint16_t> boneids_packed(boneids.size() * 4);
    for (size_t i = 0; i < boneids.size(); ++i) {
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 0], boneids[i].x);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 1], boneids[i].y);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 2], boneids[i].z);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 3], boneids[i].w);
    }
    boneids_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint16_t*>(boneids_packed.data())), boneids_packed.size());

    //boneweights - 4 half floats
    SMSH_Attribute boneweights_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4 };
    boneweights_attr.m_SizeOfAttribute = sizeof(uint16_t) * 4;
    boneweights_attr.m_Offset = 30;
    boneweights_attr.m_Normalized = false;
    const auto boneweights = vertexData.getData<glm::vec4>(6);
    vector<uint16_t> boneweights_packed(boneweights.size()*4);
    for (size_t i = 0; i < boneweights.size(); ++i) {
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 0], boneweights[i].x);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 1], boneweights[i].y);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 2], boneweights[i].z);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 3], boneweights[i].w);
    }
    boneweights_attr.setBuffer(reinterpret_cast<uint8_t*>(const_cast<uint16_t*>(boneweights_packed.data())), boneweights_packed.size());

    //strides
    unsigned int total_stride =
        (positions_attr.m_AttributeBufferSize > 0) ? positions_attr.m_SizeOfAttribute : 0 +
        (uvs_attr.m_AttributeBufferSize > 0) ? uvs_attr.m_SizeOfAttribute : 0 +
        (normals_attr.m_AttributeBufferSize > 0) ? normals_attr.m_SizeOfAttribute : 0 +
        (binormals_attr.m_AttributeBufferSize > 0) ? binormals_attr.m_Stride : 0 +
        (tangents_attr.m_AttributeBufferSize > 0) ? tangents_attr.m_Stride : 0 +
        (boneids_attr.m_AttributeBufferSize > 0) ? boneids_attr.m_Stride : 0 +
        (boneweights_attr.m_AttributeBufferSize > 0) ? boneweights_attr.m_Stride : 0;

    positions_attr.m_Stride   = total_stride;
    uvs_attr.m_Stride         = total_stride;
    normals_attr.m_Stride     = total_stride;
    binormals_attr.m_Stride   = total_stride;
    tangents_attr.m_Stride    = total_stride;
    boneids_attr.m_Stride     = total_stride;
    boneweights_attr.m_Stride = total_stride;


    //now write the attributes
    bool pos_write     = positions_attr.write(stream);
    bool uv_write      = uvs_attr.write(stream);
    bool norm_write    = normals_attr.write(stream);
    bool binorm_write  = binormals_attr.write(stream);
    bool tang_write    = tangents_attr.write(stream);
    bool boneid_write  = boneids_attr.write(stream);
    bool bonewgt_write = boneweights_attr.write(stream);

    //indices
    switch (smsh_header.m_IndiceDataTypeSize) {
        case SMSH_IndiceDataType::Unsigned_Byte: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint8_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 1U);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint16_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 2U);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 4U);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 4U);
            }
            break;
        }
    }
}