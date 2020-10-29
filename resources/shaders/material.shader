#shader vertex
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;
layout(location = 5) in ivec4 boneIDs;
layout(location = 6) in vec4 boneWeights;

uniform mat4 u_m;
uniform mat4 u_vp;

uniform mat4 lightSpaceMatrix;

uniform vec3 lightPos;
uniform vec3 viewPos;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

out VS_OUT
{
	vec2 texCoord;
	vec3 worldPos;
	vec3 tangentLightPos;
	vec3 tangentViewPos;
	vec3 tangentWorldPos;

	vec4 fragPosLightSpace;
} vs_out;

void main()
{
	
	vec4 localPos = vec4(0.0);
	for(int i = 0; i < 4; i++) {
		mat4 boneTransform = gBones[boneIDs[i]];
		vec4 posePosition = boneTransform * vec4(pos, 1.0);
		localPos += posePosition * boneWeights[i];

		// todo: apply transformation to normal as well
	}

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

	vec4 worldPos = u_m * localPos;

	vs_out.fragPosLightSpace = lightSpaceMatrix * worldPos;

	gl_Position = u_vp * worldPos;
}

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normals;
	sampler2D emissive;
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

	vec4 fragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;

uniform Material material;
uniform Light light;

float calculateShadow(vec4 fragPosLightSpace)
{
	vec3 screenSpacePos = fragPosLightSpace.xyz / fragPosLightSpace.w;
	screenSpacePos = screenSpacePos * 0.5 + 0.5;

	if(screenSpacePos.z > 1.0)
	{
		return 0.0;
	}

	float minDepth = texture(shadowMap, screenSpacePos.xy).r;
	float fragDepth = screenSpacePos.z - 0.005;

	

	// return step(fragDepth,minDepth );
	return fragDepth > minDepth  ? 1.0 : 0.0;
}

void main()
{
	vec3 color = texture(material.diffuse, fs_in.texCoord).rgb;

	// normal
	vec3 normal = texture(material.normals, fs_in.texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	// ambient
	vec3 ambient = light.ambient * color; 
	
	// diffuse
	vec3 lightDir = normalize(fs_in.tangentLightPos - fs_in.tangentWorldPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse  = light.diffuse  * diff;  

	// spec
	vec3 viewdir = normalize(fs_in.tangentViewPos - fs_in.tangentWorldPos);
	vec3 reflectdir = reflect(-lightDir, normal);
	vec3 halfwaydir = normalize(lightDir + viewdir);
	float spec = pow(max(dot(normal, halfwaydir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, fs_in.texCoord).rgb;

	vec3 emissive = texture(material.emissive, fs_in.texCoord).rgb;

	float shadow = calculateShadow(fs_in.fragPosLightSpace);
	float visible = 1.0 - shadow;

	vec3 result = emissive + (ambient + visible * (diffuse + specular)) * color; 
	
	outColor = vec4(result, 1.0f);
}
