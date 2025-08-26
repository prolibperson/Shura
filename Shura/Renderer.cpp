#include "Renderer.h"
#include "UniformBuffers.h"

bool Renderer::create_device()
{
    device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, isDebug, NULL);
    return device != nullptr;
}

bool Renderer::create_command_buffer()
{
    command_buffer = SDL_AcquireGPUCommandBuffer(device);
    return command_buffer != nullptr;
}

bool Renderer::create_depth_texture()
{
    SDL_GPUTextureCreateInfo depth_text_info = {};
    depth_text_info.type = SDL_GPU_TEXTURETYPE_2D;
    depth_text_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    depth_text_info.width = window_width;
    depth_text_info.height = window_height;
    depth_text_info.layer_count_or_depth = 1;
    depth_text_info.num_levels = 1;
    depth_text_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    depth_text_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    depth_texture = SDL_CreateGPUTexture(device, &depth_text_info);

    return depth_texture != nullptr;
}

bool Renderer::init(SDL_Window* window, Camera* camera, Input* input)
{
    rend_window = window;
    camera_inst = camera;
    input_inst = input;

    /* make device */
    if (!create_device()) {
        LogError("Failed to create device");
        return false;
    }
    Log("Device created");

    /* make cmd buffer */
    if (!create_command_buffer()) {
        LogError("Failed to create command buffer");
        return false;
    }
    Log("Command buffer created");

    /* load obj */
    if (!mesh_inst.load_obj("Assets/sponza/sponza.obj")) {
        LogError("Failed to load OBJ");
        return false;
    }
    Log("OBJ loaded");

    /* make vertex buffer */
    if (!mesh_inst.create_vertex_buffer(device)) {
        LogError("Failed to create vertex buffer");
        return false;
    }
    Log("Vertex buffer created");

    /* make index buffer */
    if (!mesh_inst.create_index_buffer(device)) {
        LogError("Failed to create index buffer");
        return false;
    }
    Log("Index buffer created");

    /* make transfer buffer */
    if (!mesh_inst.create_transfer_buffer(device)) {
        LogError("Failed to create transfer buffer");
        return false;
    }
    Log("Transfer buffer created");

    /* make depth texture */
    if (!create_depth_texture()) {
        LogError("Failed to create depth texture");
        return false;
    }
    Log("Depth texture created");

    /* make mesh */
    if (!mesh_inst.make_mesh(device)) {
        LogError("Failed to create mesh");
        return false;
    }
    Log("Mesh created");

    return true;
}

void Renderer::begin_frame()
{
    /* get command buff */
    command_buffer = SDL_AcquireGPUCommandBuffer(device);

    /* get swapchain texture */
    Uint32 width, height;
    SDL_WaitAndAcquireGPUSwapchainTexture(
        command_buffer, rend_window, &swapchain_texture, &width, &height);

    /* end frame if no texture */
    if (swapchain_texture == NULL) {
        SDL_SubmitGPUCommandBuffer(command_buffer);
        return;
    }

    /* color target */
    color_target_info.clear_color = { 0.0f, 0.0f, 0.3f, 1.0f };
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;
    color_target_info.texture = swapchain_texture;

    /* color target for imgui */
    color_target_info_imgui.load_op = SDL_GPU_LOADOP_LOAD;
    color_target_info_imgui.store_op = SDL_GPU_STOREOP_STORE;
    color_target_info_imgui.texture = swapchain_texture;

    /* depth target */
    depth_target_info.texture = depth_texture;
    depth_target_info.clear_depth = 1.0f;
    depth_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target_info.store_op = SDL_GPU_STOREOP_STORE;
    depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

    /* start imgui frame */
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    /* imgui windows */
    {
        static bool show_settings = false;

        ImGui::Begin("Shura");
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Text("FPS: %f", fps);

        ImGui::Text("L-Ctrl to toggle cursor");

        if (ImGui::Button("Settings"))
            show_settings = !show_settings;

        ImGui::End();

        if (show_settings) {
            ImGui::Begin("Settings", &show_settings);
            ImGui::SliderFloat(
                "Sensitivity", &input_inst->MOUSE_SENSITIVITY, 0.0f, 1.0f);
            ImGui::SliderFloat("Movement Speed", &input_inst->MOVE_SPEED, 0.0f, 400.0f);
            ImGui::End();
        }
    }

    /* render imgui */
    ImGui::Render();
    draw_data = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

    /* start rend pass */
    render_pass = SDL_BeginGPURenderPass(
        command_buffer, &color_target_info, 1, &depth_target_info);
}

void Renderer::draw(SDL_GPUGraphicsPipeline* graphics_pipeline)
{
    /* bind pipeline */
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

    /* bind vert buffer */
    SDL_GPUBufferBinding buffer_bindings[1];
    buffer_bindings[0].buffer = mesh_inst.get_vertex_buffer();
    buffer_bindings[0].offset = 0;
    SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);

    /* bind index buffer */
    SDL_GPUBufferBinding index_bindings{};
    index_bindings.buffer = mesh_inst.get_index_buffer();
    index_bindings.offset = 0;
    SDL_BindGPUIndexBuffer(
        render_pass, &index_bindings, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    /* uniform buffers */

    /* frag */
    ubo_frag.time.x = static_cast<float>(SDL_GetTicksNS()) / 1e9f;

    /* vert */
    ubo_vert.view = camera_inst->get_view();
    ubo_vert.projection = camera_inst->get_projection();
    ubo_vert.model = camera_inst->get_model();

    /* push!! */
    SDL_PushGPUVertexUniformData(command_buffer, 0, &ubo_vert, sizeof(ubo_vert));
    SDL_PushGPUFragmentUniformData(command_buffer, 0, &ubo_frag, sizeof(ubo_frag));

    /* make draw call hello? hi! i would like to draw ok thanks */
    SDL_DrawGPUIndexedPrimitives(
        render_pass, mesh_inst.get_index_count(), 1, 0, 0, 0);
}

void Renderer::end_frame()
{
    /* end rend pass */
    SDL_EndGPURenderPass(render_pass);

    /* imgui pass */
    SDL_GPURenderPass* imgui_pass = SDL_BeginGPURenderPass(
        command_buffer, &color_target_info_imgui, 1, nullptr);
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, imgui_pass);
    SDL_EndGPURenderPass(imgui_pass);

    /* submit to command buff*/
    SDL_SubmitGPUCommandBuffer(command_buffer);
}