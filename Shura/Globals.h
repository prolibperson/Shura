#pragma once

/* sdl */
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "SDL3_shadercross/SDL_shadercross.h"

/* imgui */
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlgpu3.h"

#include "Logger.h"

#include <vector>

constexpr const char* engine_name = "Shura";

constexpr short window_width = 1280;
constexpr short window_height = 720;

#ifdef _DEBUG
constexpr bool isDebug = true;
#else
constexpr bool isDebug = false;
#endif

#define ENSURE(func, success_msg, error_msg) \
    do { \
        if (!(func)) { \
            LogError("{}: {}", (error_msg), SDL_GetError()); \
            return false; \
        } \
        Log("{}", (success_msg)); \
    } while(0)
