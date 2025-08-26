#pragma once
#include "Globals.h"

/* vertex input layout */
struct Vertex 
{
    float x, y, z;        // position
    float r, g, b, a;     // color
    float nx, ny, nz;     // normal
};