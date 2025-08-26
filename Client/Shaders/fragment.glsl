#version 460

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_fragPos;

layout(location = 0) out vec4 frag_color;

layout(std140, set = 3, binding = 0) uniform frag_ubo {
    vec4 u_time;
};

void main()
{
    /* normal */
    vec3 N = normalize(v_normal);

    /* light dir in world space */
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));

    /* diffuse */
    float diffuse = max(dot(N, lightDir), 0.0);

    /* ambient */
    float ambient = 0.25;

    /* view direction */
    vec3 viewDir = normalize(-v_fragPos);

    /* specular */
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfDir), 0.0), 32.0);

    /* combine like goku and vegeta */
    vec3 lighting = v_color.rgb * (ambient + diffuse) + vec3(0.2) * spec;

    frag_color = vec4(lighting, v_color.a);
}
