
#include <serenity/resources/mesh/smsh.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/animation/Skeleton.h>
#include <serenity/system/TypeDefs.h>
#include <fstream>
#include <boost/iostreams/device/mapped_file.hpp>


SMSH_Fileheader::SMSH_Fileheader(MeshCPUData& cpuData) {
    auto* vertexData   = cpuData.m_VertexData;
    auto* skeleton     = cpuData.m_Skeleton;
    m_InterleavingType = SMSH_InterleavingType::Interleaved;
    m_AttributeCount   = uint32_t(vertexData->m_Format.getAttributes().size());
    m_IndiceCount      = uint32_t(vertexData->m_Indices.size());

    if (m_IndiceCount <= 255) {
        m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Byte;
    } else if (m_IndiceCount >= 256 && m_IndiceCount <= 65535) {
        m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Short;
    } else {
        m_IndiceDataTypeSize = SMSH_IndiceDataType::Unsigned_Int;
    }
    m_UserDataCount    = 2;
    m_NumberOfBones    = (skeleton) ? uint32_t(skeleton->numBones()) : 0;
}

void SMSH_File::LoadFile(const char* filename, MeshCPUData& cpuData) {
    boost::iostreams::mapped_file_source stream{ filename };
    uint32_t blockStart = 0;
    const uint8_t* streamDataBuffer = (uint8_t*)stream.data();

    SMSH_Fileheader smsh_header{ stream, blockStart };

    std::vector<uint32_t> userData{ smsh_header.m_UserDataCount, 0 };
    for (size_t i = 0; i != smsh_header.m_UserDataCount; ++i) {
        Engine::readBigEndian(userData[i], streamDataBuffer, 4, blockStart);
    }
    VertexDataFormat vertexDataFormat{ (VertexAttributeLayout)smsh_header.m_InterleavingType };
    auto attr_data = Engine::create_and_reserve<std::vector<std::tuple<SMSH_AttributeNoBuffer, uint8_t*, uint32_t>>>(smsh_header.m_AttributeCount);
    for (size_t i = 0; i != smsh_header.m_AttributeCount; ++i) {
        auto& [attr, buff, idx] = attr_data.emplace_back(SMSH_AttributeNoBuffer{}, nullptr, 0U);

        Engine::readBigEndian(attr.m_Normalized,              streamDataBuffer, 1, blockStart);
        Engine::readBigEndian(attr.m_Stride,                  streamDataBuffer, 4, blockStart);
        Engine::readBigEndian(attr.m_AttributeType,           streamDataBuffer, 4, blockStart);
        Engine::readBigEndian(attr.m_SizeOfAttribute,         streamDataBuffer, 4, blockStart);         //(example, 96 for 3 float vector, 32 for 2 half float vector, 32 for 1 packed unsigned int normal, etc)
        Engine::readBigEndian(attr.m_AttributeComponentCount, streamDataBuffer, 4, blockStart); //(3 for x,y,z | 4 for rgba, 2 for uv, etc)
        Engine::readBigEndian(attr.m_AttributeBufferSize,     streamDataBuffer, 4, blockStart);     //size of the whole data buffer in bytes
        Engine::readBigEndian(attr.m_Offset,                  streamDataBuffer, 4, blockStart);

        buff = NEW uint8_t[attr.m_AttributeBufferSize];
        std::memcpy(&buff[0], &streamDataBuffer[blockStart], attr.m_AttributeBufferSize);
        blockStart += attr.m_AttributeBufferSize;

        vertexDataFormat.add(attr.m_AttributeComponentCount, attr.m_AttributeType, bool(attr.m_Normalized), attr.m_Stride, attr.m_Offset, attr.m_SizeOfAttribute);
    }
    cpuData.m_File = filename;
    SAFE_DELETE(cpuData.m_VertexData);
    cpuData.m_VertexData = NEW VertexData(vertexDataFormat);

    for (size_t i = 0; i != smsh_header.m_AttributeCount; ++i) {
        auto& [attr, buff, idx] = attr_data[i];
        cpuData.m_VertexData->setData(i, buff, attr.m_AttributeBufferSize, attr.m_AttributeBufferSize / attr.m_SizeOfAttribute, MeshModifyFlags::None);
        delete[] buff;
    }
    //indices
    auto indices = Engine::create_and_reserve<std::vector<uint32_t>>(smsh_header.m_IndiceCount);
    switch (smsh_header.m_IndiceDataTypeSize) {
        case SMSH_IndiceDataType::Unsigned_Byte: {
            for (size_t i = 0; i != smsh_header.m_IndiceCount; ++i) {
                uint8_t indice;
                Engine::readBigEndian(indice, streamDataBuffer, 1, blockStart);
                indices.emplace_back(uint32_t(indice));
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i != smsh_header.m_IndiceCount; ++i) {
                uint16_t indice;
                Engine::readBigEndian(indice, streamDataBuffer, 2, blockStart);
                indices.emplace_back(uint32_t(indice));
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i != smsh_header.m_IndiceCount; ++i) {
                uint32_t indice;
                Engine::readBigEndian(indice, streamDataBuffer, 4, blockStart);
                indices.emplace_back(indice);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i != smsh_header.m_IndiceCount; ++i) {
                uint32_t indice;
                Engine::readBigEndian(indice, streamDataBuffer, 4, blockStart);
                indices.emplace_back(indice);
            }
            break;
        }
    }
    cpuData.m_VertexData->setIndices(indices.data(), indices.size(), MeshModifyFlags::RecalculateTriangles);

    //animation data
    if (smsh_header.m_NumberOfBones > 0) {
        auto lamda_read_mat4_as_half_floats = [&blockStart, &streamDataBuffer](glm::mat4& inMatrix) {
            for (uint32_t i = 0; i != 4; ++i) {
                for (uint32_t j = 0; j != 4; ++j) {
                    uint16_t half_float;
                    Engine::readBigEndian(half_float, streamDataBuffer, 2, blockStart);
                    inMatrix[i][j] = Engine::Math::Float32From16(half_float);
                }
            }
        };
        auto lambda_read_keyframe_component = [&blockStart, &streamDataBuffer]<typename KEYFRAME, size_t size>(KEYFRAME& keyframe) {
            using len_type = decltype(KEYFRAME::value)::length_type;
            uint16_t data[size];
            for (size_t i = 0; i != size; ++i) {
                Engine::readBigEndian(data[i], streamDataBuffer, 2, blockStart);
                keyframe.value[len_type(i)] = Engine::Math::Float32From16(data[i]);
            }
            uint16_t time;
            Engine::readBigEndian(time, streamDataBuffer, 2, blockStart);
            keyframe.time = Engine::Math::Float32From16(time);
        };
        SAFE_DELETE(cpuData.m_Skeleton);
        cpuData.m_Skeleton = NEW Engine::priv::MeshSkeleton{};
        cpuData.m_Skeleton->m_BoneOffsets.resize(smsh_header.m_NumberOfBones);
        //global inverse transform (glm::mat4 stored as half floats)
        lamda_read_mat4_as_half_floats(cpuData.m_Skeleton->m_GlobalInverseTransform);
        //bone offset vector
        for (size_t i = 0; i != smsh_header.m_NumberOfBones; ++i) {
            auto& boneOffset = cpuData.m_Skeleton->m_BoneOffsets[i];
            lamda_read_mat4_as_half_floats(boneOffset);
        }
        //animation data
        uint32_t animCount;
        Engine::readBigEndian(animCount, streamDataBuffer, 4, blockStart);
        for (uint32_t i = 0; i != animCount; ++i) {
            uint16_t half_float_ticks_per_second;
            uint16_t half_float_duration_in_ticks;
            uint16_t str_len;
            Engine::readBigEndian(half_float_ticks_per_second, streamDataBuffer, 2, blockStart);
            Engine::readBigEndian(half_float_duration_in_ticks, streamDataBuffer, 2, blockStart);
            Engine::readBigEndian(str_len, streamDataBuffer, 2, blockStart);
            auto anim_name = Engine::create_and_reserve<std::string>(str_len);
            for (size_t j = 0; j != str_len; ++j) {
                char anim_name_char;
                Engine::readBigEndian(anim_name_char, streamDataBuffer, 1, blockStart);
                anim_name += anim_name_char;
            }
            uint32_t channelCount;
            Engine::readBigEndian(channelCount, streamDataBuffer, 4, blockStart);
            float ticksPerSec = Engine::Math::Float32From16(half_float_ticks_per_second);
            float durInTicks = Engine::Math::Float32From16(half_float_duration_in_ticks);
            Engine::priv::AnimationData animationData{ ticksPerSec, durInTicks };
            animationData.m_Channels.reserve(channelCount);
            for (size_t j = 0; j != channelCount; ++j) {
                uint32_t pos_count, rot_count, scl_count;
                Engine::readBigEndian(pos_count, streamDataBuffer, 4, blockStart);
                Engine::readBigEndian(rot_count, streamDataBuffer, 4, blockStart);
                Engine::readBigEndian(scl_count, streamDataBuffer, 4, blockStart);
                Engine::priv::AnimationChannel channel;
                for (size_t k = 0; k != pos_count; ++k) {
                    auto& posKey = channel.PositionKeys.emplace_back();
                    lambda_read_keyframe_component.operator()<Engine::priv::Vector3Key, 3>(posKey);
                }
                for (size_t k = 0; k != rot_count; ++k) {
                    auto& rotKey = channel.RotationKeys.emplace_back();
                    lambda_read_keyframe_component.operator()<Engine::priv::QuatKey, 4>(rotKey);
                }
                for (size_t k = 0; k != scl_count; ++k) {
                    auto& sclKey = channel.ScalingKeys.emplace_back();
                    lambda_read_keyframe_component.operator()<Engine::priv::Vector3Key, 3>(sclKey);
                }
                animationData.m_Channels.emplace_back(std::move(channel));
            }
            const auto animIndex = cpuData.m_Skeleton->addAnimation(anim_name, std::move(animationData));
        }
        uint32_t num_of_nodes = 0;
        Engine::readBigEndian(num_of_nodes, streamDataBuffer, 4, blockStart);
        cpuData.m_NodesData.m_NodeHeirarchy.reserve(num_of_nodes);
        cpuData.m_NodesData.m_NodeTransforms.resize(num_of_nodes, glm::mat4{ 1.0f });
        cpuData.m_NodesData.m_Nodes.reserve(num_of_nodes);
        for (uint32_t i = 0; i != num_of_nodes; ++i) {
            uint16_t parent = 0;
            Engine::readBigEndian(parent, streamDataBuffer, 2, blockStart);
            cpuData.m_NodesData.m_NodeHeirarchy.push_back(parent);
        }
        for (uint32_t i = 0; i != num_of_nodes; ++i) {
            glm::mat4  node_transform{ 1.0f };
            uint8_t    isBone = 0;
            lamda_read_mat4_as_half_floats(node_transform);
            Engine::readBigEndian(isBone, streamDataBuffer, 1, blockStart);
            auto& node  = cpuData.m_NodesData.m_Nodes.emplace_back(std::move(node_transform));
            node.IsBone = static_cast<bool>(isBone);
        }
    }
}
void SMSH_File::SaveFile(const char* filename, MeshCPUData& cpuData) {
    auto& vertexData = *cpuData.m_VertexData;
    auto* skeleton   = cpuData.m_Skeleton;
    SMSH_Fileheader smsh_header{ cpuData };

    std::ofstream stream{ filename, std::ios::binary | std::ios::out };

    Engine::writeBigEndian(stream, smsh_header.m_InterleavingType);
    Engine::writeBigEndian(stream, smsh_header.m_AttributeCount);
    Engine::writeBigEndian(stream, smsh_header.m_IndiceCount);
    Engine::writeBigEndian(stream, smsh_header.m_IndiceDataTypeSize);
    Engine::writeBigEndian(stream, smsh_header.m_UserDataCount);
    Engine::writeBigEndian(stream, smsh_header.m_NumberOfBones);

    for (uint32_t i = 0; i != smsh_header.m_UserDataCount; ++i) {
        uint32_t userData = 0; //replace the Zero with whatever userdata you want
        Engine::writeBigEndian(stream, userData);
    }

#pragma region Positions
    SMSH_Attribute positions_attr { 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_3, 0, false, 0, sizeof(uint16_t) * 3, 0 
    };
    auto positions = vertexData.getPositions();
    std::vector<uint16_t> positions_packed(positions.size() * 3);
    for (size_t i = 0; i != positions.size(); ++i) {
        for (size_t j = 0; j != 3; ++j) {
            using len_type = glm::vec3::length_type;
            positions_packed[(i * 3) + j] = Engine::Math::Float16From32(positions[i][len_type(j)]);
        }
    }
    positions_attr.setBuffer(reinterpret_cast<const uint8_t*>(positions_packed.data()), uint32_t(positions_packed.size() * 2));
#pragma endregion

#pragma region UVs
    SMSH_Attribute uvs_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_2, positions_attr.m_SizeOfAttribute, false, 0, sizeof(uint16_t) * 2, 0 
    };
    auto uvs = vertexData.getData<glm::vec2>(1);
    std::vector<uint16_t> uvs_packed(uvs.size() * 2);
    for (size_t i = 0; i != uvs.size(); ++i) {
        for (size_t j = 0; j != 2; ++j) {
            using len_type = glm::vec2::length_type;
            uvs_packed[(i * 2) + j] = Engine::Math::Float16From32(uvs[i][len_type(j)]);
        }
    }
    uvs_attr.setBuffer(reinterpret_cast<const uint8_t*>(uvs_packed.data()), uint32_t(uvs_packed.size() * 2));
