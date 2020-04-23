#include "Texture.h"
#include "gl.h"
#include "stb_image.h"

Texture::Texture(const std::string& path, const std::string& textureType)
	:rendererID(0), filepath(path), type(textureType),
	buffer(nullptr), width(0), height(0), bpp(0)
{
	stbi_set_flip_vertically_on_load(1);
	buffer = stbi_load(filepath.c_str(), &width, &height, &bpp, 4);

	GLCall(glGenTextures(1, &rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	for (size_t i = 0; i < type.length(); ++i)
		type[i] = tolower(type[i]);
	materialTypeName = "material." + type;

	if (buffer)
	{
		printf("loaded [%s] texture: [%s] \n", textureType.c_str(), filepath.c_str());
		stbi_image_free(buffer);
	}
	else
	{
		printf("failed to load texture: %s\n", filepath.c_str());
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
