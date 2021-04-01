
#include <serenity/Terrain.h>
#include <serenity/ecs/components/Components.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/model/ModelInstance.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <SFML/Graphics.hpp>

using namespace Engine;

#pragma region TerrainHeightfieldShape

TerrainHeightfieldShape::TerrainHeightfieldShape(int heightWidth, int heightLength, void* data, float heightScale, float minHeight, float maxHeight, int upAxis, PHY_ScalarType type, bool flipQuads) 
    : btHeightfieldTerrainShape{ heightWidth, heightLength, data, heightScale, (btScalar)minHeight, (btScalar)maxHeight, upAxis, type, flipQuads }
{
    m_ProcessedVertices.resize(heightWidth, std::vector<bool>(heightLength, true));
}
TerrainHeightfieldShape::~TerrainHeightfieldShape() {

}
void TerrainHeightfieldShape::setData(void* data) {
    m_ProcessedVertices.clear();
    m_heightfieldDataUnknown = data;
    initialize(m_heightStickWidth, m_heightStickLength, data, m_heightScale, m_minHeight, m_maxHeight, m_upAxis, m_heightDataType, m_flipQuadEdges);
    m_ProcessedVertices.resize(m_heightStickWidth, std::vector<bool>(m_heightStickLength, true));
}
bool TerrainHeightfieldShape::getAndValidateVertex(int x, int y, btVector3& vertex, bool doBTScale) const {
    btScalar height = getRawHeightFieldValue(x, y);
    switch (m_upAxis) {
        case 0: {
            vertex.setValue(height - m_localOrigin.getX(), (-m_width / btScalar(2.0)) + x, (-m_length / btScalar(2.0)) + y);
            break;
        }case 1: {
            vertex.setValue((-m_width / btScalar(2.0)) + x, height - m_localOrigin.getY(), (-m_length / btScalar(2.0)) + y);
            break;
        }case 2: {
            vertex.setValue((-m_width / btScalar(2.0)) + x, (-m_length / btScalar(2.0)) + y, height - m_localOrigin.getZ());
            break;
        }
    }
    if (doBTScale) {
        vertex *= m_localScaling;
    }
    return m_ProcessedVertices[x][y];
}
void TerrainHeightfieldShape::processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const {
    // scale down the input aabb's so they are in local (non-scaled) coordinates
    btVector3 localAabbMin = aabbMin * btVector3(1.0f / m_localScaling[0], 1.0f / m_localScaling[1], 1.0f / m_localScaling[2]);
    btVector3 localAabbMax = aabbMax * btVector3(1.0f / m_localScaling[0], 1.0f / m_localScaling[1], 1.0f / m_localScaling[2]);

    // account for local origin
    localAabbMin += m_localOrigin;
    localAabbMax += m_localOrigin;

    //quantize the aabbMin and aabbMax, and adjust the start/end ranges
    int quantizedAabbMin[3];
    int quantizedAabbMax[3];
    quantizeWithClamp(quantizedAabbMin, localAabbMin, 0);
    quantizeWithClamp(quantizedAabbMax, localAabbMax, 1);

    // expand the min/max quantized values
    // this is to catch the case where the input aabb falls between grid points!
    for (int i = 0; i < 3; ++i) {
        quantizedAabbMin[i]--;
        quantizedAabbMax[i]++;
    }
    int startX = 0;
    int endX = m_heightStickWidth - 1;
    int startJ = 0;
    int endJ = m_heightStickLength - 1;

    switch (m_upAxis) {
        case 0: {
            if (quantizedAabbMin[1] > startX)
                startX = quantizedAabbMin[1];
            if (quantizedAabbMax[1] < endX)
                endX = quantizedAabbMax[1];
            if (quantizedAabbMin[2] > startJ)
                startJ = quantizedAabbMin[2];
            if (quantizedAabbMax[2] < endJ)
                endJ = quantizedAabbMax[2];
            break;
        }case 1: {
            if (quantizedAabbMin[0] > startX)
                startX = quantizedAabbMin[0];
            if (quantizedAabbMax[0] < endX)
                endX = quantizedAabbMax[0];
            if (quantizedAabbMin[2] > startJ)
                startJ = quantizedAabbMin[2];
            if (quantizedAabbMax[2] < endJ)
                endJ = quantizedAabbMax[2];
            break;
        }case 2: {
            if (quantizedAabbMin[0] > startX)
                startX = quantizedAabbMin[0];
            if (quantizedAabbMax[0] < endX)
                endX = quantizedAabbMax[0];
            if (quantizedAabbMin[1] > startJ)
                startJ = quantizedAabbMin[1];
            if (quantizedAabbMax[1] < endJ)
                endJ = quantizedAabbMax[1];
            break;
        }default: {
            //need to get valid m_upAxis
            btAssert(0);
        }
    }
    // TODO If m_vboundsGrid is available, use it to determine if we really need to process this area
    for (int j = startJ; j < endJ; j++) {
        for (int x = startX; x < endX; x++) {
            int indices[3] = { 0, 1, 2 };
            if (m_flipTriangleWinding) {
                indices[0] = 2;
                indices[2] = 0;
            }
            btVector3 vertices[3];
            bool      valid[3];
            if (m_flipQuadEdges || (m_useDiamondSubdivision && !((j + x) & 1)) || (m_useZigzagSubdivision && !(j & 1))) {
                //first triangle
                valid[0] = getAndValidateVertex(x, j, vertices[indices[0]], true);
                valid[1] = getAndValidateVertex(x, j + 1, vertices[indices[1]], true);
                valid[2] = getAndValidateVertex(x + 1, j + 1, vertices[indices[2]], true);

                if (valid[0] || valid[1] || valid[2]) {
                    callback->processTriangle(vertices, 2 * x, j);
                }
                //second triangle
                valid[1] = getAndValidateVertex(x + 1, j + 1, vertices[indices[1]], true);
                valid[2] = getAndValidateVertex(x + 1, j, vertices[indices[2]], true);

                if (valid[0] || valid[1] || valid[2]) {
                    callback->processTriangle(vertices, 2 * x + 1, j);
                }
            }else {
                //first triangle
                valid[0] = getAndValidateVertex(x, j, vertices[indices[0]], true);
                valid[1] = getAndValidateVertex(x, j + 1, vertices[indices[1]], true);
                valid[2] = getAndValidateVertex(x + 1, j, vertices[indices[2]], true);

                if (valid[0] || valid[1] || valid[2]) {
                    callback->processTriangle(vertices, 2 * x, j);
                }
                //second triangle 
                valid[0] = getAndValidateVertex(x + 1, j, vertices[indices[0]], true);
                valid[2] = getAndValidateVertex(x + 1, j + 1, vertices[indices[2]], true);

                if (valid[0] || valid[1] || valid[2]) {
                    callback->processTriangle(vertices, 2 * x + 1, j);
                }
            }
        }
    }
}