#pragma endregion

#pragma region Normals
    SMSH_Attribute normals_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::_4, uvs_attr.m_SizeOfAttribute + uvs_attr.m_Offset, true, 0, sizeof(uint32_t), 0
    };
    auto normals = vertexData.getData<uint32_t>(2);
    normals_attr.setBuffer(reinterpret_cast<const uint8_t*>(normals.data()), uint32_t(normals.size() * normals_attr.m_SizeOfAttribute));
#pragma endregion

#pragma region Binormals
    SMSH_Attribute binormals_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::_4, normals_attr.m_Offset + normals_attr.m_SizeOfAttribute, true, 0, sizeof(uint32_t), 0
    };
    auto binormals = vertexData.getData<uint32_t>(3);
    binormals_attr.setBuffer(reinterpret_cast<const uint8_t*>(binormals.data()), uint32_t(binormals.size() * binormals_attr.m_SizeOfAttribute));
#pragma endregion

#pragma region Tangents
    SMSH_Attribute tangents_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::_4, binormals_attr.m_Offset + binormals_attr.m_SizeOfAttribute, true, 0, sizeof(uint32_t), 0
    };
    auto tangents = vertexData.getData<uint32_t>(4);
    tangents_attr.setBuffer(reinterpret_cast<const uint8_t*>(tangents.data()), uint32_t(tangents.size() * tangents_attr.m_SizeOfAttribute));
