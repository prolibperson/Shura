#pragma once

/* sdl */
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

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