#pragma once

#include <SDL3/SDL.h>
#include "Logger.h"

constexpr const char* engine_name = "Shura";

constexpr short window_width = 1280;
constexpr short window_height = 720;

#ifdef _DEBUG
constexpr bool isDebug = true;
#else
constexpr bool isDebug = false;
#endif