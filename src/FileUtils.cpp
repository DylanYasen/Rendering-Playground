#include "FileUtils.h"
#include "assimp/Importer.hpp"
#include <assimp/postprocess.h>

#include <fstream>

Assimp::Importer importer;

const aiScene* FileUtil::LoadFBX(const char* filepath)
{
	// check if file exists
	std::ifstream stream(filepath);
	if (stream.fail())
	{
		printf("failed to import %.15s, file doesn't exist\n", filepath);
		return NULL;
	}
	else
	{
		stream.close();
	}


	const aiScene* scene = importer.ReadFile(filepath, aiProcessPreset_TargetRealtime_Quality);

	if (!scene)
	{
		printf("failed to import %.15s, %s\n", filepath, importer.GetErrorString());
		return NULL;
	}
	
	return scene;
}
