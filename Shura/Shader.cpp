#include "Shader.h"
#include "UniformBuffers.h"
#include "Vertex.h"

bool Shader::load_vertex(const char* filename, SDL_GPUDevice* device)
{
    size_t vertex_code_size;
    void* vertex_code = SDL_LoadFile(filename, &vertex_code_size);

    SDL_GPUShaderCreateInfo vertex_info{};
    vertex_info.code = (Uint8*)vertex_code;
    vertex_info.code_size = vertex_code_size;
    vertex_info.entrypoint = "main";
    vertex_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertex_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertex_info.num_samplers = 0;
    vertex_info.num_storage_buffers = 0;
    vertex_info.num_storage_textures = 0;
    vertex_info.num_uniform_buffers = 1;
    vertex_shader = SDL_CreateGPUShader(device, &vertex_info);

    SDL_free(vertex_code);

    return vertex_shader != nullptr;
}

bool Shader::load_fragment(const char* filename, SDL_GPUDevice* device)
{
    size_t fragment_code_size;
    void* fragment_code = SDL_LoadFile(filename, &fragment_code_size);

    SDL_GPUShaderCreateInfo fragment_info{};
    fragment_info.code = (Uint8*)fragment_code;
    fragment_info.code_size = fragment_code_size;
    fragment_info.entrypoint = "main";
    fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragment_info.num_samplers = 0;
    fragment_info.num_storage_buffers = 0;
    fragment_info.num_storage_textures = 0;
    fragment_info.num_uniform_buffers = 1;
    fragment_shader = SDL_CreateGPUShader(device, &fragment_info);

    SDL_free(fragment_code);

    return fragment_shader != nullptr;
}

bool Shader::setup_pipeline(SDL_GPUDevice* device, SDL_Window* window)
{
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};

    pipeline_info.vertex_shader = vertex_shader;
    pipeline_info.fragment_shader = fragment_shader;

    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
    vertex_buffer_descriptions[0].slot = 0;
    vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_descriptions[0].instance_step_rate = 0;
    vertex_buffer_descriptions[0].pitch = sizeof(Vertex);

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions
        = vertex_buffer_descriptions;

    SDL_GPUVertexAttribute vertex_attributes[3];

    /* position */
    vertex_attributes[0].buffer_slot = 0;
    vertex_attributes[0].location = 0;
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[0].offset = 0;

    /* color */
    vertex_attributes[1].buffer_slot = 0;
    vertex_attributes[1].location = 1;
    vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    vertex_attributes[1].offset = sizeof(float) * 3;

    /* normal */
    vertex_attributes[2].buffer_slot = 0;
    vertex_attributes[2].location = 2;
    vertex_attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[2].offset = sizeof(float) * 7;

    pipeline_info.vertex_input_state.num_vertex_attributes = 3;
    pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes;

    SDL_GPUColorTargetDescription color_target_descriptions[1];
    color_target_descriptions[0] = {};
    color_target_descriptions[0].blend_state.enable_blend = true;
    color_target_descriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_target_descriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    color_target_descriptions[0].blend_state.src_color_blendfactor
        = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_target_descriptions[0].blend_state.dst_color_blendfactor
        = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_target_descriptions[0].blend_state.src_alpha_blendfactor
        = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_target_descriptions[0].blend_state.dst_alpha_blendfactor
        = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_target_descriptions[0].format
        = SDL_GetGPUSwapchainTextureFormat(device, window);

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions
        = color_target_descriptions;

    pipeline_info.target_info.depth_stencil_format
        = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    pipeline_info.target_info.has_depth_stencil_target = true;

    SDL_GPUDepthStencilState depth_stencil{};
    depth_stencil.compare_op = SDL_GPU_COMPAREOP_LESS;
    depth_stencil.enable_depth_test = true;
    depth_stencil.enable_depth_write = true;

    pipeline_info.depth_stencil_state = depth_stencil;

    /* wireframe */
    // pipeline_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;

    graphics_pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);

    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, fragment_shader);

    return graphics_pipeline != nullptr;
}