#shader vertex
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out VS_OUT
{
    vec2 uv;
} vs_out;

void main()
{
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0); 

    vs_out.uv = uv;
}

#shader fragment
#version 330 core

in VS_OUT
{
    vec2 uv;
} fs_in;

uniform sampler2D depthMap; 

layout(location = 0) out vec4 outColor;

void main()
{
    float depth = texture(depthMap, fs_in.uv).r;
    outColor = vec4(vec3(depth), 1.0);
}
