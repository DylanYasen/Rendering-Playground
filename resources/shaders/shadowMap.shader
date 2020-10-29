#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 5) in ivec4 boneIDs;
layout(location = 6) in vec4 boneWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 u_m;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

void main()
{
	// skinning
	vec4 localPos = vec4(pos, 1.0);
	if(boneWeights[0] > 0.0)
	{
		localPos = vec4(0.0);
		for(int i = 0; i < 4; i++) {
			mat4 boneTransform = gBones[boneIDs[i]];
			vec4 posePosition = boneTransform * vec4(pos, 1.0);
			localPos += posePosition * boneWeights[i];

			// todo: apply transformation to normal as well
		}
	}	

	gl_Position = lightSpaceMatrix * u_m * localPos;
}

#shader fragment
#version 330 core

void main()
{
}
