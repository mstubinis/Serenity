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
    : btHeightfieldTerrainShape{ heightWidth, heightLength, data, heightScale, btScalar(minHeight), btScalar(maxHeight), upAxis, type, flipQuads }
{
    m_ProcessedVerticesSizeRows = heightWidth;
    m_ProcessedVerticesSizeCols = heightLength;
    m_ProcessedVertices.resize(heightWidth * heightLength, true);
}
void TerrainHeightfieldShape::setData(void* data) {
    m_ProcessedVertices.clear();
    m_heightfieldDataUnknown = data;
    initialize(m_heightStickWidth, m_heightStickLength, data, m_heightScale, m_minHeight, m_maxHeight, m_upAxis, m_heightDataType, m_flipQuadEdges);
    m_ProcessedVerticesSizeRows = m_heightStickWidth;
    m_ProcessedVerticesSizeCols = m_heightStickLength;
    m_ProcessedVertices.resize(m_heightStickWidth * m_heightStickLength, true);
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
    return m_ProcessedVertices[(x * m_ProcessedVerticesSizeRows) + y];
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
    int endX   = m_heightStickWidth - 1;
    int startJ = 0;
    int endJ   = m_heightStickLength - 1;

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
            }else{
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

TerrainData::~TerrainData() {
    clearData();
}
void TerrainData::clearData() {
    for (auto& shape : m_BtHeightfieldShapes) {
        shape->setUserIndex2(std::numeric_limits<int>().min());
    }
}
btScalar TerrainData::bilinearFilter(uint32_t pixelX, uint32_t pixelY, uint32_t vertexX, uint32_t vertexY, sf::Image& heightmapImage) {
    const auto pixel_00      = heightmapImage.getPixel(pixelX,     pixelY).r;
    const auto pixel_10      = heightmapImage.getPixel(pixelX + 1, pixelY).r;
    const auto pixel_01      = heightmapImage.getPixel(pixelX,     pixelY + 1).r;
    const auto pixel_11      = heightmapImage.getPixel(pixelX + 1, pixelY + 1).r;

    const auto color00       = btScalar(pixel_00);
    const auto color10       = btScalar(pixel_10);
    const auto color01       = btScalar(pixel_01);
    const auto color11       = btScalar(pixel_11);

    const auto pixelXPercent = btScalar(vertexX) / btScalar(m_PointsPerPixel + 1);
    const auto pixelYPercent = btScalar(vertexY) / btScalar(m_PointsPerPixel + 1);

    const auto tx            = pixelXPercent - btScalar(pixelX);
    const auto ty            = pixelYPercent - btScalar(pixelY);
    
    btScalar  a              = glm::mix(color00, color10, tx);
    btScalar  b              = glm::mix(color01, color11, tx);
    btScalar res             = glm::mix(a, b, ty);
    return res;
}
bool TerrainData::calculate_data(sf::Image& heightmapImage, uint32_t pointsPerPixel, float heightScale, int blurIterations) {
    const auto pixelSize = heightmapImage.getSize();
    if (pixelSize.x == 0 || pixelSize.y == 0) {
        return false;
    }
    m_PointsPerPixel  = pointsPerPixel;
    m_MinAndMaxHeight = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
    uint32_t vertColSize = ((pixelSize.x - 1) * pointsPerPixel) + pixelSize.x;
    uint32_t vertRowSize = ((pixelSize.y - 1) * pointsPerPixel) + pixelSize.y;
    //height coordinates at [row][col]

    std::vector<btScalar> tempHeightCoords( vertRowSize * vertColSize, btScalar{ 0.0 } );
    auto getIdx = [](uint32_t rowSize, uint32_t row, uint32_t col) {
        return (row * rowSize) + col;
    };
    for (uint32_t vertexRow = 0; vertexRow < vertRowSize; ++vertexRow) {
        uint32_t pxlY = vertexRow / (pointsPerPixel + 1);
        if (vertexRow == vertRowSize - 1) {
            pxlY--;
        }
        for (uint32_t vertexCol = 0; vertexCol < vertColSize; ++vertexCol) {
            uint32_t pxlX = vertexCol / (pointsPerPixel + 1);
            if (vertexCol == vertColSize - 1) {
                pxlX--;
            }
            tempHeightCoords[getIdx(vertRowSize,vertexRow,vertexCol)] = bilinearFilter(pxlX, pxlY, vertexCol, vertexRow, heightmapImage);
        }
    }
    auto tempHeightCoordsCopy = tempHeightCoords;
    auto box_blur = [&getIdx, vertRowSize, vertColSize](auto& original, auto& copy) {
        for (uint32_t x = 0; x < vertRowSize; ++x) {
            for (uint32_t y = 0; y < vertColSize; ++y) {
                if (x < 1 || y < 1 || x + 1 >= vertRowSize || y + 1 >= vertColSize) {
                    continue;
                }
                btScalar sum = original[getIdx(vertRowSize, x - 1, y - 1)];
                sum += original[getIdx(vertRowSize, x + 0, y + 1)];
                sum += original[getIdx(vertRowSize, x + 1, y + 1)];
                sum += original[getIdx(vertRowSize, x - 1, y + 0)];
                sum += original[getIdx(vertRowSize, x + 0, y + 0)];
                sum += original[getIdx(vertRowSize, x + 1, y + 0)];
                sum += original[getIdx(vertRowSize, x - 1, y - 1)];
                sum += original[getIdx(vertRowSize, x + 0, y - 1)];
                sum += original[getIdx(vertRowSize, x + 1, y - 1)];
                copy[getIdx(vertRowSize, x, y)] = sum / 9;
            }
        }
        original = copy;
    };

    //box blur - gets rid of the pixelated terrace like effect and smooths it out. 4 iterations of this looks nice
    for (int i = 0; i < blurIterations; ++i) {
        box_blur(tempHeightCoords, tempHeightCoordsCopy);
    }

    //calculate min and max heights and scale the terrain height
    btScalar scale_by = 0.15 * static_cast<btScalar>(heightScale);
    for (uint32_t i = 0; i < vertRowSize; ++i) {
        for (uint32_t j = 0; j < vertColSize; ++j) {
            tempHeightCoords[getIdx(vertRowSize, i, j)] *= scale_by;
            auto pixel = tempHeightCoords[getIdx(vertRowSize, i, j)];
            m_MinAndMaxHeight.first  = glm::min(m_MinAndMaxHeight.first, (float)pixel);
            m_MinAndMaxHeight.second = glm::max(m_MinAndMaxHeight.second, (float)pixel);

        }
    }
    //temp should now be properly populated, move it to the real array
    clearData();
    m_BtHeightfieldShapes.clear();

    const auto vertexPerSectorSize = 10;
    m_BtHeightfieldShapesSizeCols = vertColSize / vertexPerSectorSize;
    m_BtHeightfieldShapesSizeRows = vertRowSize / vertexPerSectorSize;

    //prepare data
    m_BtHeightfieldShapes.resize(m_BtHeightfieldShapesSizeRows * m_BtHeightfieldShapesSizeCols, nullptr);
    for (uint32_t sectorX = 0; sectorX < m_BtHeightfieldShapesSizeRows; ++sectorX) {
        for (uint32_t sectorY = 0; sectorY < m_BtHeightfieldShapesSizeCols; ++sectorY) {
            std::vector<float> dummy_values = { 1.0f };//Bullet will do an assert check for null data, but i am manually assigning the data later on
            auto& shape = m_BtHeightfieldShapes[getIdx(m_BtHeightfieldShapesSizeRows, sectorX, sectorY)];
            shape = new TerrainHeightfieldShape(vertexPerSectorSize + 1, vertexPerSectorSize + 1, dummy_values.data(), (float)m_HeightScale, m_MinAndMaxHeight.first, m_MinAndMaxHeight.second, 1, PHY_ScalarType::PHY_FLOAT, false);
            shape->setUserIndex2(vertexPerSectorSize);
            shape->setUserIndex(vertexPerSectorSize);
        }
    }

    //fill in data
    for (uint32_t sectorX = 0; sectorX < m_BtHeightfieldShapesSizeRows; ++sectorX) {
        for (uint32_t sectorY = 0; sectorY < m_BtHeightfieldShapesSizeCols; ++sectorY) {
            auto& shape = *m_BtHeightfieldShapes[getIdx(m_BtHeightfieldShapesSizeRows, sectorX, sectorY)];
            for (uint32_t x = sectorX * vertexPerSectorSize; x < (sectorX * vertexPerSectorSize) + (vertexPerSectorSize + 1); ++x) {
                for (uint32_t y = sectorY * vertexPerSectorSize; y < (sectorY * vertexPerSectorSize) + (vertexPerSectorSize + 1); ++y) {
                    uint32_t x_       = glm::min(x, (uint32_t)vertRowSize - 1U);
                    uint32_t y_       = glm::min(y, (uint32_t)vertColSize - 1U);
                    shape.m_Data.push_back(tempHeightCoords[getIdx(vertRowSize, x_, y_)]);
                }
            }
            shape.setData(shape.m_Data.data());
        }
    }
    return true;
}

#pragma endregion

#pragma region Terrain

Terrain::Terrain(const std::string& name, sf::Image& heightmapImage, Handle& materialHandle, uint32_t pointsPerPixel, float heightScale, int blurIterations, bool useDiamondSubdivisions, Scene* scene)
    : Entity{ *scene }
{
    m_TerrainData.calculate_data(heightmapImage, pointsPerPixel, heightScale, blurIterations);
    Terrain::setUseDiamondSubdivision(useDiamondSubdivisions);
    m_MeshHandle = Engine::Resources::addResource<Mesh>(name, *this, 0.0f);

    addComponent<ComponentModel>(m_MeshHandle, materialHandle);
    addComponent<ComponentTransform>();
    addComponent<ComponentCollisionShape>(CollisionType::EMPTY_SHAPE_PROXYTYPE); //TODO: check CollisionType::TriangleShapeStatic
    addComponent<ComponentRigidBody>();

    auto rigid   = getComponent<ComponentRigidBody>();
    auto shape   = getComponent<ComponentCollisionShape>();
    auto model   = getComponent<ComponentModel>();
    m_TerrainData.m_FinalCompoundShape = new btCompoundShape();

    btScalar maxOriginY = m_TerrainData.m_BtHeightfieldShapesSizeCols > 0 ? btScalar(m_TerrainData.m_BtHeightfieldShapesSizeCols - 1) * btScalar(m_TerrainData.m_BtHeightfieldShapes[0]->getUserIndex2()) : btScalar(0.0);
    btScalar maxOriginX = m_TerrainData.m_BtHeightfieldShapesSizeRows > 0 ? btScalar(m_TerrainData.m_BtHeightfieldShapesSizeRows - 1) * btScalar(m_TerrainData.m_BtHeightfieldShapes[0]->getUserIndex()) : btScalar(0.0);
    for (uint32_t sectorX = 0; sectorX < m_TerrainData.m_BtHeightfieldShapesSizeRows; ++sectorX) {
        for (uint32_t sectorY = 0; sectorY < m_TerrainData.m_BtHeightfieldShapesSizeCols; ++sectorY) {
            auto* heightfield = m_TerrainData.m_BtHeightfieldShapes[(m_TerrainData.m_BtHeightfieldShapesSizeRows * sectorX) + sectorY];
            auto  dimensions  = glm::ivec2{ heightfield->getUserIndex(), heightfield->getUserIndex2() };

            btTransform xform;
            xform.setIdentity();
            xform.setOrigin(btVector3{ 
                btScalar(btScalar(sectorY) * btScalar(dimensions.y)),
                btScalar(0.0), 
                btScalar(btScalar(sectorX) * btScalar(dimensions.x))
            });
            m_TerrainData.m_FinalCompoundShape->addChildShape(xform, m_TerrainData.m_BtHeightfieldShapes[(m_TerrainData.m_BtHeightfieldShapesSizeRows * sectorX) + sectorY]);
            m_TerrainData.m_FinalCompoundShape->recalculateLocalAabb();
        }
    }
    shape->setCollision(m_TerrainData.m_FinalCompoundShape);
    rigid->setMass(0.0f);
    rigid->setDynamic(false);
    rigid->addFlags(ComponentRigidBody::Flags::DISABLE_WORLD_GRAVITY);
    rigid->setGravity(0.0, 0.0, 0.0);
    Terrain::translate(-maxOriginY * btScalar(0.5), 0, -maxOriginX * btScalar(0.5));
}
Terrain::~Terrain() {

}
void Terrain::setGravity(float x, float y, float z) {
    getComponent<ComponentRigidBody>()->setGravity(x, y, z);
}
void Terrain::setGravity(const glm::vec3& gravity) {
    setGravity(gravity.x, gravity.y, gravity.z);
}
bool Terrain::getUseDiamondSubdivision() const {
    return m_TerrainData.m_UseDiamondSubDivision;
}
void Terrain::setUseDiamondSubdivision(bool useDiamond) {
    if (m_TerrainData.m_UseDiamondSubDivision == useDiamond) {
        return;
    }
    m_TerrainData.m_UseDiamondSubDivision = useDiamond;
    for (size_t i = 0; i < m_TerrainData.m_BtHeightfieldShapesSizeRows; ++i) {
        for (size_t j = 0; j < m_TerrainData.m_BtHeightfieldShapesSizeCols; ++j) {
            m_TerrainData.m_BtHeightfieldShapes[(i * m_TerrainData.m_BtHeightfieldShapesSizeRows) + j]->setUseDiamondSubdivision(useDiamond);
        }
    }
}
bool Terrain::internal_remove_quad(uint32_t indexX, uint32_t indexY) {
    int sectorX = (indexX / m_TerrainData.m_PointsPerPixel);
    int sectorY = (indexY / m_TerrainData.m_PointsPerPixel);
    int modX    = (indexX % m_TerrainData.m_PointsPerPixel);
    int modY    = (indexY % m_TerrainData.m_PointsPerPixel);
    return internal_remove_quad(sectorX, sectorY, modX, modY);
}
bool Terrain::internal_remove_quad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY) {
    if (sectorX >= m_TerrainData.m_BtHeightfieldShapesSizeRows || sectorY >= m_TerrainData.m_BtHeightfieldShapesSizeCols) {
        return false;
    }
    auto* sectorData = m_TerrainData.m_BtHeightfieldShapes[(m_TerrainData.m_BtHeightfieldShapesSizeRows * sectorX) + sectorY];
    if (!sectorData || indexX >= sectorData->m_ProcessedVerticesSizeRows - 1 || indexY >= sectorData->m_ProcessedVerticesSizeCols - 1) {
        return false;
    }
    sectorData->m_ProcessedVertices[(indexX       * sectorData->m_ProcessedVerticesSizeRows) + indexY]       = false;
    sectorData->m_ProcessedVertices[((indexX + 1) * sectorData->m_ProcessedVerticesSizeRows) + (indexY + 1)] = false;
    sectorData->m_ProcessedVertices[(indexX       * sectorData->m_ProcessedVerticesSizeRows) + (indexY + 1)] = false;
    sectorData->m_ProcessedVertices[((indexX + 1) * sectorData->m_ProcessedVerticesSizeRows) + indexY]       = false;
    return true;
}
bool Terrain::removeQuad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY) {
    bool removal_result = internal_remove_quad(sectorX, sectorY, indexX, indexY);
    if (removal_result) {
        m_MeshHandle.get<Mesh>()->internal_recalc_indices_from_terrain(*this);
    }
    return removal_result;
}
bool Terrain::removeQuads(const std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>& quads) {
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
bool Terrain::removeQuads(const std::vector<std::tuple<uint32_t, uint32_t>>& quads) {
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
void Terrain::translate(decimal x, decimal y, decimal z) {
    auto rigidBody = getComponent<ComponentRigidBody>();
    auto transform = getComponent<ComponentTransform>();
    rigidBody->removePhysicsFromWorld();
    const auto currentPos = transform->getWorldPosition();
    transform->setPosition(currentPos.x + x, currentPos.y + y, currentPos.z + z);
    rigidBody->addPhysicsToWorld();
}
void Terrain::translate(const glm_vec3& position) {
    Terrain::translate(position.x, position.y, position.z);
}
void Terrain::setPosition(decimal x, decimal y, decimal z) {
    auto rigidBody = getComponent<ComponentRigidBody>();
    auto transform = getComponent<ComponentTransform>();
    rigidBody->removePhysicsFromWorld();
    transform->setPosition(x, y, z);
    rigidBody->addPhysicsToWorld();
}
void Terrain::setPosition(const glm_vec3& position) {
    Terrain::setPosition(position.x, position.y, position.z);
}
void Terrain::setScale(float x, float y, float z) {
    auto transform = getComponent<ComponentTransform>();
    transform->setScale(x, y, z);
}
void Terrain::setScale(const glm::vec3& scl) {
    Terrain::setScale(scl.x, scl.y, scl.z);
}

#pragma endregion