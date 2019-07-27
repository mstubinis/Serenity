#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class Texture;
class MaterialLayer {
    private:
        Texture*    m_Texture;
        Texture*    m_Mask;
        Texture*    m_Cubemap;

        glm::vec4   m_Data1;
        glm::vec4   m_Data2; 

        glm::vec2   m_UVModifications;

    public:
        MaterialLayer();
        ~MaterialLayer();

        Texture* getTexture() const;
        Texture* getMask() const;
        Texture* getCubemap() const;

        void setTexture(Texture* texture);
        void setMask(Texture* mask);
        void setCubemap(Texture* cubemap);

        void sendDataToGPU();

        const glm::vec2& getUVModifications() const;

        void update(const double& dt);
};

#endif