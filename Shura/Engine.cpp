#include "Engine.h"
#include <chrono>

bool Engine::init()
{
    /* make window */
    window = SDL_CreateWindow(
        engine_name, window_width, window_height, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        LogError("Failed to create window: {}", SDL_GetError());
        return false;
    }
    Log("Window created");

    /* init renderer */
    if (!renderer_inst.init(window, &input_inst.camera_inst, &input_inst)) {
        LogError("Failed to init renderer");
        return false;
    }

    /* select window for device */
    SDL_ClaimWindowForGPUDevice(renderer_inst.get_device(), window);
    Log("Window selected for device");

    /* imgui context */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    /* imgui example code 💀 */
    ImGui::StyleColorsDark();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale);
    style.FontScaleDpi
        = main_scale;
    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = renderer_inst.get_device();
    init_info.ColorTargetFormat
        = SDL_GetGPUSwapchainTextureFormat(renderer_inst.get_device(), window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

    /* swapchain params */
    SDL_SetGPUSwapchainParameters(renderer_inst.get_device(), window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);

    if (!shader_inst.load_shaders("shaders/spv/vertex.spv", "shaders/spv/fragment.spv",
        renderer_inst.get_device())) {
        LogError("Failed to load shaders");
        return false;
    }
    Log("Shaders loaded");

    if (!shader_inst.setup_pipeline(renderer_inst.get_device(), window)) {
        LogError("Failed to setup graphics pipeline");
        return false;
    }
    Log("Pipeline initialized");

    input_inst.camera_inst.init(
        glm::vec3(
            0.0f, 100.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        0.0f,
        0.0f,
        80.0f,
        (float)window_width / (float)window_height,
        0.1f,
        100000.0f
    );

    return true;
}

bool Engine::poll_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        input_inst.handle_event(event, window);

        switch (event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return false;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                return false;
        }
    }
    return true;
}

void Engine::run()
{
    Log("Entered main loop");

    using clock = std::chrono::high_resolution_clock;
    auto last_log_time = clock::now();
    int frame_count = 0;
    float lastTime = SDL_GetTicks() / 1000.0f;

    SDL_SetWindowMouseGrab(window, true);
    SDL_SetWindowRelativeMouseMode(window, true);

    while (running) {
        float currentTime = SDL_GetTicks() / 1000.0f;
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!poll_events())
            break;

        input_inst.update_camera(deltaTime);

        renderer_inst.begin_frame();
        renderer_inst.draw(shader_inst.get_pipeline());
        renderer_inst.end_frame();

        frame_count++;

        auto now = clock::now();
        std::chrono::duration<float> elapsed = now - last_log_time;

        if (elapsed.count() >= 0.5f) {
            renderer_inst.fps = frame_count / elapsed.count();

            frame_count = 0;
            last_log_time = now;
        }
    }

    SDL_SetWindowMouseGrab(window, false);
    SDL_SetWindowRelativeMouseMode(window, false);
}

void Engine::shutdown()
{
    Log("Shutdown triggered");

    SDL_WaitForGPUIdle(renderer_inst.get_device());
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    /* yes i made this with a migraine, how did you know? */
    SDL_ReleaseGPUBuffer(
        renderer_inst.get_device(), renderer_inst.mesh_inst.get_vertex_buffer());
    SDL_ReleaseGPUBuffer(
        renderer_inst.get_device(), renderer_inst.mesh_inst.get_index_buffer());
    SDL_ReleaseGPUTransferBuffer(
        renderer_inst.get_device(), renderer_inst.mesh_inst.get_transfer_buffer());

    SDL_ReleaseGPUShader(
        renderer_inst.get_device(), shader_inst.get_vertex_shader());
    SDL_ReleaseGPUShader(
        renderer_inst.get_device(), shader_inst.get_fragment_shader());

    if (renderer_inst.get_depth_texture())
        SDL_ReleaseGPUTexture(
            renderer_inst.get_device(), renderer_inst.get_depth_texture());

    if (shader_inst.get_pipeline())
        SDL_ReleaseGPUGraphicsPipeline(
            renderer_inst.get_device(), shader_inst.get_pipeline());

    if (renderer_inst.get_device())
        SDL_DestroyGPUDevice(renderer_inst.get_device());

    SDL_DestroyWindow(window);
}
