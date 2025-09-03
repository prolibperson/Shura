#include "Mesh.h"
#include "fast_obj.h"
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

    sub_meshes.resize(materials.size());
    for (size_t i = 0; i < materials.size(); ++i)
    {
        sub_meshes[i].material_index = (uint32_t)i;
    }

    unsigned int index_offset = 0;
    for (unsigned int f = 0; f < mesh->face_count; ++f) {
        unsigned int fv = mesh->face_vertices[f];
        unsigned int mat_id = mesh->face_materials[f];

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

bool Mesh::make_mesh(SDL_GPUDevice* device)
{
    uint8_t* mapped
        = (uint8_t*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    if (!mapped)
        return false;

    SDL_memcpy(mapped, vertices.data(), vertices.size() * sizeof(Vertex));

    size_t index_buffer_offset = vertices.size() * sizeof(Vertex);
    for (const auto& sm : sub_meshes)
    {
        SDL_memcpy(mapped + index_buffer_offset, sm.indices.data(), sm.indices.size() * sizeof(uint32_t));
        index_buffer_offset += sm.indices.size() * sizeof(uint32_t);
    }

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation vertex_location{};
    vertex_location.transfer_buffer = transfer_buffer;
    vertex_location.offset = 0;

    SDL_GPUBufferRegion vertex_region{};
    vertex_region.buffer = vertex_buffer;
    vertex_region.size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
    vertex_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &vertex_location, &vertex_region, true);

    SDL_GPUTransferBufferLocation index_location{};
    index_location.transfer_buffer = transfer_buffer;
    index_location.offset = (uint32_t)(vertices.size() * sizeof(Vertex));

    SDL_GPUBufferRegion index_region{};
    index_region.buffer = index_buffer;
    index_region.size = get_index_count() * sizeof(uint32_t);
    index_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &index_location, &index_region, true);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    return true;
}