#pragma endregion

#pragma region TerrainData

TerrainData::TerrainData() {

}
TerrainData::~TerrainData() {
    clearData();
}
void TerrainData::clearData() {
    for (auto& vec : m_BtHeightfieldShapes) {
        SAFE_DELETE_VECTOR(vec);
    }
    SAFE_DELETE(m_FinalCompoundShape);
}
bool TerrainData::AdjacentPixels::valid(int offsetX, int offsetY, int centerX, int centerY) const {
    int actualX = centerX + offsetX;
    int actualY = centerY + offsetY;
    if (actualX >= 0 && actualX <= (int)imgSizeX - 1) {
        if (actualY >= 0 && actualY <= (int)imgSizeY - 1) {
            if (pixels[offsetY + 1][offsetX + 1] > NULL_VERTEX) {
                return true;
            }
        }
    }
    return false;
}

bool TerrainData::calculate_data(sf::Image& heightmapImage, uint32_t sectorSizeInPixels, uint32_t pointsPerPixel) {
    const auto pixelSize = heightmapImage.getSize();
    if (pixelSize.x == 0 || pixelSize.y == 0) {
        return false;
    }
    btScalar color_scale = 1.0 / (btScalar)pointsPerPixel;
    btScalar scale_by = 0.15;
    //init the map with points at 0.0
    std::vector<std::vector<btScalar>> temp(pixelSize.y * pointsPerPixel, std::vector<btScalar>(pixelSize.x * pointsPerPixel, btScalar(0.0)));
    m_MinAndMaxHeight = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };

    m_VerticesPerSector = sectorSizeInPixels * pointsPerPixel;
    uint32_t numSectorsWidth  = (uint32_t)temp[0].size() / m_VerticesPerSector;
    uint32_t numSectorsHeight = (uint32_t)temp.size() / m_VerticesPerSector;
    //calculate the point heights based on neighboring pixels
    for (int pxlX = 0; pxlX < (int)pixelSize.x; ++pxlX) {
        for (int pxlY = 0; pxlY < (int)pixelSize.y; ++pxlY) {
            int dataX = (pointsPerPixel * pxlX) + 1;
            int dataY = (pointsPerPixel * pxlY) + 1;
            if ((dataX < 0 || (size_t)dataX >= temp.size()) || (dataY < 0 || (size_t)dataX >= temp[0].size())) {
                continue;
            }
            auto pixel = (btScalar)heightmapImage.getPixel(pxlX, pxlY).r;
            temp[dataX][dataY] = pixel;
            auto adjacentPixels = get_adjacent_pixels(pxlX, pxlY, heightmapImage);

            //TODO: this works, but is not perfect / ideal. works with pointsPerPixel values of 3 and less
            for (int i = 1; i <= glm::max(int(pointsPerPixel) - 2, 1); ++i) {
                btScalar valid, heightValue;
                for (int oX = -1; oX <= 1; ++oX) {
                    for (int oY = -1; oY <= 1; ++oY) {
                        if (dataX + oX >= 0 && dataY + oY >= 0) {
                            valid = (btScalar)adjacentPixels.valid(oX, oY, pxlX, pxlY);
                            heightValue = pixel + (((adjacentPixels.pixels[oY + 1][oX + 1] - pixel) * color_scale) * valid);
                            temp[dataX + oX][dataY + oY] = heightValue;
                        }
                    }
                }
            }
        }
    }
    //calculate min and max heights and scale the terrain height
    for (size_t i = 0; i < temp.size(); ++i) {
        for (size_t j = 0; j < temp[i].size(); ++j) {
            temp[i][j] *= scale_by;
            auto pixel = temp[i][j];
            m_MinAndMaxHeight.first = glm::min(m_MinAndMaxHeight.first, (float)pixel);
            m_MinAndMaxHeight.second = glm::max(m_MinAndMaxHeight.second, (float)pixel);

        }
    }
    //temp should now be properly populated, move it to the real array
    clearData();

    m_BtHeightfieldShapes.clear();

    //prepare data
    m_BtHeightfieldShapes.resize(numSectorsWidth);
    for (uint32_t sectorX = 0; sectorX < numSectorsWidth; ++sectorX) {
        m_BtHeightfieldShapes[sectorX].reserve(numSectorsHeight);
        for (uint32_t sectorY = 0; sectorY < numSectorsHeight; ++sectorY) {
            std::vector<float> dummy_values = { 1.0f };//Bullet will do an assert check for null data, but i am manually assigning the data later on
            TerrainHeightfieldShape* shape = new TerrainHeightfieldShape(m_VerticesPerSector + 1, m_VerticesPerSector + 1, dummy_values.data(), (float)m_HeightScale, m_MinAndMaxHeight.first, m_MinAndMaxHeight.second, 1, PHY_ScalarType::PHY_FLOAT, false);
            shape->setUserIndex(m_VerticesPerSector);
            shape->setUserIndex2(m_VerticesPerSector);
            m_BtHeightfieldShapes[sectorX].push_back(shape);
        }
    }
    //fill in data
    for (uint32_t sectorX = 0; sectorX < numSectorsWidth; ++sectorX) {
        for (uint32_t sectorY = 0; sectorY < numSectorsHeight; ++sectorY) {
            auto& shape = *m_BtHeightfieldShapes[sectorX][sectorY];
            for (uint32_t x = 0; x < m_VerticesPerSector + 1; ++x) {
                for (uint32_t y = 0; y < m_VerticesPerSector + 1; ++y) {
                    uint32_t offset_x = (sectorX * m_VerticesPerSector) + x;
                    uint32_t offset_y = (sectorY * m_VerticesPerSector) + y;
                    uint32_t x_ = glm::min(offset_x, (uint32_t)temp.size() - 1U);
                    uint32_t y_ = glm::min(offset_y, (uint32_t)temp[x].size() - 1U);

                    btScalar height = temp[x_][y_];
                    shape.m_Data.push_back(height);
                }
            }
            shape.setData(shape.m_Data.data());
        }
    }
    return true;
}
TerrainData::AdjacentPixels TerrainData::get_adjacent_pixels(uint32_t pixelX, uint32_t pixelY, sf::Image& heightmapImage) {
    AdjacentPixels ret;
    int pxlX = (int)pixelX;
    int pxlY = (int)pixelY;
    const auto size = glm::ivec2(heightmapImage.getSize().x, heightmapImage.getSize().y);
    ret.imgSizeX = size.x;
    ret.imgSizeY = size.y;

    int xx = 0;
    int yy = 0;
    for (int i = pxlX - 1; i <= pxlX + 1; ++i) {
        if (i >= 0 && i <= size.x - 1) {
            yy = 0;
            for (int j = pxlY - 1; j <= pxlY + 1; ++j) {
                if (j >= 0 && j <= size.y - 1) {
                    ret.pixels[yy][xx] = (btScalar)heightmapImage.getPixel(i, j).r;
                }
                yy++;
            }
        }
        xx++;
    }
    return ret;
}

