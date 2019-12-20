#pragma once
#ifndef GAME_GUI_SPECIFICS_SHIP_3D_VIEWER_H
#define GAME_GUI_SPECIFICS_SHIP_3D_VIEWER_H

#include <string>
#include <glm/vec4.hpp>

class  EntityWrapper;
class  Viewport;
class  Core;
class  Scene;
class  Camera;
class  Ship;
struct GameCameraShipSelectorLogicFunctor;
class Ship3DViewer final {
    friend struct GameCameraShipSelectorLogicFunctor;
    private:
        bool             m_IsCurrentlyDragging;
        bool             m_IsCurrentlyOverShip3DWindow;
        Ship*            m_EntityWrapperShip;
        //EntityWrapper* m_EntityWrapperShip;
        Viewport*        m_ShipDisplayViewport;
        void*            m_UserPointer;
        std::string      m_ChosenShipClass;
        glm::vec4        m_Color;
    public:
        Ship3DViewer(Core& core, Scene& scene, Camera& camera, const float x, const float y, const float w, const float h);
        ~Ship3DViewer();

        void setPosition(const float x, const float y);
        void setShipClass(const std::string& shipClass);

        void setShipViewportActive(const bool& active);

        void setUserPointer(void*);
        void* getUserPointer();

        const Viewport& getShipDisplay() const;
        const std::string& getShipClass() const;

        void update(const double& dt);
        void render();
};

#endif