#pragma endregion

#pragma region BoneIDs
    SMSH_Attribute boneids_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4, tangents_attr.m_Offset + tangents_attr.m_SizeOfAttribute, false, 0, sizeof(uint16_t) * 4, 0
    };
    auto boneids = vertexData.getData<glm::vec4>(5);
    std::vector<uint16_t> boneids_packed(boneids.size() * 4);
    for (unsigned i = 0; i != boneids.size(); ++i) {
        for (unsigned j = 0; j != 4; ++j) {
            boneids_packed[(i * 4) + j] = Engine::Math::Float16From32(boneids[i][j]);
        }
    }
    boneids_attr.setBuffer(reinterpret_cast<const uint8_t*>(boneids_packed.data()), uint32_t(boneids_packed.size() * 2));
#pragma endregion

#pragma region BoneWeights
    SMSH_Attribute boneweights_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4, boneids_attr.m_Offset + boneids_attr.m_SizeOfAttribute, false, 0, sizeof(uint16_t) * 4, 0
    };
    auto boneweights = vertexData.getData<glm::vec4>(6);
    std::vector<uint16_t> boneweights_packed(boneweights.size()*4);
    for (size_t i = 0; i != boneweights.size(); ++i) {
        for (size_t j = 0; j != 4; ++j) {
            using len_type = glm::vec4::length_type;
            boneweights_packed[(i * 4) + j] = Engine::Math::Float16From32(boneweights[i][len_type(j)]);
        }
    }
    boneweights_attr.setBuffer(reinterpret_cast<const uint8_t*>(boneweights_packed.data()), uint32_t(boneweights_packed.size() * 2));