#pragma endregion

#pragma region Terrain

Terrain::Terrain(const std::string& name, sf::Image& heightmapImage, Handle& materialHandle, uint32_t sectorSizeInPixels, uint32_t pointsPerPixel, bool useDiamondSubdivisions, Scene* scene)
    : Entity{ *scene }
{
    m_TerrainData.calculate_data(heightmapImage, sectorSizeInPixels, pointsPerPixel);
    Terrain::setUseDiamondSubdivision(useDiamondSubdivisions);

    m_MeshHandle = Engine::Resources::addResource<Mesh>(name, *this, 0.0f);

    addComponent<ComponentModel>(m_MeshHandle, materialHandle);
    addComponent<ComponentBody>();
    addComponent<ComponentCollisionShape>(CollisionType::COMPOUND_SHAPE_PROXYTYPE); //TODO: check CollisionType::TriangleShapeStatic
    addComponent<ComponentBodyRigid>();

    auto rigid = getComponent<ComponentBodyRigid>();
    auto shape = getComponent<ComponentCollisionShape>();
    auto model = getComponent<ComponentModel>();
    m_TerrainData.m_FinalCompoundShape = new btCompoundShape();
    for (uint32_t sectorX = 0; sectorX < m_TerrainData.m_BtHeightfieldShapes.size(); ++sectorX) {
        for (uint32_t sectorY = 0; sectorY < m_TerrainData.m_BtHeightfieldShapes[sectorX].size(); ++sectorY) {
            auto* heightfield = m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY];
            auto  dimensions  = glm::ivec2(heightfield->getUserIndex(), heightfield->getUserIndex2());

            btTransform xform;
            xform.setIdentity();
            btVector3 origin;
            origin = btVector3(btScalar(sectorY * dimensions.y), btScalar(0.0), btScalar(sectorX * dimensions.x));
            xform.setOrigin(origin);
            m_TerrainData.m_FinalCompoundShape->addChildShape(xform, m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY]);
            m_TerrainData.m_FinalCompoundShape->recalculateLocalAabb();
        }
    }
    shape->setCollision(m_TerrainData.m_FinalCompoundShape);
    rigid->setMass(0.0f);
    rigid->setDynamic(false);
    rigid->setGravity(0, 0, 0);
    setScale(25, 25, 25);
}
Terrain::~Terrain() {

}
bool Terrain::getUseDiamondSubdivision() const {
    return m_TerrainData.m_UseDiamondSubDivision;
}
void Terrain::setUseDiamondSubdivision(bool useDiamond) {
    if (m_TerrainData.m_UseDiamondSubDivision == useDiamond) {
        return;
    }
    m_TerrainData.m_UseDiamondSubDivision = useDiamond;
    for (size_t i = 0; i < m_TerrainData.m_BtHeightfieldShapes.size(); ++i) {
        for (size_t j = 0; j < m_TerrainData.m_BtHeightfieldShapes[i].size(); ++j) {
            m_TerrainData.m_BtHeightfieldShapes[i][j]->setUseDiamondSubdivision(useDiamond);
        }
    }
}
bool Terrain::internal_remove_quad(uint32_t indexX, uint32_t indexY) {
    int sectorX = (indexX / m_TerrainData.m_VerticesPerSector);
    int sectorY = (indexY / m_TerrainData.m_VerticesPerSector);

    int modX = (indexX % m_TerrainData.m_VerticesPerSector);
    int modY = (indexY % m_TerrainData.m_VerticesPerSector);

    return internal_remove_quad(sectorX, sectorY, modX, modY);
}
bool Terrain::internal_remove_quad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY) {
    if (sectorX >= m_TerrainData.m_BtHeightfieldShapes.size() || sectorY >= m_TerrainData.m_BtHeightfieldShapes[0].size()) {
        return false;
    }
    auto* sectorData = m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY];
    if (!sectorData || indexX >= sectorData->m_ProcessedVertices.size() - 1 || indexY >= sectorData->m_ProcessedVertices[0].size() - 1) {
        return false;
    }
    sectorData->m_ProcessedVertices[indexX][indexY] = false;
    sectorData->m_ProcessedVertices[indexX + 1][indexY + 1] = false;
    sectorData->m_ProcessedVertices[indexX][indexY + 1] = false;
    sectorData->m_ProcessedVertices[indexX + 1][indexY] = false;
    return true;
}
bool Terrain::removeQuad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY) {
    bool removal_result = internal_remove_quad(sectorX, sectorY, indexX, indexY);
    if (removal_result) {
        m_MeshHandle.get<Mesh>()->internal_recalc_indices_from_terrain(*this);
    }
    return removal_result;
}
bool Terrain::removeQuads(std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>& quads) {
    if (quads.size() == 0) {
        return false;
    }
    bool atLeastOne = false;
    for (const auto& [sectorX, sectorY, indexX, indexY] : quads) {
        auto removal_result = internal_remove_quad(sectorX, sectorY, indexX, indexY);
        if (removal_result) {
            atLeastOne = true;
        }
    }
    if (atLeastOne) {
        m_MeshHandle.get<Mesh>()->internal_recalc_indices_from_terrain(*this);
    }
    return atLeastOne;
}
bool Terrain::removeQuad(uint32_t indexX, uint32_t indexY) {
    bool removal_result = internal_remove_quad(indexX, indexY);
    if (removal_result) {
        m_MeshHandle.get<Mesh>()->internal_recalc_indices_from_terrain(*this);
    }
    return removal_result;
}
bool Terrain::removeQuads(std::vector<std::tuple<uint32_t, uint32_t>>& quads) {
    if (quads.size() == 0) {
        return false;
    }
    bool atLeastOne = false;
    for (const auto& [indexX, indexY] : quads) {
        bool removal_result = internal_remove_quad(indexX, indexY);
        if (removal_result) {
            atLeastOne = true;
        }
    }
    if (atLeastOne) {
        m_MeshHandle.get<Mesh>()->internal_recalc_indices_from_terrain(*this);
    }
    return atLeastOne;
}


void Terrain::update(const float dt) {
}
void Terrain::setPosition(float x, float y, float z) {
    //auto rigidBody  = getComponent<ComponentBodyRigid>();
    auto transform  = getComponent<ComponentBody>();
    //Engine::Physics::removeRigidBody(rigidBody);
    transform->setPosition(x, y, z);
    //Engine::Physics::addRigidBody(rigidBody);
}
void Terrain::setPosition(const glm::vec3& position) {
    Terrain::setPosition(position.x, position.y, position.z);
}
void Terrain::setScale(float x, float y, float z) {
    auto transform = getComponent<ComponentBody>();
    transform->setScale(x, y, z);
}
void Terrain::setScale(const glm::vec3& scl) {
    Terrain::setScale(scl.x, scl.y, scl.z);
}

#pragma endregion