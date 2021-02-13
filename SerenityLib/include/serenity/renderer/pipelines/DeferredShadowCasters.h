#pragma once
#ifndef ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H
#define ENGINE_RENDERER_PIPELINE_DEFERRED_SHADOW_CASTERS_H

#include <serenity/lights/Lights.h>

namespace Engine::priv {
    class GLDeferredDirectionalLightShadowInfo final : public IDirectionalLightShadowData {

    };
    class GLDeferredPointLightShadowInfo final : public IPointLightShadowData {

    };
    class GLDeferredProjectionLightShadowInfo final : public IProjectionLightShadowData {

    };
    class GLDeferredRodLightShadowInfo final : public IRodLightShadowData {

    };
    class GLDeferredSpotLightShadowInfo final : public ISpotLightShadowData {

    };
    class GLDeferredSunLightShadowInfo final : public ISunLightShadowData {

    };
}

#endif