#pragma endregion

    //strides
    uint32_t total_stride =
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
            for (size_t i = 0; i != vertexData.m_Indices.size(); ++i) {
                uint8_t indice = vertexData.m_Indices[i];
                Engine::writeBigEndian(stream, indice, 1);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i != vertexData.m_Indices.size(); ++i) {
                uint16_t indice = vertexData.m_Indices[i];
                Engine::writeBigEndian(stream, indice, 2);
            }
            break;
        }case SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i != vertexData.m_Indices.size(); ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                Engine::writeBigEndian(stream, indice, 4);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i != vertexData.m_Indices.size(); ++i) {
                uint32_t indice = vertexData.m_Indices[i];
                Engine::writeBigEndian(stream, indice, 4);
            }
            break;
        }
    }
   
    //animation data
    if (smsh_header.m_NumberOfBones > 0) {
        auto lamda_write_mat4_as_half_floats = [](std::ofstream& instream, const glm::mat4& inMatrix) {
            for (uint32_t i = 0; i != 4; ++i) {
                for (uint32_t j = 0; j != 4; ++j) {
                    uint16_t half_float = Engine::Math::Float16From32(inMatrix[i][j]);
                    Engine::writeBigEndian(instream, half_float, 2);
                }
            }
        };
        auto lamda_write_string = [](std::ofstream& instream, std::string_view inString) {
            uint16_t str_len = uint16_t(inString.length());
            Engine::writeBigEndian(instream, str_len);
            for (size_t i = 0; i != str_len; ++i) {
                Engine::writeBigEndian(instream, inString[i], 1);
            }
        };
        auto lamda_write_keyframe_component = []<typename KEYFRAME, size_t size>(std::ofstream& instream, const KEYFRAME& key) {
            std::array<uint16_t, size> vec_half;
            for (int i = 0; i != size; ++i) {
                vec_half[i] = Engine::Math::Float16From32(key.value[i]);
                Engine::writeBigEndian(instream, vec_half[i]);
            }
            uint16_t time_half = Engine::Math::Float16From32(key.time);
            Engine::writeBigEndian(instream, time_half, 2);
        };

        //global inverse transform
        lamda_write_mat4_as_half_floats(stream, skeleton->m_GlobalInverseTransform);

        //bone offset vector
        for (const auto& boneOffset : skeleton->m_BoneOffsets) {
            lamda_write_mat4_as_half_floats(stream, boneOffset);
        }
        //animation data
        Engine::writeBigEndian(stream, uint32_t(skeleton->m_AnimationData.size()));
        for (const auto& [animName, animationIndex] : skeleton->m_AnimationMapping) {
            const auto& animationData = skeleton->m_AnimationData[animationIndex];
            uint16_t half_float_ticks_per_second = Engine::Math::Float16From32(animationData.m_TicksPerSecond);
            uint16_t half_float_duration_in_ticks = Engine::Math::Float16From32(animationData.m_DurationInTicks);
            Engine::writeBigEndian(stream, half_float_ticks_per_second, 2);
            Engine::writeBigEndian(stream, half_float_duration_in_ticks, 2);
            lamda_write_string(stream, animName);
            uint32_t channelSize = uint32_t(animationData.m_Channels.size());
            Engine::writeBigEndian(stream, channelSize);
            for (const auto& channel : animationData.m_Channels) {
                uint32_t pos_size = uint32_t(channel.PositionKeys.size());
                uint32_t rot_size = uint32_t(channel.RotationKeys.size());
                uint32_t scl_size = uint32_t(channel.ScalingKeys.size());
                Engine::writeBigEndian(stream, pos_size, 4);
                Engine::writeBigEndian(stream, rot_size, 4);
                Engine::writeBigEndian(stream, scl_size, 4);
                for (const auto& posKey : channel.PositionKeys) {
                    lamda_write_keyframe_component.operator()<Engine::priv::Vector3Key, 3>(stream, posKey);
                }
                for (const auto& rotKey : channel.RotationKeys) {
                    lamda_write_keyframe_component.operator()<Engine::priv::QuatKey, 4>(stream, rotKey);
                }
                for (const auto& sclKey : channel.ScalingKeys) {
                    lamda_write_keyframe_component.operator()<Engine::priv::Vector3Key, 3>(stream, sclKey);
                }
            }
        }
        //node heirarchy
        uint32_t numNodes = uint32_t(cpuData.m_NodesData.m_Nodes.size());
        Engine::writeBigEndian(stream, numNodes);
        for (uint32_t i = 0; i != numNodes; ++i) {
            uint16_t parent = cpuData.m_NodesData.m_NodeHeirarchy[i];
            Engine::writeBigEndian(stream, parent);
        }
        for (uint32_t i = 0; i != numNodes; ++i) {
            lamda_write_mat4_as_half_floats(stream, cpuData.m_NodesData.m_Nodes[i].Transform);
            uint8_t isBone = uint8_t(cpuData.m_NodesData.m_Nodes[i].IsBone);
            Engine::writeBigEndian(stream, isBone, 1);
        }
    }
    stream.close();
}