#include "Mesh.h"

bool Mesh::create_transfer_buffer(SDL_GPUDevice* device)
{
	transfer_info.size = sizeof(vertices);
	transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
	if (transfer_buffer == NULL)
	{
		return false;
	}
	return true;
}

bool Mesh::create_vertex_buffer(SDL_GPUDevice* device)
{
	buffer_info.size = sizeof(vertices);
	buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
	if (vertex_buffer == NULL)
	{
		return false;
	}
	return true;
}

bool Mesh::make_triangle(SDL_GPUDevice* device)
{
	vert_data = (Vertex*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);

	SDL_memcpy(vert_data, vertices, sizeof(vertices));

	SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

	SDL_GPUTransferBufferLocation location{};
	location.transfer_buffer = transfer_buffer;
	location.offset = 0;

	SDL_GPUBufferRegion region{};
	region.buffer = vertex_buffer;
	region.size = sizeof(vertices);
	region.offset = 0;

	SDL_UploadToGPUBuffer(copyPass, &location, &region, true);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);

	return true;
}