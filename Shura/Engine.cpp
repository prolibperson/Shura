#include "Engine.h"
#include <chrono>
//#include "SDL3\SDL_gpu.h"

bool Engine::init()
{
    /* make window */
    window = SDL_CreateWindow(engine_name, window_width, window_height, SDL_WINDOW_RESIZABLE);
    Log("Window created");

    /* init renderer */
    if (!renderer_inst.init(window)) 
    {
        LogError("Failed to init renderer");
        return false;
    }

    /* select window for device */
    SDL_ClaimWindowForGPUDevice(renderer_inst.get_device(), window);
    Log("Window selected for device");

    /* swapchain params */
    SDL_SetGPUSwapchainParameters(renderer_inst.get_device(), window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        SDL_GPU_PRESENTMODE_IMMEDIATE);

	if (!shader_inst.load_shaders("shaders/vertex.spv", "shaders/fragment.spv", renderer_inst.get_device()))
	{
		LogError("Failed to load shaders");
		return false;
	}
	Log("Shaders loaded");

	if (!shader_inst.setup_pipeline(renderer_inst.get_device(), window))
	{
		LogError("Failed to setup graphics pipeline");
		return false;
	}
    Log("Graphics pipeline setup complete");

    return true;
}

bool Engine::poll_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            return false;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode = SDL_SCANCODE_ESCAPE) return false;
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

    while (running)
    {
        if (!poll_events())
            break;

        renderer_inst.begin_frame();
        renderer_inst.draw(shader_inst.get_pipeline());
        renderer_inst.end_frame();

        frame_count++;

        auto now = clock::now();
        std::chrono::duration<float> elapsed = now - last_log_time;

        if (elapsed.count() >= 0.5f)
        {
            float fps = frame_count / elapsed.count();
            Log("FPS: " + std::to_string(fps));

            frame_count = 0;
            last_log_time = now;
        }
    }
}

void Engine::shutdown()
{
    Log("Shutdown triggered");

    /* yes i made this with a migraine, how did you know? */
    SDL_ReleaseGPUBuffer(renderer_inst.get_device(), renderer_inst.mesh_inst.get_vertex_buffer());
    SDL_ReleaseGPUTransferBuffer(renderer_inst.get_device(), renderer_inst.mesh_inst.get_transfer_buffer());

    SDL_ReleaseGPUShader(renderer_inst.get_device(), shader_inst.get_vertex_shader());
    SDL_ReleaseGPUShader(renderer_inst.get_device(), shader_inst.get_fragment_shader());

    SDL_ReleaseGPUGraphicsPipeline(renderer_inst.get_device(), shader_inst.get_pipeline());

    SDL_DestroyGPUDevice(renderer_inst.get_device());
    SDL_DestroyWindow(window);
}
