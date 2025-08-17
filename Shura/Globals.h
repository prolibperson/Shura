#pragma once

#include <SDL3/SDL.h>
#include "Logger.h"

constexpr const char* engine_name = "Shura";

constexpr short window_width = 960;
constexpr short window_height = 540;

#ifdef _DEBUG
constexpr bool isDebug = true;
#else
constexpr bool isDebug = false;
#endif