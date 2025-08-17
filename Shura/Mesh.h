#pragma once
#include "Globals.h"
#include "Vertex.h"

class Mesh
{
public:
	bool make_triangle(SDL_GPUDevice* device);

	bool create_transfer_buffer(SDL_GPUDevice* device);
	bool create_vertex_buffer(SDL_GPUDevice* device);

	inline Vertex* get_vert_data() { return vert_data; }
	inline SDL_GPUBuffer* get_vertex_buffer() { return vertex_buffer; }
	inline SDL_GPUTransferBuffer* get_transfer_buffer() { return transfer_buffer; }
private:
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;
	SDL_GPUBuffer* vertex_buffer = nullptr;
	Vertex* vert_data = nullptr;

	SDL_GPUTransferBufferCreateInfo transfer_info{};
	SDL_GPUBufferCreateInfo buffer_info{};
};

