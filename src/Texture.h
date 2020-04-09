#pragma once

class Texture
{
private:
	unsigned int rendererID;
	const char* filepath;
	unsigned char* buffer;
	int width, height;
	int bpp;  // bits per pixel

public:
	Texture(const char* path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return width; };
	inline int GetHeight() const { return height; };
};