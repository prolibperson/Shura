#include "Mesh.h"
#include "fast_obj.h"
#include "UniformBuffers.h"
#include <fstream>
#include <future>
#include <execution>
#include <numeric>

bool Mesh::create_transfer_buffer(SDL_GPUDevice* device)
{
    transfer_info.size = static_cast<uint32_t>(
        vertices.size() * sizeof(Vertex) + get_index_count() * sizeof(uint32_t));
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

    return transfer_buffer != nullptr;
}

bool Mesh::create_vertex_buffer(SDL_GPUDevice* device)
{
    buffer_info.size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);

    return vertex_buffer != nullptr;
}

bool Mesh::create_index_buffer(SDL_GPUDevice* device)
{
    index_info.size = get_index_count() * sizeof(uint32_t);
    index_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer = SDL_CreateGPUBuffer(device, &index_info);

    return index_buffer != nullptr;
}

uint32_t Mesh::get_index_count()
{
    size_t count = 0;
    for (const auto& sub_mesh : sub_meshes)
    {
        count += sub_mesh.indices.size();
    }
    return static_cast<uint32_t>(count);
}

bool Mesh::load_obj(const std::string& path)
{
    Log("Loading OBJ at path {}", path);

    fastObjMesh* mesh = fast_obj_read(path.c_str());
    if (!mesh) {
        return false;
    }

    vertices.clear();
    materials.clear();
    sub_meshes.clear();

    for (unsigned int i = 0; i < mesh->material_count; i++) {
        const fastObjMaterial& mat = mesh->materials[i];
        Material m{};

        if (mat.name)
            strncpy_s(m.name, mat.name, sizeof(m.name) - 1);

        m.ambient = { static_cast<uint8_t>(mat.Ka[0] * 255.0f), static_cast<uint8_t>(mat.Ka[1] * 255.0f), static_cast<uint8_t>(mat.Ka[2] * 255.0f), 255 };
        m.diffuse = { static_cast<uint8_t>(mat.Kd[0] * 255.0f), static_cast<uint8_t>(mat.Kd[1] * 255.0f), static_cast<uint8_t>(mat.Kd[2] * 255.0f), 255 };
        m.specular = { static_cast<uint8_t>(mat.Ks[0] * 255.0f), static_cast<uint8_t>(mat.Ks[1] * 255.0f), static_cast<uint8_t>(mat.Ks[2] * 255.0f), 255 };

        m.shininess = mat.Ns;
        m.opacity = mat.d;
        m.refractive_index = mat.Ni;
        m.illumination_model = mat.illum;

        auto resolve_tex = [&](unsigned int idx) -> std::string {
            if (idx == 0 || idx >= mesh->texture_count) return "";
            return mesh->textures[idx].path ? std::string(mesh->textures[idx].path) : "";
            };

        m.ambient_map_path = resolve_tex(mat.map_Ka);
        m.has_ambient_map = !m.ambient_map_path.empty();

        m.diffuse_map_path = resolve_tex(mat.map_Kd);
        m.has_diffuse_map = !m.diffuse_map_path.empty();

        m.displacement_map_path = resolve_tex(mat.map_bump);
        m.has_displacement_map = !m.displacement_map_path.empty();

        m.alpha_map_path = resolve_tex(mat.map_d);
        m.has_alpha_map = !m.alpha_map_path.empty();

        m.is_transparent = m.opacity < 1.0f || m.has_alpha_map;

        materials.push_back(m);
    }

    /* default material :D */
    if (materials.empty())
    {
        Material default_mat{};
        strncpy_s(default_mat.name, "default", sizeof(default_mat.name) - 1);

        default_mat.diffuse = { 200, 200, 200, 255 };
        default_mat.ambient = { 50, 50, 50, 255 };
        default_mat.specular = { 255, 255, 255, 255 };
        default_mat.shininess = 32.0f;
        default_mat.opacity = 1.0f;
        default_mat.refractive_index = 1.0f;
        default_mat.illumination_model = 2;

        materials.push_back(default_mat);
	}

    sub_meshes.resize(materials.size());
    for (size_t i = 0; i < materials.size(); ++i)
    {
        sub_meshes[i].material_index = (uint32_t)i;
    }

    unsigned int index_offset = 0;
    for (unsigned int f = 0; f < mesh->face_count; ++f) {
        unsigned int fv = mesh->face_vertices[f];
        unsigned int mat_id = mesh->face_materials[f];

        if(mat_id >= sub_meshes.size())
			mat_id = 0;

        for (unsigned int v = 0; v < fv; ++v) {
            fastObjIndex idx = mesh->indices[index_offset + v];
            Vertex vert{};

            vert.x = mesh->positions[3 * idx.p + 0];
            vert.y = mesh->positions[3 * idx.p + 1];
            vert.z = mesh->positions[3 * idx.p + 2];

            if (mat_id < materials.size())
            {
                const auto& mat_diffuse = materials[mat_id].diffuse;
                vert.r = mat_diffuse.r / 255.0f;
                vert.g = mat_diffuse.g / 255.0f;
                vert.b = mat_diffuse.b / 255.0f;
                vert.a = materials[mat_id].opacity;
            }
            else
            {
                vert.r = vert.g = vert.b = 0.8f;
                vert.a = 1.0f;
            }

            if (idx.n > 0) {
                vert.nx = mesh->normals[3 * idx.n + 0];
                vert.ny = mesh->normals[3 * idx.n + 1];
                vert.nz = mesh->normals[3 * idx.n + 2];
            }
            else {
                vert.nx = vert.ny = 0.0f;
                vert.nz = 1.0f;
            }

            if (idx.t > 0) {
                vert.u = mesh->texcoords[2 * idx.t + 0];
                vert.v = 1.0f - mesh->texcoords[2 * idx.t + 1];
            }
            else {
                vert.u = vert.v = 0.0f;
            }

            vertices.push_back(vert);
            sub_meshes[mat_id].indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
        }
        index_offset += fv;
    }

    Log("Loaded {} vertices, {} submeshes, {} materials", vertices.size(),
        sub_meshes.size(), materials.size());

    fast_obj_destroy(mesh);
    return true;
}

