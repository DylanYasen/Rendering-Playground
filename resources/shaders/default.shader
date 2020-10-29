#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

out VS_OUT
{
    vec2 uv;
    vec4 fragPosLightSpace;
} vs_out;

uniform mat4 u_m;
uniform mat4 u_vp;

uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 worldPos = u_m * vec4(pos, 1.0);

    vs_out.fragPosLightSpace = lightSpaceMatrix * worldPos;
    vs_out.uv = uv;

    gl_Position = u_vp * worldPos;
}

#shader fragment
#version 330 core

in VS_OUT
{
    vec2 uv;
    vec4 fragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;

layout(location = 0) out vec4 outColor;

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 screenSpacePos = fragPosLightSpace.xyz / fragPosLightSpace.w;
    screenSpacePos = screenSpacePos * 0.5 + 0.5;

    if(screenSpacePos.z > 1.0){
        return 0.0;    
    }
    
    
    float minDepth = texture(shadowMap, screenSpacePos.xy).r;
    float fragDepth = screenSpacePos.z - 0.005;
    
    return step(minDepth, fragDepth);
}

void main()
{
    vec3 color = vec3(1.0);

    float shadow = calculateShadow(fs_in.fragPosLightSpace);
    float visible = 1.0 - shadow;
    color *= visible;

    outColor = vec4(color, 1.0);
    
    // outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
