#pragma once

#include <string>

//enum PixelFormat
//{
//	GL_RED,
//	GL_RG,
//	GL_RGB,
//	GL_BGR,
//	GL_RGBA,
//	GL_BGRA,
//	GL_RED_INTEGER,
//	GL_RG_INTEGER,
//	GL_RGB_INTEGER,
//	GL_BGR_INTEGER,
//	GL_RGBA_INTEGER,
//	GL_BGRA_INTEGER,
//	GL_STENCIL_INDEX,
//	GL_DEPTH_COMPONENT,
//	GL_DEPTH_STENCIL
//};

// class TextureBuilder
// {
// public:
// public
// 	TextureBuilder *width(unsigned int w);
// public
// 	TextureBuilder *height(unsigned int h);
// public
// 	TextureBuilder *levels(unsigned int level);
// public
// 	TextureBuilder *internalFormat(unsigned int w);
// public
// 	TextureBuilder *pixelFormat(unsigned int w);
// public
// 	TextureBuilder *pixelDataType(unsigned int w);
// };

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

	void InitDepthAttachment();
	void InitColor();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return width; };
	inline int GetHeight() const { return height; };
	inline const std::string &GetType() const { return type; };
	inline const std::string &GetMaterialTypeName() const { return materialTypeName; };
};
