#pragma once
#ifndef ENGINE_ENGINE_OPTIONS_H
#define ENGINE_ENGINE_OPTIONS_H

#include <serenity/renderer/RendererIncludes.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/SSAO.h>

struct EngineOptions final {
    AntiAliasingAlgorithm              aa_algorithm       = AntiAliasingAlgorithm::None;
    char**                             argv;
    int                                argc               = 0;
    unsigned char                      ssao_level         = SSAOLevel::Medium;
    unsigned char                      hdr                = HDRAlgorithm::None;

    int                                window_mode        = 0; //0 = normal, 1 = fullscreen, 2 = windowed_fullscreen

    bool                               editor_enabled     = false;
    bool                               god_rays_enabled   = true;
    bool                               fog_enabled        = false;
    bool                               show_console       = false;
    bool                               maximized          = false;
    bool                               vsync              = true;
    unsigned int                       width              = 1024;
    unsigned int                       height             = 768;
    std::string                        window_title       = "Engine";
    std::string                        icon;
};

#endif