#pragma once

#include <string>

class Texture
{
private:
	unsigned int rendererID;
	std::string filepath;
	std::string type;
	std::string materialTypeName;
    unsigned char* buffer;
	int width, height;
	int bpp;  // bits per pixel

public:
	Texture(const std::string& path, const std::string& textureType = "none");
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return width; };
	inline int GetHeight() const { return height; };
	inline const std::string& GetType() const { return type; };
	inline const std::string& GetMaterialTypeName() const { return materialTypeName; };
};
