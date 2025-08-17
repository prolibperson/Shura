#pragma once
#include "Globals.h"

class Renderer
{
public:
	bool init(SDL_Window* window);
	void begin_frame();
	void end_frame();

	inline SDL_GPUDevice* get_device() { return device; }
	inline SDL_GPUBuffer* get_vertex_buffer() { return vertex_buffer; }

private:
	SDL_GPUDevice* device = nullptr;
	SDL_GPUCommandBuffer* command_buffer = nullptr;
	SDL_GPUTexture* swapchain_texture = nullptr;
	SDL_GPURenderPass* render_pass = nullptr;
	SDL_Window* rend_window = nullptr;
	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUColorTargetInfo color_target_info{};
	SDL_GPUBufferCreateInfo buffer_info{};

	bool create_device();
	bool create_command_buffer();
	bool create_vertex_buffer();
};

