#include "Renderer.h"
#include "Vertex.h"

bool Renderer::create_device()
{
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, isDebug, NULL);
	if (device == NULL)
	{
		return false;
	}
	return true;
}

bool Renderer::create_command_buffer()
{
	command_buffer = SDL_AcquireGPUCommandBuffer(device);
	if (command_buffer == NULL)
	{
		return false;
	}
	return true;
}

bool Renderer::create_vertex_buffer()
{
	buffer_info.size = sizeof(vertices);
	buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
	if (vertex_buffer == NULL)
	{
		return false;
	}
	return true;
}

bool Renderer::init(SDL_Window* window)
{
	rend_window = window;

	/* make device */
	if (!create_device())
	{
		LogError("Failed to create device");
		return false;
	}
	Log("Device created");

	/* make cmd buffer */
	if (!create_command_buffer())
	{
		LogError("Failed to create command buffer");
		return false;
	}
	Log("Command buffer created");

	/* make vertex buffer */
	if (!create_vertex_buffer())
	{
		LogError("Failed to create vertex buffer");
		return false;
	}
	Log("Vertex buffer created");

	return true;
}

bool Renderer::begin_frame()
{
	/* get command buff */
	command_buffer = SDL_AcquireGPUCommandBuffer(device);

	/* get swapchain texture */
	Uint32 width, height;
	SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, rend_window, &swapchain_texture, &width, &height);

	/* end frame if no texture */
	if (swapchain_texture == NULL)
	{
		SDL_SubmitGPUCommandBuffer(command_buffer);
		return true;
	}

	/* color target */
	color_target_info.clear_color = { 240 / 255.0f, 240 / 255.0f, 240 / 255.0f, 255 / 255.0f };
	color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
	color_target_info.store_op = SDL_GPU_STOREOP_STORE;
	color_target_info.texture = swapchain_texture;

	/* start rend pass */
	render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);

	return true;
}

bool Renderer::end_frame()
{
	/* end rend pass */
	SDL_EndGPURenderPass(render_pass);

	/* submit to command buff*/
	SDL_SubmitGPUCommandBuffer(command_buffer);

	return true;
}
