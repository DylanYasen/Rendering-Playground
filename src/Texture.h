#pragma once

class Texture
{
private:
	unsigned int rendererID;
	const char* filepath;
	const char* type;
    unsigned char* buffer;
	int width, height;
	int bpp;  // bits per pixel

public:
	Texture(const char* path, const char* type = "raw");
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return width; };
	inline int GetHeight() const { return height; };
};
