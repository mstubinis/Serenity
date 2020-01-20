#pragma once
#ifndef ENGINE_ENGINE_OPTIONS_H
#define ENGINE_ENGINE_OPTIONS_H

#include <string>
#include <core/engine/renderer/RendererIncludes.h>

struct EngineOptions final {
    AntiAliasingAlgorithm::Algorithm   aa_algorithm;
    char**                             argv;
    int                                argc;
    bool                               ssao_enabled;
    bool                               hdr_enabled;

    int                                window_mode; //0 = normal, 1 = fullscreen, 2 = windowed_fullscreen

    bool                               god_rays_enabled;
    bool                               fog_enabled;
    bool                               show_console;
    bool                               maximized;
    unsigned int                       width;
    unsigned int                       height;
    std::string                        window_title;
    std::string                        icon;
    EngineOptions() {
        window_title      = "Engine";
        icon              = "";
        width             = 1024;
        height            = 768;
        ssao_enabled      = true;
        hdr_enabled       = true;
        god_rays_enabled  = true;
        window_mode       = 0;
        fog_enabled       = false;
        show_console      = false;
        maximized         = false;
        aa_algorithm      = AntiAliasingAlgorithm::FXAA;
        argc              = 0;
    }
};

#endif