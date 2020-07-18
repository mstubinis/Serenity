#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/mesh/smsh.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;

void SMSH_File::LoadFile(Mesh* mesh, const char* filename) {
    boost::iostreams::mapped_file_source stream(filename);
    SMSH_Fileheader smsh_header;

    uint32_t blockStart = 0;
    const uint8_t* streamDataBuffer = (uint8_t*)stream.data();

    readBigEndian(smsh_header.m_InterleavingType,   streamDataBuffer, 1, blockStart);
    readBigEndian(smsh_header.m_AttributeCount,     streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceCount,        streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceDataTypeSize, streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_UserDataCount,      streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_NumberOfBones,      streamDataBuffer, 4, blockStart);

    vector<unsigned int> userData(smsh_header.m_UserDataCount, 0);
    for (size_t i = 0; i < smsh_header.m_UserDataCount; ++i) {
        readBigEndian(userData[i], streamDataBuffer, 4, blockStart);
    }
    VertexDataFormat vertexDataFormat;
    vertexDataFormat.m_InterleavingType = static_cast<VertexAttributeLayout::Type>(smsh_header.m_InterleavingType);

    vector<tuple<SMSH_AttributeNoBuffer, uint8_t*, unsigned int>> attr_data;
    attr_data.reserve(smsh_header.m_AttributeCount);
    for (size_t i = 0; i < smsh_header.m_AttributeCount; ++i) {
        auto& p = attr_data.emplace_back(make_tuple(SMSH_AttributeNoBuffer(), nullptr, 0U));
        auto& attr = std::get<0>(p);
        auto& buff = std::get<1>(p);

        readBigEndian(attr.m_Normalized,              streamDataBuffer, 1, blockStart);
        readBigEndian(attr.m_Stride,                  streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_AttributeType,           streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_SizeOfAttribute,         streamDataBuffer, 4, blockStart);         //(example, 96 for 3 float vector, 32 for 2 half float vector, 32 for 1 packed unsigned int normal, etc)
        readBigEndian(attr.m_AttributeComponentCount, streamDataBuffer, 4, blockStart); //(3 for x,y,z | 4 for rgba, 2 for uv, etc)
        readBigEndian(attr.m_AttributeBufferSize,     streamDataBuffer, 4, blockStart);     //size of the whole data buffer
        readBigEndian(attr.m_Offset,                  streamDataBuffer, 4, blockStart);

        buff = NEW uint8_t[attr.m_AttributeBufferSize];
        std::memcpy(&buff[0], &streamDataBuffer[blockStart], attr.m_AttributeBufferSize);
        blockStart += attr.m_AttributeBufferSize;

        vertexDataFormat.add(attr.m_AttributeComponentCount, attr.m_AttributeType, static_cast<bool>(attr.m_Normalized), attr.m_Stride, attr.m_Offset, attr.m_SizeOfAttribute);
    }
    mesh->m_File = filename;
    mesh->m_VertexData = NEW VertexData(vertexDataFormat);

    for (size_t i = 0; i < smsh_header.m_AttributeCount; ++i) {
        auto& p    = attr_data[i];
        auto& attr = std::get<0>(p);
        auto& buff = std::get<1>(p);
        mesh->m_VertexData->setData(i, buff, attr.m_AttributeBufferSize, attr.m_AttributeBufferSize / attr.m_SizeOfAttribute, false, false);
        delete[] buff;
    }
    //indices
    std::vector<unsigned int> indices;
    indices.reserve(smsh_header.m_IndiceCount);
    switch (smsh_header.m_IndiceDataTypeSize) {
        case SMSH_IndiceDataType::Unsigned_Byte: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint8_t indice;
                readBigEndian(indice, streamDataBuffer, 1U, blockStart);
                indices.push_back(static_cast<unsigned int>(indice));
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint16_t indice;
                readBigEndian(indice, streamDataBuffer, 2U, blockStart);
                indices.push_back(static_cast<unsigned int>(indice));
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint32_t indice;
                readBigEndian(indice, streamDataBuffer, 4U, blockStart);
                indices.push_back(indice);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                uint32_t indice;
                readBigEndian(indice, streamDataBuffer, 4U, blockStart);
                indices.push_back(indice);
            }
            break;
        }
    }
    mesh->m_VertexData->setIndices(indices.data(), indices.size(), false, false, true);

    //animation data
    if (smsh_header.m_NumberOfBones > 0U) {
        auto lamda_read_mat4_as_half_floats = [&blockStart, &streamDataBuffer](glm::mat4& inMatrix) {
            for (unsigned int i = 0U; i < 4U; ++i) {
                for (unsigned int j = 0U; j < 4U; ++j) {
                    uint16_t half_float;
                    readBigEndian(half_float, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&inMatrix[i][j], half_float);
                }
            }
        };
        mesh->m_Skeleton = NEW Engine::priv::MeshSkeleton();
        mesh->m_Skeleton->m_NumBones = smsh_header.m_NumberOfBones;
        //global inverse transform (glm::mat4 stored as half floats)
        lamda_read_mat4_as_half_floats(mesh->m_Skeleton->m_GlobalInverseTransform);
        //bone mapping
        for (size_t i = 0; i < smsh_header.m_NumberOfBones; ++i) {
            uint32_t map_value;
            uint16_t str_len;
            std::string map_name = "";
            readBigEndian(map_value, streamDataBuffer, 4U, blockStart);
            readBigEndian(str_len, streamDataBuffer, 2U, blockStart);
            for (size_t j = 0; j < str_len; ++j) {
                char c;
                readBigEndian(c, streamDataBuffer, 1, blockStart);
                map_name += c;
            }
            mesh->m_Skeleton->m_BoneMapping.emplace(map_name, map_value);
        }
        //bone info vector
        for (size_t i = 0; i < smsh_header.m_NumberOfBones; ++i) {
            auto& info = mesh->m_Skeleton->m_BoneInfo.emplace_back();
            lamda_read_mat4_as_half_floats(info.BoneOffset);
            lamda_read_mat4_as_half_floats(info.FinalTransform);
        }
        //animation data
        uint32_t animCount;
        readBigEndian(animCount, streamDataBuffer, 4U, blockStart);
        for (uint32_t i = 0; i < animCount; ++i) {
            //auto x = ret.skeleton->m_AnimationData.emplace_back();
            uint16_t half_float_ticks_per_second;
            uint16_t half_float_duration_in_ticks;
            uint16_t str_len;
            std::string anim_name = "";
            readBigEndian(half_float_ticks_per_second, streamDataBuffer, 2U, blockStart);
            readBigEndian(half_float_duration_in_ticks, streamDataBuffer, 2U, blockStart);
            readBigEndian(str_len, streamDataBuffer, 2U, blockStart);
            for (size_t j = 0; j < str_len; ++j) {
                char anim_name_char;
                readBigEndian(anim_name_char, streamDataBuffer, 1U, blockStart);
                anim_name += anim_name_char;
            }
            uint32_t keyframeCount;
            readBigEndian(keyframeCount, streamDataBuffer, 4U, blockStart);

            float ticksPerSec, durInTicks;
            Engine::Math::Float32From16(&ticksPerSec, half_float_ticks_per_second);
            Engine::Math::Float32From16(&durInTicks, half_float_duration_in_ticks);
            Engine::priv::AnimationData animationData(*mesh, ticksPerSec, durInTicks);
            for (size_t j = 0; j < keyframeCount; ++j) {
                //itr.second.m_KeyframeData
                uint32_t pos_count, rot_count, scl_count;
                uint16_t keyframe_name_len;
                std::string keyframe_key = "";
                readBigEndian(pos_count, streamDataBuffer, 4U, blockStart);
                readBigEndian(rot_count, streamDataBuffer, 4U, blockStart);
                readBigEndian(scl_count, streamDataBuffer, 4U, blockStart);
                readBigEndian(keyframe_name_len, streamDataBuffer, 2U, blockStart);
                for (size_t k = 0; k < keyframe_name_len; ++k) {
                    char keyframe_key_char;
                    readBigEndian(keyframe_key_char, streamDataBuffer, 1U, blockStart);
                    keyframe_key += keyframe_key_char;
                }

                Engine::priv::AnimationChannel channel;
                for (size_t k = 0; k < pos_count; ++k) {
                    auto& posKey = channel.PositionKeys.emplace_back();
                    uint16_t pos_half[3];
                    for (int i = 0; i < 3; ++i) {
                        readBigEndian(pos_half[i], streamDataBuffer, 2, blockStart);
                        Engine::Math::Float32From16(&posKey.value[i], pos_half[i]);
                    }
                    uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&posKey.time, time);
                }
                for (size_t k = 0; k < rot_count; ++k) {
                    auto& rotKey = channel.RotationKeys.emplace_back();
                    uint16_t rot_half[4];

                    readBigEndian(rot_half[0], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.x, rot_half[0]);
                    readBigEndian(rot_half[1], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.y, rot_half[1]);
                    readBigEndian(rot_half[2], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.z, rot_half[2]);
                    readBigEndian(rot_half[3], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.w, rot_half[3]);

                    uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&rotKey.time, time);
                }
                for (size_t k = 0; k < scl_count; ++k) {
                    auto& sclKey = channel.ScalingKeys.emplace_back();
                    uint16_t scl_half[3];
                    for (int i = 0; i < 3; ++i) {
                        readBigEndian(scl_half[i], streamDataBuffer, 2, blockStart);
                        Engine::Math::Float32From16(&sclKey.value[i], scl_half[i]);
                    }
                    uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&sclKey.time, time);
                }
                animationData.m_KeyframeData.emplace(keyframe_key, channel);
            }
            mesh->m_Skeleton->m_AnimationData.emplace(anim_name, animationData);
        }
        uint32_t num_of_nodes;
        readBigEndian(num_of_nodes, streamDataBuffer, 4U, blockStart);
        std::vector<Engine::priv::MeshInfoNode*> sortedNodes;
        sortedNodes.reserve(num_of_nodes);
        for (uint32_t i = 0; i < num_of_nodes; ++i) {
            glm::mat4 node_transform;
            lamda_read_mat4_as_half_floats(node_transform);
            uint16_t str_len;
            std::string node_name = "";
            readBigEndian(str_len, streamDataBuffer, 2U, blockStart);
            for (auto j = 0; j < str_len; ++j) {
                char node_name_char;
                readBigEndian(node_name_char, streamDataBuffer, 1U, blockStart);
                node_name += node_name_char;
            }
            sortedNodes.push_back(NEW Engine::priv::MeshInfoNode(std::move(node_name), std::move(node_transform)));
        }
        for (uint32_t i = 0; i < num_of_nodes; ++i) {
            uint32_t num_of_children;
            readBigEndian(num_of_children, streamDataBuffer, 4U, blockStart);
            for (uint32_t j = 0; j < num_of_children; ++j) {
                uint32_t child;
                readBigEndian(child, streamDataBuffer, 4U, blockStart);
                sortedNodes[i]->Children.push_back(sortedNodes[child]);
                sortedNodes[child]->Parent = sortedNodes[i];
            }
        }
        Engine::priv::MeshInfoNode* root = nullptr;
        for (size_t i = 0; i < sortedNodes.size(); ++i) {
            if (!sortedNodes[i]->Parent) {
                root = sortedNodes[i];
                break;
            }
        }
        mesh->m_RootNode = root;
    }
}
void SMSH_File::SaveFile(const char* filename, Mesh& mesh) {
    auto& vertexData = *mesh.m_VertexData;
    auto* skeleton   = mesh.m_Skeleton;
    SMSH_Fileheader smsh_header;

    smsh_header.m_InterleavingType       = SMSH_InterleavingType::Interleaved;
    smsh_header.m_AttributeCount         = (unsigned int)vertexData.m_Format.m_Attributes.size();
    smsh_header.m_IndiceCount            = (unsigned int)vertexData.m_Indices.size();

    if (smsh_header.m_IndiceCount <= 255) {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Byte;
    }else if (smsh_header.m_IndiceCount >= 256 && smsh_header.m_IndiceCount <= 65535) {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Short;
    }else {
        smsh_header.m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Int;
    }
    smsh_header.m_UserDataCount          = 2U;
    smsh_header.m_NumberOfBones          = (skeleton) ? skeleton->numBones() : 0U;

    std::ofstream stream(filename, ios::binary | ios::out);

    writeBigEndian(stream, smsh_header.m_InterleavingType);
    writeBigEndian(stream, smsh_header.m_AttributeCount);
    writeBigEndian(stream, smsh_header.m_IndiceCount);
    writeBigEndian(stream, smsh_header.m_IndiceDataTypeSize);
    writeBigEndian(stream, smsh_header.m_UserDataCount);
    writeBigEndian(stream, smsh_header.m_NumberOfBones);

    for (unsigned int i = 0; i < smsh_header.m_UserDataCount; ++i) {
        unsigned int userData = 0; //replace the Zero with whatever userdata you want
        writeBigEndian(stream, userData);
    }

    //positions - 3 half floats
    SMSH_Attribute positions_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_3 };
    positions_attr.m_SizeOfAttribute = sizeof(uint16_t) * 3;
    positions_attr.m_Offset          = 0;
    positions_attr.m_Normalized      = false;
    auto positions                   = vertexData.getPositions();
    vector<uint16_t> positions_packed(positions.size() * 3);
    for (size_t i = 0; i < positions.size(); ++i) {
        Engine::Math::Float16From32(&positions_packed[(i * 3) + 0], positions[i].x);
        Engine::Math::Float16From32(&positions_packed[(i * 3) + 1], positions[i].y);
        Engine::Math::Float16From32(&positions_packed[(i * 3) + 2], positions[i].z);
    }
    positions_attr.setBuffer(reinterpret_cast<const uint8_t*>(positions_packed.data()), (unsigned int)(positions_packed.size() * 2));

    //uvs - 2 half floats
    SMSH_Attribute uvs_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_2 };
    uvs_attr.m_SizeOfAttribute = sizeof(uint16_t) * 2;
    uvs_attr.m_Offset          = positions_attr.m_SizeOfAttribute;
    uvs_attr.m_Normalized      = false;
    auto uvs                   = vertexData.getData<glm::vec2>(1);
    vector<uint16_t> uvs_packed(uvs.size() * 2);
    for (size_t i = 0; i < uvs.size(); ++i) {
        Engine::Math::Float16From32(&uvs_packed[(i * 2) + 0], uvs[i].x);
        Engine::Math::Float16From32(&uvs_packed[(i * 2) + 1], uvs[i].y);
    }
    uvs_attr.setBuffer(reinterpret_cast<const uint8_t*>(uvs_packed.data()), (unsigned int)(uvs_packed.size() * 2));

    //normals - 1 packed unsigned 32 int
    SMSH_Attribute normals_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    normals_attr.m_SizeOfAttribute = sizeof(uint32_t);
    normals_attr.m_Offset          = uvs_attr.m_SizeOfAttribute + uvs_attr.m_Offset;
    normals_attr.m_Normalized      = true;
    auto normals                   = vertexData.getData<uint32_t>(2);
    normals_attr.setBuffer(reinterpret_cast<const uint8_t*>(normals.data()), (unsigned int)(normals.size() * normals_attr.m_SizeOfAttribute));

    //binormals - 1 packed unsigned 32 int
    SMSH_Attribute binormals_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    binormals_attr.m_SizeOfAttribute = sizeof(uint32_t);
    binormals_attr.m_Offset          = normals_attr.m_Offset + normals_attr.m_SizeOfAttribute;
    binormals_attr.m_Normalized      = true; 
    auto binormals                   = vertexData.getData<uint32_t>(3);
    binormals_attr.setBuffer(reinterpret_cast<const uint8_t*>(binormals.data()), (unsigned int)(binormals.size() * binormals_attr.m_SizeOfAttribute));

    //tangents - 1 packed unsigned 32 int
    SMSH_Attribute tangents_attr{ SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA };
    tangents_attr.m_SizeOfAttribute = sizeof(uint32_t);
    tangents_attr.m_Offset          = binormals_attr.m_Offset + binormals_attr.m_SizeOfAttribute;
    tangents_attr.m_Normalized      = true;
    auto tangents                   = vertexData.getData<uint32_t>(4);
    tangents_attr.setBuffer(reinterpret_cast<const uint8_t*>(tangents.data()), (unsigned int)(tangents.size() * tangents_attr.m_SizeOfAttribute));

    //boneids - 4 half floats
    SMSH_Attribute boneids_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4 };
    boneids_attr.m_SizeOfAttribute = sizeof(uint16_t) * 4;
    boneids_attr.m_Offset          = tangents_attr.m_Offset + tangents_attr.m_SizeOfAttribute;
    boneids_attr.m_Normalized      = false;
    auto boneids                   = vertexData.getData<glm::vec4>(5);
    vector<uint16_t> boneids_packed(boneids.size() * 4);
    for (size_t i = 0; i < boneids.size(); ++i) {
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 0], boneids[i].x);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 1], boneids[i].y);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 2], boneids[i].z);
        Engine::Math::Float16From32(&boneids_packed[(i * 4) + 3], boneids[i].w);
    }
    boneids_attr.setBuffer(reinterpret_cast<const uint8_t*>(boneids_packed.data()), (unsigned int)(boneids_packed.size() * 2));

    //boneweights - 4 half floats
    SMSH_Attribute boneweights_attr{ SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4 };
    boneweights_attr.m_SizeOfAttribute = sizeof(uint16_t) * 4;
    boneweights_attr.m_Offset          = boneids_attr.m_Offset + boneids_attr.m_SizeOfAttribute;
    boneweights_attr.m_Normalized      = false;
    auto boneweights                   = vertexData.getData<glm::vec4>(6);
    vector<uint16_t> boneweights_packed(boneweights.size()*4);
    for (size_t i = 0; i < boneweights.size(); ++i) {
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 0], boneweights[i].x);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 1], boneweights[i].y);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 2], boneweights[i].z);
        Engine::Math::Float16From32(&boneweights_packed[(i * 4) + 3], boneweights[i].w);
    }
    boneweights_attr.setBuffer(reinterpret_cast<const uint8_t*>(boneweights_packed.data()), (unsigned int)(boneweights_packed.size() * 2));

    //strides
    unsigned int total_stride =
        ((positions_attr.m_AttributeBufferSize > 0) ?   positions_attr.m_SizeOfAttribute : 0) +
        ((uvs_attr.m_AttributeBufferSize > 0) ?         uvs_attr.m_SizeOfAttribute : 0) +
        ((normals_attr.m_AttributeBufferSize > 0) ?     normals_attr.m_SizeOfAttribute : 0) +
        ((binormals_attr.m_AttributeBufferSize > 0) ?   binormals_attr.m_SizeOfAttribute : 0) +
        ((tangents_attr.m_AttributeBufferSize > 0) ?    tangents_attr.m_SizeOfAttribute : 0) +
        ((boneids_attr.m_AttributeBufferSize > 0) ?     boneids_attr.m_SizeOfAttribute : 0) +
        ((boneweights_attr.m_AttributeBufferSize > 0) ? boneweights_attr.m_SizeOfAttribute : 0);

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
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                uint8_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 1U);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                uint16_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 2U);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 4U);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 4U);
            }
            break;
        }
    }
    //animation data
    if (smsh_header.m_NumberOfBones > 0U) {
        auto lamda_write_mat4_as_half_floats = [](std::ofstream& instream, const glm::mat4& inMatrix) {
            for (unsigned int i = 0U; i < 4U; ++i) {
                for (unsigned int j = 0U; j < 4U; ++j) {
                    uint16_t half_float;
                    Engine::Math::Float16From32(&half_float, inMatrix[i][j]);
                    writeBigEndian(instream, half_float, 2U);
                }
            }
        };
        auto lamda_write_string = [](std::ofstream& instream, const std::string& inString) {
            uint16_t str_len = static_cast<uint16_t>(inString.length());
            writeBigEndian(instream, str_len);
            for (size_t i = 0; i < str_len; ++i) {
                writeBigEndian(instream, inString[i], 1U);
            }
        };

        //global inverse transform
        lamda_write_mat4_as_half_floats(stream, skeleton->m_GlobalInverseTransform);
        //bone mapping (string key to unsigned int)
        for (const auto& itr : skeleton->m_BoneMapping) {
            writeBigEndian(stream, itr.second, 4U);
            lamda_write_string(stream, itr.first);
        }
        //bone info vector
        for (const auto& info : skeleton->m_BoneInfo) {
            //bone offset
            lamda_write_mat4_as_half_floats(stream, info.BoneOffset);
            //final transform
            lamda_write_mat4_as_half_floats(stream, info.FinalTransform);
        }
        //animation data
        writeBigEndian(stream, static_cast<uint32_t>(skeleton->m_AnimationData.size()));
        for (const auto& itr : skeleton->m_AnimationData) {
            uint16_t half_float_ticks_per_second;
            uint16_t half_float_duration_in_ticks;
            Engine::Math::Float16From32(&half_float_ticks_per_second, itr.second.m_TicksPerSecond);
            Engine::Math::Float16From32(&half_float_duration_in_ticks, itr.second.m_DurationInTicks);
            writeBigEndian(stream, half_float_ticks_per_second, 2U);
            writeBigEndian(stream, half_float_duration_in_ticks, 2U);
            lamda_write_string(stream, itr.first);
            uint32_t keyframeSize = static_cast<uint32_t>(itr.second.m_KeyframeData.size());
            writeBigEndian(stream, keyframeSize);
            for (const auto& keyframeData : itr.second.m_KeyframeData) {
                uint32_t pos_size     = static_cast<uint32_t>(keyframeData.second.PositionKeys.size());
                uint32_t rot_size     = static_cast<uint32_t>(keyframeData.second.RotationKeys.size());
                uint32_t scl_size     = static_cast<uint32_t>(keyframeData.second.ScalingKeys.size());

                writeBigEndian(stream, pos_size, 4U);
                writeBigEndian(stream, rot_size, 4U);
                writeBigEndian(stream, scl_size, 4U);
                lamda_write_string(stream, keyframeData.first);
                auto lamda_write_vec3 = [](std::ofstream& instream, const Engine::priv::Vector3Key& key) {
                    uint16_t vec_half[3];
                    for (int i = 0; i < 3; ++i) {
                        Engine::Math::Float16From32(&vec_half[i], key.value[i]);
                        writeBigEndian(instream, vec_half[i]);
                    }
                    uint16_t time_half;
                    Engine::Math::Float16From32(&time_half, key.time);
                    writeBigEndian(instream, time_half, 2U);
                };

                for (const auto& posKey : keyframeData.second.PositionKeys) {
                    lamda_write_vec3(stream, posKey);
                }
                for (const auto& rotKey : keyframeData.second.RotationKeys) {
                    uint16_t quat_half[4];

                    Engine::Math::Float16From32(&quat_half[0], rotKey.value.x);
                    writeBigEndian(stream, quat_half[0], 2U);

                    Engine::Math::Float16From32(&quat_half[1], rotKey.value.y);
                    writeBigEndian(stream, quat_half[1], 2U);

                    Engine::Math::Float16From32(&quat_half[2], rotKey.value.z);
                    writeBigEndian(stream, quat_half[2], 2U);

                    Engine::Math::Float16From32(&quat_half[3], rotKey.value.w);
                    writeBigEndian(stream, quat_half[3], 2U);

                    uint16_t time_half;
                    Engine::Math::Float16From32(&time_half, rotKey.time);
                    writeBigEndian(stream, time_half, 2U);
                }
                for (const auto& sclKey : keyframeData.second.ScalingKeys) {
                    lamda_write_vec3(stream, sclKey);
                }
            }
        }
        //node heirarchy
        std::vector<Engine::priv::MeshInfoNode*> sortedNodes;
        std::queue<Engine::priv::MeshInfoNode*> q;
        q.push(mesh.m_RootNode);
        while (!q.empty()) {
            auto size = q.size();
            while (size > 0) {
                auto* front = q.front();
                sortedNodes.push_back(front);
                for (const auto& child : front->Children) {
                    q.push(child);
                }
                q.pop();
                --size;
            }
        }
        auto sorter = [](Engine::priv::MeshInfoNode* a, Engine::priv::MeshInfoNode* b) {
            return a->Name < b->Name;
        };
        std::sort(sortedNodes.begin(), sortedNodes.end(), sorter);
        std::unordered_map<std::string, uint32_t> nameMap; //string name => index in sorted vector
        nameMap.reserve(sortedNodes.size());
        for (size_t i = 0; i < sortedNodes.size(); ++i) {
            nameMap.emplace(sortedNodes[i]->Name, static_cast<uint32_t>(i));
        }
        std::vector<std::vector<uint32_t>> relationships;
        relationships.resize(sortedNodes.size());
        for (size_t i = 0; i < sortedNodes.size(); ++i) {
            auto* node = sortedNodes[i];
            for (size_t j = 0; j < node->Children.size(); ++j) {
                auto* child = node->Children[j];
                relationships[i].push_back(nameMap.at(child->Name));
            }
        }
        uint32_t number_of_nodes = static_cast<uint32_t>(sortedNodes.size());
        writeBigEndian(stream, number_of_nodes, 4U);
        for (const auto& node : sortedNodes) {
            lamda_write_mat4_as_half_floats(stream, node->Transform);
            lamda_write_string(stream, node->Name);
        }
        for (size_t i = 0; i < relationships.size(); ++i) {
            uint32_t number_of_children = static_cast<uint32_t>(relationships[i].size());
            writeBigEndian(stream, number_of_children, 4U);
            for (size_t j = 0; j < number_of_children; ++j) {
                writeBigEndian(stream, relationships[i][j], 4U);
            }
        }
    }
    stream.close();
}