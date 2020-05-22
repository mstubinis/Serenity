#include "Terrain.h"

#include <ecs/Components.h>
#include <core/engine/system/Engine.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/system/Engine.h>
#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/physics/Collision.h>

#include <core/engine/model/ModelInstance.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace Engine;

#pragma region TerrainHeightfieldShape

TerrainHeightfieldShape::TerrainHeightfieldShape(int heightWidth, int heightLength, const void* data, float heightScale, float minHeight, float maxHeight, int upAxis, PHY_ScalarType type, bool flipQuads)
: btHeightfieldTerrainShape(heightWidth, heightLength, data, heightScale, (btScalar)minHeight, (btScalar)maxHeight, upAxis, type, flipQuads) {
    m_ProcessedVertices.resize(heightWidth, vector<bool>(heightLength, true));
}
TerrainHeightfieldShape::~TerrainHeightfieldShape() {

}
void TerrainHeightfieldShape::setData(const void* data) {
    m_ProcessedVertices.clear();
    m_heightfieldDataUnknown = data;
    initialize(m_heightStickWidth, m_heightStickLength, data, m_heightScale, m_minHeight, m_maxHeight, m_upAxis, m_heightDataType, m_flipQuadEdges);
    m_ProcessedVertices.resize(m_heightStickWidth, vector<bool>(m_heightStickLength, true));
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
        }default: {
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
    int startX  = 0;
    int endX    = m_heightStickWidth - 1;
    int startJ  = 0;
    int endJ    = m_heightStickLength - 1;

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
                {
                    valid[0] = getAndValidateVertex(x,     j,     vertices[indices[0]], true);
                    valid[1] = getAndValidateVertex(x,     j + 1, vertices[indices[1]], true);
                    valid[2] = getAndValidateVertex(x + 1, j + 1, vertices[indices[2]], true);

                    if (valid[0] || valid[1] || valid[2]) {
                        callback->processTriangle(vertices, 2 * x, j);
                    }
                }
                //second triangle
                {
                    valid[1] = getAndValidateVertex(x + 1, j + 1, vertices[indices[1]], true);
                    valid[2] = getAndValidateVertex(x + 1, j,     vertices[indices[2]], true);

                    if (valid[0] || valid[1] || valid[2]) {
                        callback->processTriangle(vertices, 2 * x + 1, j);
                    }
                }
            }else{
                //first triangle
                {
                    valid[0] = getAndValidateVertex(x,     j,     vertices[indices[0]], true);
                    valid[1] = getAndValidateVertex(x,     j + 1, vertices[indices[1]], true);
                    valid[2] = getAndValidateVertex(x + 1, j,     vertices[indices[2]], true);

                    if (valid[0] || valid[1] || valid[2]) {
                        callback->processTriangle(vertices, 2 * x, j);
                    }
                }
                //second triangle 
                {
                    valid[0] = getAndValidateVertex(x + 1, j,     vertices[indices[0]], true);
                    valid[2] = getAndValidateVertex(x + 1, j + 1, vertices[indices[2]], true);

                    if (valid[0] || valid[1] || valid[2]) {
                        callback->processTriangle(vertices, 2 * x + 1, j);
                    }
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

bool TerrainData::AdjacentPixels::valid(unsigned int x, unsigned int y) const {
    if (x >= 0 && x <= (imgSizeX) - 1) {
        if (y >= 0 && y <= (imgSizeY) - 1) {
            if (topLeft > NULL_VERTEX) {
                return true;
            }
            if (top > NULL_VERTEX) {
                return true;
            }
            if (topRight > NULL_VERTEX) {
                return true;
            }
            if (left > NULL_VERTEX) {
                return true;
            }
            if (right > NULL_VERTEX) {
                return true;
            }
            if (btmLeft > NULL_VERTEX) {
                return true;
            }
            if (btm > NULL_VERTEX) {
                return true;
            }
            if (btmRight > NULL_VERTEX) {
                return true;
            }
        }
    }
    return false;
}


bool TerrainData::calculate_data(sf::Image& heightmapImage, unsigned int sectorSizeInPixels, unsigned int pointsPerPixel) {
    const auto pixelSize = heightmapImage.getSize();
    if (pixelSize.x == 0 || pixelSize.y == 0) {
        return false;
    }
    const btScalar color_scale = 1.0 / static_cast<btScalar>(pointsPerPixel);
    btScalar scale_by = 0.15;
    //init the map with points at 0.0
    vector<vector<btScalar>> temp; temp.resize(pixelSize.y * pointsPerPixel, vector<btScalar>(pixelSize.x * pointsPerPixel, btScalar(0.0)));
    m_MinAndMaxHeight = make_pair(9999999999999999.9f, -9999999999999999.9f);

    m_VerticesPerSector           = sectorSizeInPixels * pointsPerPixel;
    unsigned int numSectorsWidth  = (unsigned int)temp[0].size() / m_VerticesPerSector;
    unsigned int numSectorsHeight = (unsigned int)temp.size() / m_VerticesPerSector;
    //calculate the point heights based on neighboring pixels
    for (int pxlX = 0; pxlX < (int)pixelSize.x; ++pxlX) {
        for (int pxlY = 0; pxlY < (int)pixelSize.y; ++pxlY) {
            int dataX = (pointsPerPixel * pxlX) + 1;
            int dataY = (pointsPerPixel * pxlY) + 1;
            if ((dataX < 0 || dataX >= temp.size()) || (dataY < 0 || dataX >= temp[0].size()) ) {
                continue;
            }
            auto pixel          = static_cast<btScalar>(heightmapImage.getPixel(pxlX, pxlY).r);
            temp[dataX][dataY]  = pixel;
            auto adjacentPixels = get_adjacent_pixels(pxlX, pxlY, heightmapImage);

            //TODO: this works, but is not perfect / ideal. works with points_per_pixel values of 3 and less
            for (int i = 1; i <= glm::max(int(pointsPerPixel) - 2, 1); ++i) {
                //top-left
                {
                    if (dataX - i >= 0 && dataY - i >= 0) {
                        if (adjacentPixels.valid(pxlX - i, pxlY - i)) {
                            temp[dataX - i][dataY - i] = pixel + ((adjacentPixels.topLeft - pixel) * color_scale);
                        }else{
                            temp[dataX - i][dataY - i] = pixel;
                        }
                    }
                }
                //top
                {
                    if (dataY - i >= 0) {
                        if (adjacentPixels.valid(pxlX, pxlY - i)) {
                            temp[dataX][dataY - i] = pixel + ((adjacentPixels.top - pixel) * color_scale);
                        }else{
                            temp[dataX][dataY - i] = pixel;
                        }
                    }
                }
                //top-right
                {
                    if (dataX + i < temp.size() && dataY - i >= 0) {
                        if (adjacentPixels.valid(pxlX + i, pxlY - i)) {
                            temp[dataX + i][dataY - i] = pixel + ((adjacentPixels.topRight - pixel) * color_scale);
                        }else{
                            temp[dataX + i][dataY - i] = pixel;
                        }
                    }
                }
                //left
                {
                    if (dataX - i >= 0) {
                        if (adjacentPixels.valid(pxlX - i, pxlY)) {
                            temp[dataX - i][dataY] = pixel + ((adjacentPixels.left - pixel) * color_scale);
                        }else{
                            temp[dataX - i][dataY] = pixel;
                        }
                    }
                }
                //right
                {
                    if (dataX + i < temp.size()) {
                        if (adjacentPixels.valid(pxlX + i, pxlY)) {
                            temp[dataX + i][dataY] = pixel + ((adjacentPixels.right - pixel) * color_scale);
                        }else{
                            temp[dataX + i][dataY] = pixel;
                        }
                    }
                }
                //btm-left
                {
                    if (dataX - i >= 0 && dataY + i < temp[0].size()) {
                        if (adjacentPixels.valid(pxlX - i, pxlY + i)) {
                            temp[dataX - i][dataY + i] = pixel + ((adjacentPixels.btmLeft - pixel) * color_scale);
                        }else{
                            temp[dataX - i][dataY + i] = pixel;
                        }
                    }
                }
                //btm
                {
                    if (dataY + i < temp[0].size()) {
                        if (adjacentPixels.valid(pxlX, pxlY + i)) {
                            temp[dataX][dataY + i] = pixel + ((adjacentPixels.btm - pixel) * color_scale);
                        }else{
                            temp[dataX][dataY + i] = pixel;
                        }
                    }
                }
                //btm-right
                {
                    if (dataX + i < temp.size() && dataY + i < temp[0].size()) {
                        if (adjacentPixels.valid(pxlX + i, pxlY + i)) {
                            temp[dataX + i][dataY + i] = pixel + ((adjacentPixels.btmRight - pixel) * color_scale);
                        }else{
                            temp[dataX + i][dataY + i] = pixel;
                        }
                    }
                }
            }
        }
    }
    //calculate min and max heights and scale the terrain height
    for (size_t i = 0; i < temp.size(); ++i) {
        for (size_t j = 0; j < temp[i].size(); ++j) {
            temp[i][j]              *= scale_by;
            auto pixel               = temp[i][j];
            m_MinAndMaxHeight.first  = glm::min(m_MinAndMaxHeight.first, (float)pixel);
            m_MinAndMaxHeight.second = glm::max(m_MinAndMaxHeight.second, (float)pixel);

        }
    }
    //temp should now be properly populated, move it to the real array
    clearData();

    m_BtHeightfieldShapes.clear();

    //prepare data
    m_BtHeightfieldShapes.resize(numSectorsWidth);
    for (unsigned int sectorX = 0; sectorX < numSectorsWidth; ++sectorX) {
        m_BtHeightfieldShapes[sectorX].reserve(numSectorsHeight);
        for (unsigned int sectorY = 0; sectorY < numSectorsHeight; ++sectorY) {
            vector<float> dummy_values = { 1.0f };//Bullet will do an assert check for null data, but i am manually assigning the data later on
            TerrainHeightfieldShape* shape = new TerrainHeightfieldShape(m_VerticesPerSector + 1, m_VerticesPerSector + 1, dummy_values.data(), (float)m_HeightScale, m_MinAndMaxHeight.first, m_MinAndMaxHeight.second, 1, PHY_ScalarType::PHY_FLOAT, false);
            shape->setUserIndex(m_VerticesPerSector);
            shape->setUserIndex2(m_VerticesPerSector);
            m_BtHeightfieldShapes[sectorX].push_back(shape);
        }
    }
    //fill in data
    for (unsigned int sectorX = 0; sectorX < numSectorsWidth; ++sectorX) {
        for (unsigned int sectorY = 0; sectorY < numSectorsHeight; ++sectorY) {
            auto& shape = *m_BtHeightfieldShapes[sectorX][sectorY];
            for (unsigned int x = 0; x < m_VerticesPerSector + 1; ++x) {
                for (unsigned int y = 0; y < m_VerticesPerSector + 1; ++y) {
                    unsigned int offset_x = (sectorX * m_VerticesPerSector) + x;
                    unsigned int offset_y = (sectorY * m_VerticesPerSector) + y;
                    unsigned int x_       = glm::min(offset_x, (unsigned int)(temp.size() - 1));
                    unsigned int y_       = glm::min(offset_y, (unsigned int)(temp[x].size() - 1));

                    btScalar height = temp[x_][y_];
                    shape.m_Data.push_back(height);
                }
            }
            shape.setData(shape.m_Data.data());
        }
    }
    return true;
}
TerrainData::AdjacentPixels TerrainData::get_adjacent_pixels(unsigned int x, unsigned int y, sf::Image& heightmapImage) {
    AdjacentPixels ret;
    int xSigned = static_cast<int>(x);
    int ySigned = static_cast<int>(y);
    const auto heightmapSize = glm::ivec2(heightmapImage.getSize().x, heightmapImage.getSize().y);
    ret.imgSizeX = heightmapSize.x;
    ret.imgSizeY = heightmapSize.y;
    //top-left
    if (xSigned - 1 >= 0 && ySigned - 1 >= 0) {
        ret.topLeft = (static_cast<btScalar>(heightmapImage.getPixel(x - 1, y - 1).r));
    }
    //top
    if (ySigned - 1 >= 0) {
        ret.top = (static_cast<btScalar>(heightmapImage.getPixel(x, y - 1).r));
    }
    //top-right
    if (xSigned + 1 <= heightmapSize.x-1 && ySigned - 1 >= 0) {
        ret.topRight = (static_cast<btScalar>(heightmapImage.getPixel(x + 1, y - 1).r));
    }
    //left
    if (xSigned - 1 >= 0) {
        ret.left = (static_cast<btScalar>(heightmapImage.getPixel(x - 1, y).r));
    }
    //right
    if (xSigned + 1 <= heightmapSize.x - 1) {
        ret.right = (static_cast<btScalar>(heightmapImage.getPixel(x + 1, y).r));
    }
    //btm-left
    if (xSigned - 1 >= 0 && ySigned + 1 <= heightmapSize.y - 1) {
        ret.btmLeft = (static_cast<btScalar>(heightmapImage.getPixel(x - 1, y + 1).r));
    }
    //btm
    if (ySigned + 1 <= heightmapSize.y - 1) {
        ret.btm = (static_cast<btScalar>(heightmapImage.getPixel(x, y + 1).r));
    }
    //btm-right
    if (xSigned + 1 <= heightmapSize.x - 1 && ySigned + 1 <= heightmapSize.y - 1) {
        ret.btmRight = (static_cast<btScalar>(heightmapImage.getPixel(x + 1, y + 1).r));
    }
    return ret;
}

#pragma endregion

#pragma region Terrain

Terrain::Terrain(const string& name, sf::Image& heightmapImage, Handle& materialHandle, unsigned int sectorSizeInPixels, unsigned int pointsPerPixel, bool useDiamondSubdivisions, Scene* scene) : Entity(*scene){
    m_TerrainData.calculate_data(heightmapImage, sectorSizeInPixels, pointsPerPixel);
    Terrain::setUseDiamondSubdivision(useDiamondSubdivisions);
    m_Mesh = NEW Mesh(name, *this, 0.000f);
    Handle handle  = priv::Core::m_Engine->m_ResourceManager.m_Resources.add(m_Mesh, ResourceType::Mesh);

    addComponent<ComponentModel>(m_Mesh, materialHandle);
    addComponent<ComponentBody>(CollisionType::Compound); //TODO: check CollisionType::TriangleShapeStatic
    auto* body   = getComponent<ComponentBody>();
    auto* model  = getComponent<ComponentModel>();
    Collision* c = NEW Collision(*body);

    m_TerrainData.m_FinalCompoundShape = new btCompoundShape();
    for (unsigned int sectorX = 0; sectorX < m_TerrainData.m_BtHeightfieldShapes.size(); ++sectorX) {
        for (unsigned int sectorY = 0; sectorY < m_TerrainData.m_BtHeightfieldShapes[sectorX].size(); ++sectorY) {
            auto* heightfield = m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY];
            auto  dimensions  = glm::ivec2(heightfield->getUserIndex(), heightfield->getUserIndex2());

            btTransform xform;
            xform.setIdentity();
            btVector3 origin;
            origin = btVector3( btScalar(sectorY * dimensions.y), btScalar(0.0), btScalar(sectorX * dimensions.x) );
            xform.setOrigin(origin);
            m_TerrainData.m_FinalCompoundShape->addChildShape(xform, m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY]);
            m_TerrainData.m_FinalCompoundShape->recalculateLocalAabb();
        }
    }
    c->setBtShape(m_TerrainData.m_FinalCompoundShape);
    body->setCollision(c);
    body->setMass(0.0f);
    body->setDynamic(false);
    body->setGravity(0, 0, 0);
    setScale(25, 25, 25);
}
Terrain::~Terrain(){

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
bool Terrain::internal_remove_quad(unsigned int indexX, unsigned int indexY) {
    int sectorX = (indexX / m_TerrainData.m_VerticesPerSector);
    int sectorY = (indexY / m_TerrainData.m_VerticesPerSector);

    int modX    = (indexX % m_TerrainData.m_VerticesPerSector);
    int modY    = (indexY % m_TerrainData.m_VerticesPerSector);

    return internal_remove_quad(sectorX, sectorY, modX, modY);
}
bool Terrain::internal_remove_quad(unsigned int sectorX, unsigned int sectorY, unsigned int indexX, unsigned int indexY) {
    if (sectorX >= m_TerrainData.m_BtHeightfieldShapes.size() || sectorY >= m_TerrainData.m_BtHeightfieldShapes[0].size()) {
        return false;
    }
    auto* sectorData = m_TerrainData.m_BtHeightfieldShapes[sectorX][sectorY];
    if (!sectorData || indexX >= sectorData->m_ProcessedVertices.size()-1 || indexY >= sectorData->m_ProcessedVertices[0].size()-1) {
        return false;
    }
    sectorData->m_ProcessedVertices[indexX    ][indexY    ] = false;
    sectorData->m_ProcessedVertices[indexX + 1][indexY + 1] = false;
    sectorData->m_ProcessedVertices[indexX    ][indexY + 1] = false;
    sectorData->m_ProcessedVertices[indexX + 1][indexY    ] = false;
    return true;
}
bool Terrain::removeQuad(unsigned int sectorX, unsigned int sectorY, unsigned int indexX, unsigned int indexY) {
    bool removal_result = internal_remove_quad(sectorX, sectorY, indexX, indexY);
    if (removal_result) {
        m_Mesh->internal_recalc_indices_from_terrain(*this);
    }
    return removal_result;
}
bool Terrain::removeQuads(vector<tuple<unsigned int, unsigned int, unsigned int, unsigned int>>& quads) {
    if (quads.size() == 0) {
        return false;
    }
    bool atLeastOne = false;
    for (const auto& tuple : quads) {
        auto sectorX = std::get<0>(tuple);
        auto sectorY = std::get<1>(tuple);
        auto indexX  = std::get<2>(tuple);
        auto indexY  = std::get<3>(tuple);
        auto removal_result = internal_remove_quad(sectorX, sectorY, indexX, indexY);
        if (removal_result) {
            atLeastOne = true;
        }
    }
    if (atLeastOne) {
        m_Mesh->internal_recalc_indices_from_terrain(*this);
    }
    return atLeastOne;
}
bool Terrain::removeQuad(unsigned int indexX, unsigned int indexY) {
    bool removal_result = internal_remove_quad(indexX, indexY);
    if (removal_result) {
        m_Mesh->internal_recalc_indices_from_terrain(*this);
    }
    return removal_result;
}
bool Terrain::removeQuads(vector<tuple<unsigned int, unsigned int>>& quads) {
    if (quads.size() == 0) {
        return false;
    }
    bool atLeastOne = false;
    for (const auto& tuple : quads) {
        auto indexX = std::get<0>(tuple);
        auto indexY = std::get<1>(tuple);
        bool removal_result = internal_remove_quad(indexX, indexY);
        if (removal_result) {
            atLeastOne = true;
        }
    }
    if (atLeastOne) {
        m_Mesh->internal_recalc_indices_from_terrain(*this);
    }
    return atLeastOne;
}


void Terrain::update(const float dt){
}
void Terrain::setPosition(float x, float y, float z){
    ComponentBody& body = *getComponent<ComponentBody>();
    //Physics::removeRigidBody(body);
    body.setPosition(x, y, z);
    //Physics::addRigidBody(body);
}
void Terrain::setPosition(const glm::vec3& position){
    Terrain::setPosition(position.x, position.y, position.z);
}
void Terrain::setScale(float x, float y, float z){
    auto components = getComponents<ComponentModel, ComponentBody>();
    auto* model     = get<0>(components);
    auto* body      = get<1>(components);
    body->setScale(x, y, z);
}
void Terrain::setScale(const glm::vec3& scl){
    Terrain::setScale(scl.x, scl.y, scl.z);
}

#pragma endregion