#pragma once
#include "Globals.h"
#include "Material.h"
#include "Vertex.h"

struct sub_mesh
{
	uint32_t material_index;
	std::vector<uint32_t> indices;
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<sub_mesh> sub_meshes;
	std::vector<Material> materials;

	bool make_mesh(SDL_GPUDevice* device);

	bool create_transfer_buffer(SDL_GPUDevice* device);
	bool create_vertex_buffer(SDL_GPUDevice* device);
	bool create_index_buffer(SDL_GPUDevice* device);
	uint32_t get_index_count();

	bool load_obj(const std::string& path);

	inline Vertex* get_vert_data() { return vert_data; }
	inline SDL_GPUBuffer* get_vertex_buffer() { return vertex_buffer; }
	inline SDL_GPUBuffer* get_index_buffer() { return index_buffer; }
	inline size_t get_submesh_count() { return sub_meshes.size(); }
	inline SDL_GPUTransferBuffer* get_transfer_buffer() { return transfer_buffer; }

private:
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;

	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUBuffer* index_buffer = nullptr;

	Vertex* vert_data = nullptr;

	SDL_GPUTransferBufferCreateInfo transfer_info{};
	SDL_GPUBufferCreateInfo buffer_info{};
	SDL_GPUBufferCreateInfo index_info{};
};
