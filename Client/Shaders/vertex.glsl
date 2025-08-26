#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec3 a_normal;

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec3 v_normal;
layout (location = 2) out vec3 v_fragPos;

layout(std140, set = 1, binding = 0) uniform vert_ubo {
    mat4 u_model;
    mat4 u_view;
    mat4 u_projection;
};

void main() {
    /* world pos */
    vec4 worldPos = u_model * vec4(a_position, 1.0);
    v_fragPos = worldPos.xyz;

    /* normal matrix */
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    v_normal = normalize(normalMatrix * a_normal);

    /* pass color */
    v_color = a_color;

    /* gl position */
    gl_Position = u_projection * u_view * worldPos;
}