bool Mesh::make_mesh(SDL_GPUDevice* device, SDL_GPUCommandBuffer* command_buffer)
{
    uint8_t* mapped = (uint8_t*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    if (!mapped)
        return false;

    size_t vertex_bytes = vertices.size() * sizeof(Vertex);
    SDL_memcpy(mapped, vertices.data(), vertex_bytes);

    size_t index_buffer_offset = vertex_bytes;
    for (const auto& sm : sub_meshes)
    {
        size_t bytes = sm.indices.size() * sizeof(uint32_t);
        if (bytes > 0)
        {
            SDL_memcpy(mapped + index_buffer_offset, sm.indices.data(), bytes);
            index_buffer_offset += bytes;
        }
    }

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    if (!command_buffer)
    {
        LogError("make_mesh called without a valid command buffer :(");
        return false;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation vertex_location{};
    vertex_location.transfer_buffer = transfer_buffer;
    vertex_location.offset = 0;

    SDL_GPUBufferRegion vertex_region{};
    vertex_region.buffer = vertex_buffer;
    vertex_region.size = static_cast<uint32_t>(vertex_bytes);
    vertex_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &vertex_location, &vertex_region, true);

    SDL_GPUTransferBufferLocation index_location{};
    index_location.transfer_buffer = transfer_buffer;
    index_location.offset = static_cast<uint32_t>(vertex_bytes);

    SDL_GPUBufferRegion index_region{};
    index_region.buffer = index_buffer;
    index_region.size = get_index_count() * sizeof(uint32_t);
    index_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &index_location, &index_region, true);

    SDL_EndGPUCopyPass(copy_pass);

    return true;
}

void Mesh::render(SDL_GPURenderPass* render_pass, uint32_t base_index_offset, SDL_GPUTexture* fallback_texture, SDL_GPUSampler* default_sampler)
{
    SDL_GPUBufferBinding buffer_bindings[1];
    buffer_bindings[0].buffer = get_vertex_buffer();
    buffer_bindings[0].offset = 0;
    SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);

    SDL_GPUBufferBinding index_bindings{};
    index_bindings.buffer = get_index_buffer();
    index_bindings.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass, &index_bindings, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    uint32_t first_index = 0;
    for (const auto& sub_mesh : sub_meshes)
    {
        const auto& mat = materials[sub_mesh.material_index];

        SDL_GPUTexture* tex = (mat.has_diffuse_map && mat.diffuse_texture) ? mat.diffuse_texture : fallback_texture;
        SDL_GPUSampler* samp = (mat.has_diffuse_map && mat.diffuse_sampler) ? mat.diffuse_sampler : default_sampler;

        SDL_GPUTextureSamplerBinding ts_binding{};
        ts_binding.texture = tex;
        ts_binding.sampler = samp;
        SDL_BindGPUFragmentSamplers(render_pass, 0, &ts_binding, 1);

        const uint32_t index_count = static_cast<uint32_t>(sub_mesh.indices.size());
        if (index_count == 0)
            continue;

        SDL_DrawGPUIndexedPrimitives(
            render_pass,
            index_count,
            1,
            first_index,
            0,
            0
        );
        first_index += index_count;
    }
}