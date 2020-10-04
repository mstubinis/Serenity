#pragma once
#ifndef ENGINE_TERRAIN_H
#define ENINGE_TERRAIN_H

namespace sf{ 
    class Image;
}
class  Scene;
class  btRigidBody;
class  btVector3;
class  btTriangleCallback;
class  Handle;
class  btHeightfieldTerrainShape;
class  btCompoundShape;
class  Terrain;
class  TerrainData;
class  Mesh;

#include <ecs/Entity.h>
#include <core/engine/events/Observer.h>

#include <LinearMath/btScalar.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <core/engine/resources/Handle.h>

constexpr btScalar NULL_VERTEX = static_cast<btScalar>(-255.0);

class TerrainHeightfieldShape : public btHeightfieldTerrainShape {
    friend class TerrainData;
    friend class Terrain;
    friend class Mesh;
    private:
        std::vector<btScalar>           m_Data;
        std::vector<std::vector<bool>>  m_ProcessedVertices;
    public:
        TerrainHeightfieldShape(int heightWidth, int heightLength, void* data, float heightScale, float minHeight, float maxHeight, int upAxis, PHY_ScalarType type, bool flipQuads = false);
        virtual ~TerrainHeightfieldShape();

        bool getAndValidateVertex(int x, int y, btVector3& vertex, bool doBTScale) const;
        void processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const override;
        void setData(void* data);
};

class TerrainData {
    struct AdjacentPixels final {
        unsigned int imgSizeX = 0U;
        unsigned int imgSizeY = 0U;

        std::array<std::array<btScalar, 3>, 3> pixels;

        bool valid(int x, int y, int centerX, int centerY) const;
        AdjacentPixels() {
            for (size_t i = 0; i < pixels.size(); ++i) {
                pixels[i].fill(NULL_VERTEX);
            }
        }
    };
    friend class Terrain;
    friend class Mesh;
    private:
        bool                                                 m_UseDiamondSubDivision = false;
        btScalar                                             m_HeightScale           = 1.0;
        std::vector<std::vector<TerrainHeightfieldShape*>>   m_BtHeightfieldShapes;
        std::pair<float, float>                              m_MinAndMaxHeight       = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
        btCompoundShape*                                     m_FinalCompoundShape    = nullptr;
        unsigned int                                         m_VerticesPerSector     = 0;

        bool calculate_data(sf::Image& heightmapImage, unsigned int sectorSizeInPixels, unsigned int pointsPerPixel);

        AdjacentPixels get_adjacent_pixels(unsigned int x, unsigned int y, sf::Image& heightmapImage);
    public:
        TerrainData();
        virtual ~TerrainData();

        void clearData();
};

class Terrain : public Observer, public Entity {
    friend class Mesh;
    private:
        Handle       m_MeshHandle   = Handle{};
        TerrainData  m_TerrainData;

        bool internal_remove_quad(unsigned int indexX, unsigned int indexY);
        bool internal_remove_quad(unsigned int sectorX, unsigned int sectorY, unsigned int indexX, unsigned int indexY);
    public:
        Terrain(const std::string& name, sf::Image& heightmapImage, Handle& material, unsigned int sectorSizeInPixels = 4, unsigned int pointsPerPixel = 3, bool useDiamondSubdivisions = false, Scene* scene = nullptr);
        virtual ~Terrain();

        bool removeQuad(unsigned int sectorX, unsigned int sectorY, unsigned int indexX, unsigned int indexY);
        bool removeQuads(std::vector<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>>& quads);
        bool removeQuad(unsigned int indexX, unsigned int indexY);
        bool removeQuads(std::vector<std::tuple<unsigned int, unsigned int>>& quads);

        void setPosition(float x, float y, float z); 
        void setPosition(const glm::vec3& position);

        void setScale(float x, float y, float z); 
        void setScale(const glm::vec3& scale);

        bool getUseDiamondSubdivision() const;
        void setUseDiamondSubdivision(bool useDiamond);

        virtual void update(const float dt);
};
#endif