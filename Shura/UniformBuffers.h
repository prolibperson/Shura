#pragma once
#include "Globals.h"
#include "glm/glm.hpp"

struct uniform_buffer_frag
{
    glm::vec4 time;
};

struct uniform_buffer_vert
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

static uniform_buffer_frag ubo_frag{};
static uniform_buffer_vert ubo_vert{};