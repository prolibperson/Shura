#version 460

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_fragPos;
layout(location = 3) in vec2 v_texCoord;

layout(location = 0) out vec4 frag_color;

layout(set = 2, binding = 0) uniform sampler2D u_diffuse_texture;

layout(std140, set = 3, binding = 0) uniform frag_ubo {
    vec4 u_time;
};

void main()
{
    /* sample texture */
    vec4 tex_color = texture(u_diffuse_texture, v_texCoord);

    /* alpha clip */
    if (tex_color.a < 0.5)
        discard;

    /* normalized surface normal */
    vec3 N = normalize(v_normal);

    /* light dir */
    vec3 light_dir = normalize(vec3(0.3, 1.0, 0.5));

    /* diffuse and ambient terms */
    float diffuse = max(dot(N, light_dir), 0.0);
    float ambient = 0.25;

    /* view direction */
    vec3 view_dir = normalize(-v_fragPos);

    /* spec term */
    vec3 half_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(N, half_dir), 0.0), 32.0);

    /* combine */
    vec3 lighting = tex_color.rgb * (ambient + diffuse) + vec3(0.2) * spec;

    frag_color = vec4(lighting * tex_color.a, tex_color.a);
}
