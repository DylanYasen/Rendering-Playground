#pragma once

#include <assimp/scene.h>

class FileUtil
{
public:
	static const aiScene* LoadModel(const char* filepath);
};
