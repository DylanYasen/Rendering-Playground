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
	gl_Position = vec4(pos, 0.0, 1.0);
	vs_out.uv = uv;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 outColor;

in VS_OUT
{
	vec2 uv;
} fs_in;

uniform sampler2D colorOutput; 

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;

	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 ReinhardTonemap(vec3 color)
{
	return color / (color + vec3(1.0));
}

vec3 ACESFilm(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);

	// vec3 result = clamp(texCol0.rgb - Density*(texCol1.rgb), 0.0, 1.0);
}

void main()
{
	vec3 hdrColor = texture(colorOutput, fs_in.uv).rgb;
	vec3 mapped = ReinhardTonemap(hdrColor);
	
	const float gamma = 2.2;
	mapped = pow(mapped, vec3(1.0 / gamma));
	
	outColor = vec4(mapped, 1.0f);
}
