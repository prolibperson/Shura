#include "MeshManager.h"

SDL_GPUTexture* MeshManager::load_texture(const char* path, SDL_GPUDevice* device)
{
    Log("Path: {}", path);
    SDL_Surface* surface = IMG_Load(path);
    if (!surface)
    {
        LogError("Failed to load texture");
        return nullptr;
    }

#ifdef _DEBUG
    Log("Loaded surface: {}x{}", surface->w, surface->h);
#endif

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.width = surface->w;
    texture_info.height = surface->h;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &texture_info);

    SDL_GPUTransferBufferCreateInfo upload_buffer_info = {};
    upload_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    upload_buffer_info.size = (size_t)surface->w * surface->h * 4;
    SDL_GPUTransferBuffer* upload_buffer = SDL_CreateGPUTransferBuffer(device, &upload_buffer_info);

    void* mapped = SDL_MapGPUTransferBuffer(device, upload_buffer, false);
    SDL_ConvertPixels(surface->w, surface->h, surface->format, surface->pixels, surface->pitch, SDL_PIXELFORMAT_RGBA32, mapped, surface->w * 4);
    SDL_UnmapGPUTransferBuffer(device, upload_buffer);

    SDL_DestroySurface(surface);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTextureTransferInfo transfer_info = {};
    transfer_info.transfer_buffer = upload_buffer;
    transfer_info.offset = 0;
    transfer_info.pixels_per_row = 0;

    SDL_GPUTextureRegion texture_region = {};
    texture_region.texture = texture;
    texture_region.w = (uint32_t)texture_info.width;
    texture_region.h = (uint32_t)texture_info.height;
    texture_region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &transfer_info, &texture_region, true);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);

	SDL_UnmapGPUTransferBuffer(device, transfer_info.transfer_buffer);
	transfer_info.transfer_buffer = nullptr;

    return texture;
}

bool MeshManager::load_mesh(const std::string& path, SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmd, SDL_GPUSampler* default_sampler)
{
    auto mesh = std::make_unique<Mesh>();

    if (!mesh->load_obj(path)) return false;
    if (!mesh->create_vertex_buffer(device)) return false;
    if (!mesh->create_index_buffer(device)) return false;
    if (!mesh->create_transfer_buffer(device)) return false;
    if (!mesh->make_mesh(device, cmd)) return false;

    for (auto& material : mesh->materials)
    {
        if (material.has_diffuse_map)
        {
            material.diffuse_texture = load_texture(material.diffuse_map_path.c_str(), device);
            material.diffuse_sampler = default_sampler;
        }
    }

    meshes.push_back(std::move(mesh));
    return true;
}

void MeshManager::render_all(SDL_GPURenderPass* pass, SDL_GPUTexture* fallback_texture, SDL_GPUSampler* default_sampler)
{
    uint32_t global_index_offset = 0;

    for (auto& mesh : meshes)
    {
        mesh->render(pass, global_index_offset, fallback_texture, default_sampler);
    }
}

void MeshManager::cleanup(SDL_GPUDevice* device)
{
    for (auto& mesh : meshes)
    {
        for (auto& mat : mesh->materials)
        {
            if (mat.diffuse_sampler) SDL_ReleaseGPUSampler(device, mat.diffuse_sampler);
            if (mat.diffuse_texture) SDL_ReleaseGPUTexture(device, mat.diffuse_texture);
            /* TODO: add all texture slots T-T */
        }

        if (mesh->get_vertex_buffer()) SDL_ReleaseGPUBuffer(device, mesh->get_vertex_buffer());
        if (mesh->get_index_buffer()) SDL_ReleaseGPUBuffer(device, mesh->get_index_buffer());
        if (mesh->get_transfer_buffer()) SDL_ReleaseGPUTransferBuffer(device, mesh->get_transfer_buffer());
    }

    meshes.clear();
}

Mesh* MeshManager::get_mesh(size_t index)
{
    if (index >= meshes.size()) return nullptr;
    return meshes[index].get();
}