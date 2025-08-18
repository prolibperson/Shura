#pragma once
#include "Globals.h"

#include "Renderer.h"

class Engine
{
public:
	bool init();
	void run();
	void shutdown();

	inline SDL_Window* get_window() { return window; }
private:
	bool running = true;
	SDL_Window* window = nullptr;

	Renderer renderer_inst{};
	Shader shader_inst{};

	bool poll_events();
};

