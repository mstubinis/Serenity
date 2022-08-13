#pragma once
#ifndef ENGINE_ENGINE_OPTIONS_H
#define ENGINE_ENGINE_OPTIONS_H

#include <serenity/renderer/APIManager.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/system/window/WindowIncludes.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/SSAO.h>

struct EngineOptions final {
    std::string                        window_title       = "Engine";
    std::string                        icon;
    RenderingAPI::Type                 renderingAPI       = RenderingAPI::OpenGL;
    uint32_t                           width              = 1024;
    uint32_t                           height             = 768;
    AntiAliasingAlgorithm              aa_algorithm       = AntiAliasingAlgorithm::None;
    char**                             argv               = nullptr;
    int                                argc               = 0;
    uint8_t                            ssao_level         = SSAOLevel::Medium;
    uint8_t                            hdr                = HDRAlgorithm::None;
    WindowMode::Mode                   window_mode        = WindowMode::Windowed;
    bool                               editor_enabled     = false;
    bool                               god_rays_enabled   = true;
    bool                               fog_enabled        = false;
    bool                               show_console       = false;
    bool                               maximized          = false;
    bool                               vsync              = true;
};

#endif