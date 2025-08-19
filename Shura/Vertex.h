#pragma once
#include "Globals.h"

/* vertex input layout */
struct Vertex
{
    float x, y, z;      //vec3 position
    float r, g, b, a;   //vec4 color
};

/* vertices */
static Vertex vertices[]
{
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}     // bottom right vertex
};

/* indices */
static uint16_t indices[] =
{
    0, 1, 2
};