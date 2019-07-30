#pragma once
#ifndef ENGINE_MATERIAL_LAYER_H
#define ENGINE_MATERIAL_LAYER_H

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <core/engine/materials/MaterialEnums.h>

class MaterialLayer;

typedef boost::function<void(const float& dt)> boost_uv_func;


class  Texture;
struct SimpleUVTranslationFunctor;
class MaterialLayer final{
    friend struct SimpleUVTranslationFunctor;
    private:
        Texture*    m_Texture;
        Texture*    m_Mask;
        Texture*    m_Cubemap;

        //x = blend mode? | y = texture enabled? | z = mask enabled? | w = cubemap enabled?
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

        const glm::vec4& data1() const;
        const glm::vec4& data2() const;
        const MaterialLayerBlendMode::Mode blendMode() const;

        void addUVModificationFunctor(const boost_uv_func& functor);
        void addUVModificationSimpleTranslation(const float& translationX, const float& translationY);

        void setBlendMode(const MaterialLayerBlendMode::Mode& mode);
        void setTexture(Texture* texture);
        void setMask(Texture* mask);
        void setCubemap(Texture* cubemap);
        void setTexture(const std::string& textureFile);
        void setMask(const std::string& maskFile);
        void setCubemap(const std::string& cubemapFile);

        void setData1(const float& x, const float& y, const float& z, const float& w);
        void setData2(const float& x, const float& y, const float& z, const float& w);

        void sendDataToGPU(const std::string& uniform_component_string, const unsigned int& component_index, const unsigned int& layer_index);

        const glm::vec2& getUVModifications() const;

        void update(const double& dt);
};

#endif