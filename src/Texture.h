#pragma once

#include <string>

class Texture
{
	enum FilterOption
	{
		NEAREST,
		LINEAR
	};

	enum MipMapOption
	{
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR
	};

public:
	unsigned int rendererID;
	std::string filepath;
	std::string type;
	std::string materialTypeName;
	unsigned char *buffer;
	int width, height;
	int bpp; // bits per pixel

public:
	Texture(const std::string &path, const std::string &textureType = "none");
	Texture(unsigned int width, unsigned int height);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return width; };
	inline int GetHeight() const { return height; };
	inline const std::string &GetType() const { return type; };
	inline const std::string &GetMaterialTypeName() const { return materialTypeName; };
};
