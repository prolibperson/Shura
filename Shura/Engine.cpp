#include "Engine.h"

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
    while (running)
    {
        if (!poll_events())
            break;

        renderer_inst.begin_frame();
        renderer_inst.end_frame();
    }
}

void Engine::shutdown()
{
    Log("Shutdown triggered");

    /* yes i made this with a migraine, how did you know? */
    SDL_ReleaseGPUBuffer(renderer_inst.get_device(), renderer_inst.mesh_inst.get_vertex_buffer());
    SDL_ReleaseGPUTransferBuffer(renderer_inst.get_device(), renderer_inst.mesh_inst.get_transfer_buffer());

    SDL_DestroyGPUDevice(renderer_inst.get_device());
    SDL_DestroyWindow(window);
}
