#pragma once
#include "Globals.h"
#include "Mesh.h"
#include "Shader.h"

class Renderer
{
public:
	bool init(SDL_Window* window);
	void begin_frame();
	void draw(SDL_GPUGraphicsPipeline* graphics_pipeline);
	void end_frame();

	Mesh mesh_inst{};

	inline SDL_GPUDevice* get_device() { return device; }

private:
	SDL_GPUDevice* device = nullptr;
	SDL_GPUCommandBuffer* command_buffer = nullptr;
	SDL_GPUTexture* swapchain_texture = nullptr;
	SDL_GPURenderPass* render_pass = nullptr;
	SDL_Window* rend_window = nullptr;

	SDL_GPUColorTargetInfo color_target_info{};

	//Mesh mesh_inst{};

	bool create_device();
	bool create_command_buffer();
};

