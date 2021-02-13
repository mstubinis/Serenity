#pragma once
#ifndef ENGINE_ENGINE_OPTIONS_H
#define ENGINE_ENGINE_OPTIONS_H

#include <serenity/renderer/RendererIncludes.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/SSAO.h>

struct EngineOptions final {
    AntiAliasingAlgorithm   aa_algorithm;
    char**                             argv;
    int                                argc;
    unsigned char                      ssao_level;
    unsigned char                      hdr;

    int                                window_mode; //0 = normal, 1 = fullscreen, 2 = windowed_fullscreen

    bool                               god_rays_enabled;
    bool                               fog_enabled;
    bool                               show_console;
    bool                               maximized;
    bool                               vsync;
    unsigned int                       width;
    unsigned int                       height;
    std::string                        window_title;
    std::string                        icon;
    EngineOptions() {
        window_title      = "Engine";
        width             = 1024;
        height            = 768;
        ssao_level        = SSAOLevel::Medium;
        hdr               = HDRAlgorithm::None;
        god_rays_enabled  = true;
        vsync             = true;
        window_mode       = 0;
        fog_enabled       = false;
        show_console      = false;
        maximized         = false;
        aa_algorithm      = AntiAliasingAlgorithm::None;
        argc              = 0;
    }
};

#endif