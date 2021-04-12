#pragma once
#ifndef ENGINE_TERRAIN_H
#define ENGINE_TERRAIN_H

namespace sf {
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

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/entity/Entity.h>
#include <serenity/events/Observer.h>

#include <LinearMath/btScalar.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <serenity/resources/Handle.h>
#include <array>

constexpr btScalar NULL_VERTEX = static_cast<btScalar>(-255.0);

class TerrainHeightfieldShape final : public btHeightfieldTerrainShape {
    friend class TerrainData;
    friend class Terrain;
    friend class Mesh;
    private:
        std::vector<btScalar>  m_Data;
        std::vector<bool>      m_ProcessedVertices;
        uint32_t               m_ProcessedVerticesSizeRows = 0;
        uint32_t               m_ProcessedVerticesSizeCols = 0;
    public:
        TerrainHeightfieldShape(int heightWidth, int heightLength, void* data, float heightScale, float minHeight, float maxHeight, int upAxis, PHY_ScalarType, bool flipQuads = false);

        bool getAndValidateVertex(int x, int y, btVector3& vertex, bool doBTScale) const;
        void processAllTriangles(btTriangleCallback*, const btVector3& aabbMin, const btVector3& aabbMax) const override;
        void setData(void* data);
};

class TerrainData final {
    friend class Terrain;
    friend class Mesh;
    private:
        std::vector<TerrainHeightfieldShape*>                m_BtHeightfieldShapes;
        std::pair<float, float>                              m_MinAndMaxHeight             = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
        btCompoundShape*                                     m_FinalCompoundShape          = nullptr;
        uint32_t                                             m_PointsPerPixel              = 0;
        uint32_t                                             m_BtHeightfieldShapesSizeRows = 0;
        uint32_t                                             m_BtHeightfieldShapesSizeCols = 0;
        btScalar                                             m_HeightScale                 = 1.0;
        bool                                                 m_UseDiamondSubDivision       = false;

        bool calculate_data(sf::Image& heightmapImage, uint32_t pointsPerPixel, float heightScale, int blurIterations);
        btScalar bilinearFilter(uint32_t pixelX, uint32_t pixelY, uint32_t vertexX, uint32_t vertexY, sf::Image& heightmapImage);

    public:
        TerrainData() = default;
        ~TerrainData();

        void clearData();
};

class Terrain : public Observer, public Entity {
    friend class Mesh;
    private:
        Handle        m_MeshHandle;
        TerrainData   m_TerrainData;

        bool internal_remove_quad(uint32_t indexX, uint32_t indexY);
        bool internal_remove_quad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY);
    public:
        Terrain(const std::string& name, sf::Image& heightmapImage, Handle& material, uint32_t pointsPerPixel = 2, float heightScale = 1.0f, int blurIterations = 4, bool useDiamondSubdivisions = false, Scene* scene = nullptr);
        virtual ~Terrain();

        bool removeQuad(uint32_t sectorX, uint32_t sectorY, uint32_t indexX, uint32_t indexY);
        bool removeQuads(const std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>& quads);
        bool removeQuad(uint32_t indexX, uint32_t indexY);
        bool removeQuads(const std::vector<std::tuple<uint32_t, uint32_t>>& quads);

        void setPosition(decimal x, decimal y, decimal z);
        void setPosition(const glm_vec3& position);

        void translate(decimal x, decimal y, decimal z);
        void translate(const glm_vec3& position);

        void setScale(float x, float y, float z);
        void setScale(const glm::vec3& scale);

        bool getUseDiamondSubdivision() const;
        void setUseDiamondSubdivision(bool useDiamond);
};
#endif