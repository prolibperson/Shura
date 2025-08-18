#include "Renderer.h"
#include "UniformBuffers.h"

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
	if (!mesh_inst.create_vertex_buffer(device))
	{
		LogError("Failed to create vertex buffer");
		return false;
	}
	Log("Vertex buffer created");

	/* make transfer buffer */
	if (!mesh_inst.create_transfer_buffer(device))
	{
		LogError("Failed to create transfer buffer");
		return false;
	}
	Log("Transfer buffer created");

	/* triangle mesh */
	/* TODO: error checking cuz nothing returns false in the func ;-; */
	if (!mesh_inst.make_triangle(device))
	{
		LogError("Failed to create triangle mesh");
		return false;
	}
	Log("Triangle mesh created");

	return true;
}

void Renderer::begin_frame()
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
		return;
	}

	/* color target */
	color_target_info.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
	color_target_info.store_op = SDL_GPU_STOREOP_STORE;
	color_target_info.texture = swapchain_texture;

	/* start rend pass */
	render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);
}

void Renderer::draw(SDL_GPUGraphicsPipeline* graphics_pipeline)
{
	/* bind pipeline */
	SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);

	/* bind vert buffer */
	SDL_GPUBufferBinding bufferBindings[1];
	bufferBindings[0].buffer = mesh_inst.get_vertex_buffer();
	bufferBindings[0].offset = 0;
	SDL_BindGPUVertexBuffers(render_pass, 0, bufferBindings, 1);

	/* uniform buffer for pulsating effect */
	time_uniform.time = SDL_GetTicksNS() / 1e9f;
	SDL_PushGPUFragmentUniformData(command_buffer, 0, &time_uniform, sizeof(uniform_buffer));

	/* make draw call hello? hi! i would like to draw ok thanks */
	SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
}

void Renderer::end_frame()
{
	/* end rend pass */
	SDL_EndGPURenderPass(render_pass);

	/* submit to command buff*/
	SDL_SubmitGPUCommandBuffer(command_buffer);
}
