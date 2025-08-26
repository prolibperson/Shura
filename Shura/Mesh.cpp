#include "Mesh.h"
#include "tiny_obj_loader.h"
#include <fstream>

bool Mesh::create_transfer_buffer(SDL_GPUDevice* device)
{
    transfer_info.size = static_cast<Uint32>(
        vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t));
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

    return transfer_buffer != nullptr;
}

bool Mesh::create_vertex_buffer(SDL_GPUDevice* device)
{
    buffer_info.size = static_cast<Uint32>(vertices.size() * sizeof(Vertex));
    buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);

    return vertex_buffer != nullptr;
}

bool Mesh::create_index_buffer(SDL_GPUDevice* device)
{
    index_info.size = static_cast<Uint32>(indices.size() * sizeof(uint32_t));
    index_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer = SDL_CreateGPUBuffer(device, &index_info);

    return index_buffer != nullptr;
}

bool Mesh::load_obj(const std::string& path)
{
    Log("Loading OBJ at path {}", path);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials_tiny;
    std::string warn, err;

    std::string directory = path.substr(0, path.find_last_of("/\\") + 1);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials_tiny, &warn, &err,
        path.c_str(), directory.c_str())) {
        if (!warn.empty())
            Log("TinyObjLoader warning: {}", warn);
        if (!err.empty())
            Log("TinyObjLoader error: {}", err);
        return false;
    }

    if (!warn.empty())
        Log("TinyObjLoader warning: {}", warn);
    if (!err.empty())
        Log("TinyObjLoader error: {}", err);

    vertices.clear();
    indices.clear();
    materials.clear();

    // Convert TinyObj materials to our Material struct
    for (const auto& mat : materials_tiny) {
        Material m{};
        strncpy_s(m.name, mat.name.c_str(), sizeof(m.name) - 1);

        m.ambient = { Uint8(mat.ambient[0] * 255), Uint8(mat.ambient[1] * 255),
         Uint8(mat.ambient[2] * 255), 255 };
        m.diffuse = { Uint8(mat.diffuse[0] * 255), Uint8(mat.diffuse[1] * 255),
         Uint8(mat.diffuse[2] * 255), 255 };
        m.specular = { Uint8(mat.specular[0] * 255), Uint8(mat.specular[1] * 255),
         Uint8(mat.specular[2] * 255), 255 };

        m.shininess = mat.shininess;
        m.opacity = mat.dissolve;
        m.refractive_index = mat.ior;
        m.illumination_model = mat.illum;

        // Store relative texture paths
        m.diffuse_map_path
            = mat.diffuse_texname.empty() ? "" : directory + mat.diffuse_texname;
        m.has_diffuse_map = !m.diffuse_map_path.empty();

        m.ambient_map_path
            = mat.ambient_texname.empty() ? "" : directory + mat.ambient_texname;
        m.has_ambient_map = !m.ambient_map_path.empty();

        m.displacement_map_path = mat.displacement_texname.empty()
            ? ""
            : directory + mat.displacement_texname;
        m.has_displacement_map = !m.displacement_map_path.empty();

        m.alpha_map_path
            = mat.alpha_texname.empty() ? "" : directory + mat.alpha_texname;
        m.has_alpha_map = !m.alpha_map_path.empty();

        m.is_transparent = m.opacity < 1.0f || m.has_alpha_map;

        materials.push_back(m);
    }

    // Build vertex buffer and index buffer
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex v{};

            // Vertex position
            v.x = attrib.vertices[3 * index.vertex_index + 0];
            v.y = attrib.vertices[3 * index.vertex_index + 1];
            v.z = attrib.vertices[3 * index.vertex_index + 2];

            // Default color
            v.r = v.g = v.b = 0.8f;
            v.a = 1.0f;

            // Vertex normal
            if (index.normal_index >= 0) {
                v.nx = attrib.normals[3 * index.normal_index + 0];
                v.ny = attrib.normals[3 * index.normal_index + 1];
                v.nz = attrib.normals[3 * index.normal_index + 2];
            }
            else {
                v.nx = v.ny = 0.0f;
                v.nz = 1.0f;
            }

            vertices.push_back(v);
            indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
        }
    }

    Log("Loaded {} vertices, {} indices, {} materials", vertices.size(),
        indices.size(), materials.size());
    return true;
}

bool Mesh::make_mesh(SDL_GPUDevice* device)
{
    uint8_t* mapped
        = (uint8_t*)SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
    if (!mapped)
        return false;

    SDL_memcpy(mapped, vertices.data(), vertices.size() * sizeof(Vertex));

    SDL_memcpy(mapped + vertices.size() * sizeof(Vertex), indices.data(),
        indices.size() * sizeof(uint32_t));

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTransferBufferLocation vertex_location{};
    vertex_location.transfer_buffer = transfer_buffer;
    vertex_location.offset = 0;

    SDL_GPUBufferRegion vertex_region{};
    vertex_region.buffer = vertex_buffer;
    vertex_region.size = static_cast<Uint32>(vertices.size() * sizeof(Vertex));
    vertex_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &vertex_location, &vertex_region, true);

    SDL_GPUTransferBufferLocation index_location{};
    index_location.transfer_buffer = transfer_buffer;
    index_location.offset = static_cast<Uint32>(vertices.size() * sizeof(Vertex));

    SDL_GPUBufferRegion index_region{};
    index_region.buffer = index_buffer;
    index_region.size = static_cast<Uint32>(indices.size() * sizeof(uint32_t));
    index_region.offset = 0;

    SDL_UploadToGPUBuffer(copy_pass, &index_location, &index_region, true);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);

    return true;
}