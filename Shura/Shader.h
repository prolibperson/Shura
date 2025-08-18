#pragma once
#include "Globals.h"

class Shader
{
public:
	bool load_vertex(const char* filename, SDL_GPUDevice* device);
	bool load_fragment(const char* filename, SDL_GPUDevice* device);

	bool load_shaders(const char* filename_vert, const char* filename_frag, SDL_GPUDevice* device)
	{
		return load_vertex(filename_vert, device) && load_fragment(filename_frag, device);
	}

	bool setup_pipeline(SDL_GPUDevice* device, SDL_Window* window);

	SDL_GPUShader* get_vertex_shader() { return vertex_shader; }
	SDL_GPUShader* get_fragment_shader() { return fragment_shader; }
	SDL_GPUGraphicsPipeline* get_pipeline() { return graphics_pipeline; }

private:
	SDL_GPUShader* vertex_shader = nullptr;
	SDL_GPUShader* fragment_shader = nullptr;

	SDL_GPUGraphicsPipeline* graphics_pipeline = nullptr;
};

