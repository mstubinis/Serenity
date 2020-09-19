#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/smsh.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/Skeleton.h>

#include <boost/iostreams/device/mapped_file.hpp>


SMSH_Fileheader::SMSH_Fileheader(Mesh& mesh) {
    auto& vertexData   = mesh.getVertexData();
    auto* skeleton     = mesh.getSkeleton();
    m_InterleavingType = (unsigned char)SMSH_InterleavingType::Interleaved;
    m_AttributeCount   = (unsigned int)vertexData.m_Format.m_Attributes.size();
    m_IndiceCount      = (unsigned int)vertexData.m_Indices.size();

    if (m_IndiceCount <= 255) {
        m_IndiceDataTypeSize = (unsigned int)SMSH_IndiceDataType::Unsigned_Byte;
    }else if (m_IndiceCount >= 256 && m_IndiceCount <= 65535) {
        m_IndiceDataTypeSize = (unsigned int)SMSH_IndiceDataType::Unsigned_Short;
    }else {
        m_IndiceDataTypeSize = (unsigned int)SMSH_IndiceDataType::Unsigned_Int;
    }
    m_UserDataCount    = 2U;
    m_NumberOfBones    = (skeleton) ? skeleton->numBones() : 0U;

}

void SMSH_File::LoadFile(Mesh* mesh, const char* filename) {
    boost::iostreams::mapped_file_source stream(filename);
    SMSH_Fileheader smsh_header;

    std::uint32_t blockStart = 0;
    const std::uint8_t* streamDataBuffer = (std::uint8_t*)stream.data();

    readBigEndian(smsh_header.m_InterleavingType,   streamDataBuffer, 1, blockStart);
    readBigEndian(smsh_header.m_AttributeCount,     streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceCount,        streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_IndiceDataTypeSize, streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_UserDataCount,      streamDataBuffer, 4, blockStart);
    readBigEndian(smsh_header.m_NumberOfBones,      streamDataBuffer, 4, blockStart);

    std::vector<unsigned int> userData(smsh_header.m_UserDataCount, 0);
    for (size_t i = 0; i < smsh_header.m_UserDataCount; ++i) {
        readBigEndian(userData[i], streamDataBuffer, 4, blockStart);
    }
    VertexDataFormat vertexDataFormat;
    vertexDataFormat.m_InterleavingType = (VertexAttributeLayout)smsh_header.m_InterleavingType;

    std::vector<std::tuple<SMSH_AttributeNoBuffer, std::uint8_t*, unsigned int>> attr_data;
    attr_data.reserve(smsh_header.m_AttributeCount);
    for (size_t i = 0; i < smsh_header.m_AttributeCount; ++i) {
        auto& [attr, buff, idx] = attr_data.emplace_back(SMSH_AttributeNoBuffer{}, nullptr, 0U);

        readBigEndian(attr.m_Normalized,              streamDataBuffer, 1, blockStart);
        readBigEndian(attr.m_Stride,                  streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_AttributeType,           streamDataBuffer, 4, blockStart);
        readBigEndian(attr.m_SizeOfAttribute,         streamDataBuffer, 4, blockStart);         //(example, 96 for 3 float vector, 32 for 2 half float vector, 32 for 1 packed unsigned int normal, etc)
        readBigEndian(attr.m_AttributeComponentCount, streamDataBuffer, 4, blockStart); //(3 for x,y,z | 4 for rgba, 2 for uv, etc)
        readBigEndian(attr.m_AttributeBufferSize,     streamDataBuffer, 4, blockStart);     //size of the whole data buffer
        readBigEndian(attr.m_Offset,                  streamDataBuffer, 4, blockStart);

        buff = NEW std::uint8_t[attr.m_AttributeBufferSize];
        std::memcpy(&buff[0], &streamDataBuffer[blockStart], attr.m_AttributeBufferSize);
        blockStart += attr.m_AttributeBufferSize;

        vertexDataFormat.add(attr.m_AttributeComponentCount, attr.m_AttributeType, (bool)attr.m_Normalized, attr.m_Stride, attr.m_Offset, attr.m_SizeOfAttribute);
    }
    mesh->m_File = filename;
    mesh->m_VertexData = NEW VertexData(vertexDataFormat);

    for (size_t i = 0; i < smsh_header.m_AttributeCount; ++i) {
        auto& [attr, buff, idx] = attr_data[i];
        mesh->m_VertexData->setData(i, buff, attr.m_AttributeBufferSize, attr.m_AttributeBufferSize / attr.m_SizeOfAttribute, false, false);
        delete[] buff;
    }
    //indices
    std::vector<unsigned int> indices;
    indices.reserve(smsh_header.m_IndiceCount);
    switch (smsh_header.m_IndiceDataTypeSize) {
        case (unsigned int)SMSH_IndiceDataType::Unsigned_Byte: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                std::uint8_t indice;
                readBigEndian(indice, streamDataBuffer, 1U, blockStart);
                indices.emplace_back((unsigned int)indice);
            }
            break;
        }case (unsigned int)SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                std::uint16_t indice;
                readBigEndian(indice, streamDataBuffer, 2U, blockStart);
                indices.emplace_back((unsigned int)indice);
            }
            break;
        }case (unsigned int)SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                std::uint32_t indice;
                readBigEndian(indice, streamDataBuffer, 4U, blockStart);
                indices.emplace_back(indice);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i < smsh_header.m_IndiceCount; ++i) {
                std::uint32_t indice;
                readBigEndian(indice, streamDataBuffer, 4U, blockStart);
                indices.emplace_back(indice);
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
                    std::uint16_t half_float;
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
            std::uint32_t map_value;
            std::uint16_t str_len;
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
        std::uint32_t animCount;
        readBigEndian(animCount, streamDataBuffer, 4U, blockStart);
        for (std::uint32_t i = 0; i < animCount; ++i) {
            //auto x = ret.skeleton->m_AnimationData.emplace_back();
            std::uint16_t half_float_ticks_per_second;
            std::uint16_t half_float_duration_in_ticks;
            std::uint16_t str_len;
            std::string anim_name = "";
            readBigEndian(half_float_ticks_per_second, streamDataBuffer, 2U, blockStart);
            readBigEndian(half_float_duration_in_ticks, streamDataBuffer, 2U, blockStart);
            readBigEndian(str_len, streamDataBuffer, 2U, blockStart);
            for (size_t j = 0; j < str_len; ++j) {
                char anim_name_char;
                readBigEndian(anim_name_char, streamDataBuffer, 1U, blockStart);
                anim_name += anim_name_char;
            }
            std::uint32_t keyframeCount;
            readBigEndian(keyframeCount, streamDataBuffer, 4U, blockStart);

            float ticksPerSec, durInTicks;
            Engine::Math::Float32From16(&ticksPerSec, half_float_ticks_per_second);
            Engine::Math::Float32From16(&durInTicks, half_float_duration_in_ticks);
            Engine::priv::AnimationData animationData(*mesh, ticksPerSec, durInTicks);
            for (size_t j = 0; j < keyframeCount; ++j) {
                //itr.second.m_KeyframeData
                std::uint32_t pos_count, rot_count, scl_count;
                std::uint16_t keyframe_name_len;
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
                    std::uint16_t pos_half[3];
                    for (int i = 0; i < 3; ++i) {
                        readBigEndian(pos_half[i], streamDataBuffer, 2, blockStart);
                        Engine::Math::Float32From16(&posKey.value[i], pos_half[i]);
                    }
                    std::uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&posKey.time, time);
                }
                for (size_t k = 0; k < rot_count; ++k) {
                    auto& rotKey = channel.RotationKeys.emplace_back();
                    std::uint16_t rot_half[4];

                    readBigEndian(rot_half[0], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.x, rot_half[0]);
                    readBigEndian(rot_half[1], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.y, rot_half[1]);
                    readBigEndian(rot_half[2], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.z, rot_half[2]);
                    readBigEndian(rot_half[3], streamDataBuffer, 2, blockStart);  Engine::Math::Float32From16(&rotKey.value.w, rot_half[3]);

                    std::uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&rotKey.time, time);
                }
                for (size_t k = 0; k < scl_count; ++k) {
                    auto& sclKey = channel.ScalingKeys.emplace_back();
                    std::uint16_t scl_half[3];
                    for (int i = 0; i < 3; ++i) {
                        readBigEndian(scl_half[i], streamDataBuffer, 2, blockStart);
                        Engine::Math::Float32From16(&sclKey.value[i], scl_half[i]);
                    }
                    std::uint16_t time;
                    readBigEndian(time, streamDataBuffer, 2, blockStart);
                    Engine::Math::Float32From16(&sclKey.time, time);
                }
                animationData.m_KeyframeData.emplace(keyframe_key, channel);
            }
            mesh->m_Skeleton->m_AnimationData.emplace(anim_name, animationData);
        }
        std::uint32_t num_of_nodes;
        readBigEndian(num_of_nodes, streamDataBuffer, 4U, blockStart);
        std::vector<Engine::priv::MeshInfoNode*> sortedNodes;
        sortedNodes.reserve(num_of_nodes);
        for (std::uint32_t i = 0; i < num_of_nodes; ++i) {
            glm::mat4 node_transform;
            lamda_read_mat4_as_half_floats(node_transform);
            std::uint16_t str_len;
            std::string node_name = "";
            readBigEndian(str_len, streamDataBuffer, 2U, blockStart);
            for (auto j = 0; j < str_len; ++j) {
                char node_name_char;
                readBigEndian(node_name_char, streamDataBuffer, 1U, blockStart);
                node_name += node_name_char;
            }
            sortedNodes.push_back(NEW Engine::priv::MeshInfoNode(std::move(node_name), std::move(node_transform)));
        }
        for (std::uint32_t i = 0; i < num_of_nodes; ++i) {
            std::uint32_t num_of_children;
            readBigEndian(num_of_children, streamDataBuffer, 4U, blockStart);
            for (std::uint32_t j = 0; j < num_of_children; ++j) {
                std::uint32_t child;
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
    SMSH_Fileheader smsh_header(mesh);

    std::ofstream stream(filename, std::ios::binary | std::ios::out);

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
    SMSH_Attribute positions_attr { 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_3, 0, false, 0, sizeof(std::uint16_t) * 3, 0 
    };
    auto positions = vertexData.getPositions();
    std::vector<std::uint16_t> positions_packed(positions.size() * 3);
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = 0; j < 3; ++j) {
            Engine::Math::Float16From32(&positions_packed[(i * 3) + j], positions[i][j]);
        }
    }
    positions_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(positions_packed.data()), (unsigned int)(positions_packed.size() * 2));

    //uvs - 2 half floats
    SMSH_Attribute uvs_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_2, positions_attr.m_SizeOfAttribute, false, 0, sizeof(std::uint16_t) * 2, 0 
    };
    auto uvs = vertexData.getData<glm::vec2>(1);
    std::vector<std::uint16_t> uvs_packed(uvs.size() * 2);
    for (size_t i = 0; i < uvs.size(); ++i) {
        for (size_t j = 0; j < 2; ++j) {
            Engine::Math::Float16From32(&uvs_packed[(i * 2) + j], uvs[i][j]);
        }
    }
    uvs_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(uvs_packed.data()), (unsigned int)(uvs_packed.size() * 2));

    //normals - 1 packed unsigned 32 int
    SMSH_Attribute normals_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA, uvs_attr.m_SizeOfAttribute + uvs_attr.m_Offset, true, 0, sizeof(std::uint32_t), 0
    };
    auto normals = vertexData.getData<uint32_t>(2);
    normals_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(normals.data()), (unsigned int)(normals.size() * normals_attr.m_SizeOfAttribute));

    //binormals - 1 packed unsigned 32 int
    SMSH_Attribute binormals_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA, normals_attr.m_Offset + normals_attr.m_SizeOfAttribute, true, 0, sizeof(std::uint32_t), 0
    };
    auto binormals = vertexData.getData<uint32_t>(3);
    binormals_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(binormals.data()), (unsigned int)(binormals.size() * binormals_attr.m_SizeOfAttribute));

    //tangents - 1 packed unsigned 32 int
    SMSH_Attribute tangents_attr{ 
        SMSH_AttributeDataType::INT_2_10_10_10_REV, SMSH_AttributeComponentSize::BGRA, binormals_attr.m_Offset + binormals_attr.m_SizeOfAttribute, true, 0, sizeof(std::uint32_t), 0
    };
    auto tangents = vertexData.getData<uint32_t>(4);
    tangents_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(tangents.data()), (unsigned int)(tangents.size() * tangents_attr.m_SizeOfAttribute));

    //boneids - 4 half floats
    SMSH_Attribute boneids_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4, tangents_attr.m_Offset + tangents_attr.m_SizeOfAttribute, false, 0, sizeof(std::uint16_t) * 4, 0
    };
    auto boneids = vertexData.getData<glm::vec4>(5);
    std::vector<std::uint16_t> boneids_packed(boneids.size() * 4);
    for (size_t i = 0; i < boneids.size(); ++i) {
        for (size_t j = 0; j < 4; ++j) {
            Engine::Math::Float16From32(&boneids_packed[(i * 4) + j], boneids[i][j]);
        }
    }
    boneids_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(boneids_packed.data()), (unsigned int)(boneids_packed.size() * 2));

    //boneweights - 4 half floats
    SMSH_Attribute boneweights_attr{ 
        SMSH_AttributeDataType::Half_Float, SMSH_AttributeComponentSize::_4, boneids_attr.m_Offset + boneids_attr.m_SizeOfAttribute, false, 0, sizeof(std::uint16_t) * 4, 0
    };
    auto boneweights = vertexData.getData<glm::vec4>(6);
    std::vector<std::uint16_t> boneweights_packed(boneweights.size()*4);
    for (size_t i = 0; i < boneweights.size(); ++i) {
        for (size_t j = 0; j < 4; ++j) {
            Engine::Math::Float16From32(&boneweights_packed[(i * 4) + j], boneweights[i][j]);
        }
    }
    boneweights_attr.setBuffer(reinterpret_cast<const std::uint8_t*>(boneweights_packed.data()), (unsigned int)(boneweights_packed.size() * 2));

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
        case (unsigned int)SMSH_IndiceDataType::Unsigned_Byte: {
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                std::uint8_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 1U);
            }
            break;
        }case (unsigned int)SMSH_IndiceDataType::Unsigned_Short: {
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                std::uint16_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 2U);
            }
            break;
        }case (unsigned int)SMSH_IndiceDataType::Unsigned_Int: {
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                std::uint32_t indice = vertexData.m_Indices[i];
                writeBigEndian(stream, indice, 4U);
            }
            break;
        }default: { //unsigned int if default, TODO: raise error if this is reached?
            for (size_t i = 0; i < vertexData.m_Indices.size(); ++i) {
                std::uint32_t indice = vertexData.m_Indices[i];
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
                    std::uint16_t half_float;
                    Engine::Math::Float16From32(&half_float, inMatrix[i][j]);
                    writeBigEndian(instream, half_float, 2U);
                }
            }
        };
        auto lamda_write_string = [](std::ofstream& instream, const std::string& inString) {
            std::uint16_t str_len = (std::uint16_t)inString.length();
            writeBigEndian(instream, str_len);
            for (size_t i = 0; i < str_len; ++i) {
                writeBigEndian(instream, inString[i], 1U);
            }
        };

        //global inverse transform
        lamda_write_mat4_as_half_floats(stream, skeleton->m_GlobalInverseTransform);
        //bone mapping (string key to unsigned int)
        for (const auto& [key, id] : skeleton->m_BoneMapping) {
            writeBigEndian(stream, id, 4U);
            lamda_write_string(stream, key);
        }
        //bone info vector
        for (const auto& info : skeleton->m_BoneInfo) {
            //bone offset
            lamda_write_mat4_as_half_floats(stream, info.BoneOffset);
            //final transform
            lamda_write_mat4_as_half_floats(stream, info.FinalTransform);
        }
        //animation data
        writeBigEndian(stream, (std::uint32_t)skeleton->m_AnimationData.size());
        for (const auto& [animName, animationData] : skeleton->m_AnimationData) {
            std::uint16_t half_float_ticks_per_second;
            std::uint16_t half_float_duration_in_ticks;
            Engine::Math::Float16From32(&half_float_ticks_per_second, animationData.m_TicksPerSecond);
            Engine::Math::Float16From32(&half_float_duration_in_ticks, animationData.m_DurationInTicks);
            writeBigEndian(stream, half_float_ticks_per_second, 2U);
            writeBigEndian(stream, half_float_duration_in_ticks, 2U);
            lamda_write_string(stream, animName);
            std::uint32_t keyframeSize = (std::uint32_t)animationData.m_KeyframeData.size();
            writeBigEndian(stream, keyframeSize);
            for (const auto& [name, channel] : animationData.m_KeyframeData) {
                std::uint32_t pos_size = (std::uint32_t)channel.PositionKeys.size();
                std::uint32_t rot_size = (std::uint32_t)channel.RotationKeys.size();
                std::uint32_t scl_size = (std::uint32_t)channel.ScalingKeys.size();

                writeBigEndian(stream, pos_size, 4U);
                writeBigEndian(stream, rot_size, 4U);
                writeBigEndian(stream, scl_size, 4U);
                lamda_write_string(stream, name);
                auto lamda_write_vec3 = [](std::ofstream& instream, const Engine::priv::Vector3Key& key) {
                    std::array<std::uint16_t, 3> vec_half;
                    for (int i = 0; i < 3; ++i) {
                        Engine::Math::Float16From32(&vec_half[i], key.value[i]);
                        writeBigEndian(instream, vec_half[i]);
                    }
                    std::uint16_t time_half;
                    Engine::Math::Float16From32(&time_half, key.time);
                    writeBigEndian(instream, time_half, 2U);
                };

                for (const auto& posKey : channel.PositionKeys) {
                    lamda_write_vec3(stream, posKey);
                }
                for (const auto& rotKey : channel.RotationKeys) {
                    std::array<std::uint16_t, 4> quat_half;

                    Engine::Math::Float16From32(&quat_half[0], rotKey.value.x);
                    writeBigEndian(stream, quat_half[0], 2U);

                    Engine::Math::Float16From32(&quat_half[1], rotKey.value.y);
                    writeBigEndian(stream, quat_half[1], 2U);

                    Engine::Math::Float16From32(&quat_half[2], rotKey.value.z);
                    writeBigEndian(stream, quat_half[2], 2U);

                    Engine::Math::Float16From32(&quat_half[3], rotKey.value.w);
                    writeBigEndian(stream, quat_half[3], 2U);

                    std::uint16_t time_half;
                    Engine::Math::Float16From32(&time_half, rotKey.time);
                    writeBigEndian(stream, time_half, 2U);
                }
                for (const auto& sclKey : channel.ScalingKeys) {
                    lamda_write_vec3(stream, sclKey);
                }
            }
        }
        //node heirarchy
        std::vector<Engine::priv::MeshInfoNode*> sortedNodes;
        std::queue<Engine::priv::MeshInfoNode*> q;
        q.push(mesh.m_RootNode);
        while (!q.empty()) {
            auto qSize = q.size();
            while (qSize--) {
                auto* front = q.front();
                sortedNodes.push_back(front);
                for (const auto& child : front->Children) {
                    q.push(child);
                }
                q.pop();
            }
        }
        auto sorter = [](Engine::priv::MeshInfoNode* a, Engine::priv::MeshInfoNode* b) {
            return a->Name < b->Name;
        };
        std::sort(sortedNodes.begin(), sortedNodes.end(), sorter);
        std::unordered_map<std::string, uint32_t> nameMap; //string name => index in sorted vector
        nameMap.reserve(sortedNodes.size());
        for (size_t i = 0; i < sortedNodes.size(); ++i) {
            nameMap.emplace(sortedNodes[i]->Name, (std::uint32_t)i);
        }
        std::vector<std::vector<std::uint32_t>> relationships;
        relationships.resize(sortedNodes.size());
        for (size_t i = 0; i < sortedNodes.size(); ++i) {
            auto* node = sortedNodes[i];
            for (size_t j = 0; j < node->Children.size(); ++j) {
                auto* child = node->Children[j];
                relationships[i].emplace_back(nameMap.at(child->Name));
            }
        }
        std::uint32_t number_of_nodes = (std::uint32_t)sortedNodes.size();
        writeBigEndian(stream, number_of_nodes, 4U);
        for (const auto& node : sortedNodes) {
            lamda_write_mat4_as_half_floats(stream, node->Transform);
            lamda_write_string(stream, node->Name);
        }
        for (size_t i = 0; i < relationships.size(); ++i) {
            std::uint32_t number_of_children = (std::uint32_t)relationships[i].size();
            writeBigEndian(stream, number_of_children, 4U);
            for (size_t j = 0; j < number_of_children; ++j) {
                writeBigEndian(stream, relationships[i][j], 4U);
            }
        }
    }
    stream.close();
}