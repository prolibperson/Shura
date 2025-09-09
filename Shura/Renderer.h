#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Input.h"
#include "MeshManager.h"
#include "Shader.h"

class Renderer {
public:
	bool init(SDL_Window* window, Camera* camera, Input* input);
	void begin_frame();
	void draw(SDL_GPUGraphicsPipeline* graphics_pipeline);
	void end_frame();

	void cleanup();

	bool create_depth_texture();
	bool create_sampler();
	SDL_GPUTexture* load_texture(const char* path);

	std::vector<Mesh> meshes;

	float fps = 0;

	inline SDL_GPUDevice* get_device() { return device; }
	inline SDL_GPUTexture* get_depth_texture() { return depth_texture; }
	inline SDL_GPUTexture* get_fallback_texture() { return fallback_texture; }

private:
	SDL_GPUSampler* default_sampler = nullptr;

	SDL_GPUDevice* device = nullptr;
	SDL_GPUCommandBuffer* command_buffer = nullptr;

	SDL_GPUTexture* swapchain_texture = nullptr;
	SDL_GPUTexture* depth_texture = nullptr;
	SDL_GPUTexture* fallback_texture = nullptr;

	SDL_GPURenderPass* render_pass = nullptr;
	SDL_Window* rend_window = nullptr;

	ImDrawData* draw_data = nullptr;

	Camera* camera_inst = nullptr;
	Input* input_inst = nullptr;

	MeshManager mesh_mgr{};

	SDL_GPUColorTargetInfo color_target_info{};
	SDL_GPUColorTargetInfo color_target_info_imgui{};
	SDL_GPUDepthStencilTargetInfo depth_target_info{};

	bool create_device();
	bool create_command_buffer();
	bool init_fallback_texture();
};
