#include "Renderer.h"
#include <SDL3_image/SDL_image.h>
#include "UniformBuffers.h"
#include <future>

bool Renderer::create_device()
{
    device = SDL_CreateGPUDevice(
		SDL_ShaderCross_GetSPIRVShaderFormats(), isDebug, NULL);
    return device != nullptr;
}

bool Renderer::create_command_buffer()
{
    command_buffer = SDL_AcquireGPUCommandBuffer(device);
    return command_buffer != nullptr;
}

SDL_GPUTexture* Renderer::load_texture(const char* path)
{
	Log("Path: {}", path);
	SDL_Surface* surface = IMG_Load(path);
	if (!surface)
	{
		LogError("Failed to load texture");
		return nullptr;
	}

#ifdef _DEBUG
    Log("Loaded surface: {}x{}", surface->w, surface->h);
#endif

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.width = surface->w;
    texture_info.height = surface->h;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &texture_info);

	SDL_GPUTransferBufferCreateInfo upload_buffer_info = {};
	upload_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	upload_buffer_info.size = (size_t)surface->w * surface->h * 4;
	SDL_GPUTransferBuffer* upload_buffer = SDL_CreateGPUTransferBuffer(device, &upload_buffer_info);

	void* mapped = SDL_MapGPUTransferBuffer(device, upload_buffer, false);
	SDL_ConvertPixels(surface->w, surface->h, surface->format, surface->pixels, surface->pitch, SDL_PIXELFORMAT_RGBA32, mapped, surface->w * 4);
	SDL_UnmapGPUTransferBuffer(device, upload_buffer);

	SDL_DestroySurface(surface);

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
	SDL_GPUTextureTransferInfo transfer_info = {};
	transfer_info.transfer_buffer = upload_buffer;
	transfer_info.offset = 0;
	transfer_info.pixels_per_row = 0;

	SDL_GPUTextureRegion texture_region = {};
	texture_region.texture = texture;
	texture_region.w = (uint32_t)texture_info.width;
	texture_region.h = (uint32_t)texture_info.height;
	texture_region.d = 1;

	SDL_UploadToGPUTexture(copy_pass, &transfer_info, &texture_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(cmd);

	return texture;
}

bool Renderer::create_sampler()
{
	SDL_GPUSamplerCreateInfo sampler_info{};
	sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
	sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
	sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	default_sampler = SDL_CreateGPUSampler(device, &sampler_info);
	return default_sampler != nullptr;
}

bool Renderer::create_depth_texture()
{
    SDL_GPUTextureCreateInfo depth_text_info{};
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
    ENSURE(create_device(), "Device created", "Failed to create device");

    /* make cmd buffer */
    ENSURE(create_command_buffer(), "Command buffer created", "Failed to create command buffer");

    /* load obj */
    ENSURE(mesh_inst.load_obj("Assets/sponza/sponza.obj"), "OBJ loaded", "Failed to load OBJ");

    /* make vertex buffer */
    ENSURE(mesh_inst.create_vertex_buffer(device), "Vertex buffer created", "Failed to create vertex buffer");

    /* make index buffer */
    ENSURE(mesh_inst.create_index_buffer(device), "Index buffer created", "Failed to create index buffer");

    /* make transfer buffer */
    ENSURE(mesh_inst.create_transfer_buffer(device), "Transfer buffer created", "Failed to create transfer buffer");

    /* make depth texture */
    ENSURE(create_depth_texture(), "Depth texture created", "Failed to create depth texture");

    /* make mesh */
    ENSURE(mesh_inst.make_mesh(device), "Mesh created", "Failed to create mesh");

    /* create sampler */
    ENSURE(create_sampler(), "Sampler created", "Failed to create sampler");

	/* load textures */
	for (auto& material : mesh_inst.materials)
	{
		if (material.has_diffuse_map)
		{
			material.diffuse_texture = load_texture(material.diffuse_map_path.c_str());
			material.diffuse_sampler = default_sampler;
		}
	}

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

    /* imgui windows (TODO: move this elsewhere, maybe some imguimanager class?) */
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

    /* draw submeshes */
	uint32_t first_index = 0;
	for (const auto& sub_mesh : mesh_inst.sub_meshes)
	{
		const auto& material = mesh_inst.materials[sub_mesh.material_index];
		if (material.has_diffuse_map && material.diffuse_texture != nullptr)
		{
			SDL_GPUTextureSamplerBinding texture_sampler_binding;
			texture_sampler_binding.texture = material.diffuse_texture;
			texture_sampler_binding.sampler = material.diffuse_sampler;
			SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_sampler_binding, 1);
		}

		SDL_DrawGPUIndexedPrimitives(
			render_pass,
			static_cast<Uint32>(sub_mesh.indices.size()),
			1,
			first_index,
			0,
			0
		);
		first_index += static_cast<Uint32>(sub_mesh.indices.size());
	}
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

void Renderer::cleanup()
{
    if (!device) return;

    SDL_WaitForGPUIdle(device);

    for (auto& mat : mesh_inst.materials)
    {
        if (mat.diffuse_sampler) { SDL_ReleaseGPUSampler(device, mat.diffuse_sampler); mat.diffuse_sampler = nullptr; }
        if (mat.diffuse_texture) { SDL_ReleaseGPUTexture(device, mat.diffuse_texture); mat.diffuse_texture = nullptr; }
        if (mat.ambient_sampler) { SDL_ReleaseGPUSampler(device, mat.ambient_sampler); mat.ambient_sampler = nullptr; }
        if (mat.ambient_texture) { SDL_ReleaseGPUTexture(device, mat.ambient_texture); mat.ambient_texture = nullptr; }
        if (mat.displacement_sampler) { SDL_ReleaseGPUSampler(device, mat.displacement_sampler); mat.displacement_sampler = nullptr; }
        if (mat.displacement_texture) { SDL_ReleaseGPUTexture(device, mat.displacement_texture); mat.displacement_texture = nullptr; }
        if (mat.alpha_sampler) { SDL_ReleaseGPUSampler(device, mat.alpha_sampler); mat.alpha_sampler = nullptr; }
        if (mat.alpha_texture) { SDL_ReleaseGPUTexture(device, mat.alpha_texture); mat.alpha_texture = nullptr; }
    }

    if (mesh_inst.get_vertex_buffer()) {
        SDL_ReleaseGPUBuffer(device, mesh_inst.get_vertex_buffer());
    }
    if (mesh_inst.get_index_buffer()) {
        SDL_ReleaseGPUBuffer(device, mesh_inst.get_index_buffer());
    }
    if (mesh_inst.get_transfer_buffer()) {
        SDL_ReleaseGPUTransferBuffer(device, mesh_inst.get_transfer_buffer());
    }

    if (depth_texture) {
        SDL_ReleaseGPUTexture(device, depth_texture);
        depth_texture = nullptr;
    }

    if (swapchain_texture) {
        SDL_ReleaseGPUTexture(device, swapchain_texture);
        swapchain_texture = nullptr;
    }

    if (default_sampler) {
        SDL_ReleaseGPUSampler(device, default_sampler);
        default_sampler = nullptr;
    }

    SDL_DestroyGPUDevice(device);
    device = nullptr;
}