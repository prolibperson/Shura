#pragma once
#include "Globals.h"

struct Material {
    char name[64];

    SDL_Color ambient;
    SDL_Color diffuse; 
    SDL_Color specular;
    float shininess;
    float opacity;
    float refractive_index;

    std::string diffuse_map_path;
    std::string ambient_map_path;
    std::string displacement_map_path;
    std::string alpha_map_path;

    SDL_GPUTexture* diffuse_texture = nullptr;
    SDL_GPUTexture* ambient_texture = nullptr;
    SDL_GPUTexture* displacement_texture = nullptr;
    SDL_GPUTexture* alpha_texture = nullptr;

    SDL_GPUSampler* diffuse_sampler = nullptr;
    SDL_GPUSampler* ambient_sampler = nullptr;
    SDL_GPUSampler* displacement_sampler = nullptr;
    SDL_GPUSampler* alpha_sampler = nullptr;

    int illumination_model;

    bool has_diffuse_map : 1;
    bool has_ambient_map : 1;
    bool has_displacement_map : 1;
    bool has_alpha_map : 1;
    bool is_transparent : 1;
};