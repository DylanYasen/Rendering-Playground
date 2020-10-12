#shader vertex
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;

uniform mat4 u_m;
uniform mat4 u_mvp;

uniform vec3 lightPos;
uniform vec3 viewPos;

out VS_OUT
{
	vec2 texCoord;
	vec3 worldPos;
	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentWorldPos;
} vs_out;

void main()
{
	vs_out.worldPos = vec3(u_m * vec4(pos, 1.0));
	vs_out.texCoord = texCoord;

	// todo: Gram-Schmidt process 
	vec3 T = normalize(vec3(u_m * vec4(tangent,   0.0)));
   	vec3 B = normalize(vec3(u_m * vec4(bitangent, 0.0)));
   	vec3 N = normalize(vec3(u_m * vec4(normal,    0.0)));
	mat3 TBN = transpose(mat3(T, B, N));   
	vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos  = TBN * viewPos;
    vs_out.tangentWorldPos = TBN * vs_out.worldPos;

	gl_Position = u_mvp * vec4(pos, 1.0f);
}

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normals;
	float     shininess;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 0) out vec4 outColor;

in VS_OUT
{
	vec2 texCoord;
	vec3 worldPos;
	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentWorldPos;
} fs_in;

uniform Material material;
uniform Light light;

void main()
{
	// normal
	vec3 normal = texture(material.normals, fs_in.texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 ambient  = light.ambient  * texture(material.diffuse, fs_in.texCoord).rgb;

	// diffuse
	vec3 lightDir = normalize(fs_in.tangentLightPos - fs_in.tangentWorldPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse  = light.diffuse  * diff * texture(material.diffuse, fs_in.texCoord).rgb;  
	
	// sepc
	vec3 viewdir = normalize(fs_in.tangentViewPos - fs_in.tangentWorldPos);
	vec3 reflectdir = reflect(-lightDir, normal);
	vec3 halfwaydir = normalize(lightDir + viewdir);
	float spec = pow(max(dot(normal, halfwaydir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoord).rgb;

	vec3 result = ambient + diffuse + specular; 
	outColor = vec4(result, 1.0f);
}
