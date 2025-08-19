#include "Mesh.h"

bool Mesh::create_transfer_buffer(SDL_GPUDevice* device)
{
	transfer_info.size = sizeof(vertices) + sizeof(indices);
	transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

	return transfer_buffer != nullptr;
}

bool Mesh::create_vertex_buffer(SDL_GPUDevice* device)
{
	buffer_info.size = sizeof(vertices);
	buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);

	return vertex_buffer != nullptr;
}

bool Mesh::create_index_buffer(SDL_GPUDevice* device)
{
	index_info.size = sizeof(indices);
	index_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	index_buffer = SDL_CreateGPUBuffer(device, &index_info);

	return index_buffer != nullptr;
}

bool Mesh::make_mesh(SDL_GPUDevice* device)
{
    uint8_t* mapped = (uint8_t*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);

    SDL_memcpy(mapped, vertices, sizeof(vertices));

    SDL_memcpy(mapped + sizeof(vertices), indices, sizeof(indices));

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation vertex_location{};
    vertex_location.transfer_buffer = transfer_buffer;
    vertex_location.offset = 0;

    SDL_GPUBufferRegion vertex_region{};
    vertex_region.buffer = vertex_buffer;
    vertex_region.size = sizeof(vertices);
    vertex_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &vertex_location, &vertex_region, true);

    SDL_GPUTransferBufferLocation index_location{};
    index_location.transfer_buffer = transfer_buffer;
    index_location.offset = sizeof(vertices);

    SDL_GPUBufferRegion index_region{};
    index_region.buffer = index_buffer;
    index_region.size = sizeof(indices);
    index_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &index_location, &index_region, true);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    return true;
}