#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_m;
uniform mat4 u_v;
uniform mat4 u_p;

void main()
{
	gl_Position = u_p * u_v * u_m * vec4(pos, 1.0f);
	v_texCoord = texCoord;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 v_texCoord;

uniform sampler2D u_tex;

void main()
{
	vec4 texColor = texture(u_tex, v_texCoord);
	outColor = texColor;
}
