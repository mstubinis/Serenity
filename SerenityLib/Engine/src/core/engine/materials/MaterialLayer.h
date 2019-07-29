#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class MaterialLayer;

typedef boost::function<glm::vec2(MaterialLayer* layer, const float& x, const float& y, const float& dt)> boost_uv_func;

class Texture;
class MaterialLayer final{
    private:
        Texture*    m_Texture;
        Texture*    m_Mask;
        Texture*    m_Cubemap;

        glm::vec4   m_Data1;
        glm::vec4   m_Data2; 

        glm::vec2   m_UVModifications;

        std::vector<boost_uv_func> m_UVModificationQueue;

    public:
        MaterialLayer();
        ~MaterialLayer();

        Texture* getTexture() const;
        Texture* getMask() const;
        Texture* getCubemap() const;

        void addUVModificationFunctor(const boost_uv_func& functor);
        void addUVModificationSimpleTranslation(const float& translationX, const float& translationY);

        void setTexture(Texture* texture);
        void setMask(Texture* mask);
        void setCubemap(Texture* cubemap);

        void setData1(const float& x, const float& y, const float& z, const float& w);
        void setData2(const float& x, const float& y, const float& z, const float& w);

        void sendDataToGPU();

        const glm::vec2& getUVModifications() const;

        void update(const double& dt);
};

#endif