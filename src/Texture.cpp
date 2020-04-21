#include "Texture.h"
#include "gl.h"
#include "stb_image.h"

Texture::Texture(const char* path, const char* type)
	:rendererID(0), filepath(path), buffer(nullptr),
	width(0), height(0), bpp(0), type(type)
{
	stbi_set_flip_vertically_on_load(1);
	buffer = stbi_load(path, &width, &height, &bpp, 4);

	GLCall(glGenTextures(1, &rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	if (buffer)
	{
		stbi_image_free(buffer);
	}
	else
	{
		printf("failed to load texture: %s\n", path);
	}
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &rendererID));
}

void Texture::Bind(unsigned int slot /*= 0*/) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
