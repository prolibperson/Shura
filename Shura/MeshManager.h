#pragma once
#include "Globals.h"
#include "Mesh.h"

class MeshManager
{
public:
	SDL_GPUTexture* load_texture(const char* path, SDL_GPUDevice* device);
	bool load_mesh(const std::string& path, SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmd, SDL_GPUSampler* default_sampler);
	void render_all(SDL_GPURenderPass* render_pass, SDL_GPUTexture* fallback_texture, SDL_GPUSampler* default_sampler);
	void cleanup(SDL_GPUDevice* device);

	Mesh* get_mesh(size_t index);

private:
	std::vector<std::unique_ptr<Mesh>> meshes;
};

