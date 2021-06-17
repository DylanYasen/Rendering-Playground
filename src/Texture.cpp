#include "Texture.h"
#include "gl.h"
#include "stb_image.h"
#include "Allocator.h"

static unsigned int usedTextureSlot = 0;

Texture::Texture(const std::string &path, const std::string &textureType)
	: rendererID(0), filepath(path), type(textureType),
	  buffer(nullptr), width(0), height(0), bpp(0)
{
	// todo: flipUV vs flipTexture
	stbi_set_flip_vertically_on_load(1);

	buffer = stbi_load(filepath.c_str(), &width, &height, &bpp, 4);

	GLCall(glGenTextures(1, &rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));

	// todo: parameterize these things
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
		// this is on the gpu, tracking by ID instead of the buffer address.
		{
			size_t size = width * height * bpp / 8;
			MemTracker::track(rendererID, EResourceType::Texture, size);
		}

		printf("loaded [%s] texture: [%s] \n", textureType.c_str(), filepath.c_str());
		stbi_image_free(buffer);
	}
	else
	{
		printf("failed to load texture: %s\n", filepath.c_str());
	}
}

Texture::Texture(unsigned int width, unsigned int height)
	: width(width), height(height)
{
	GLCall(glGenTextures(1, &rendererID));
}

void Texture::InitDepthAttachment()
{
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));

	// TODO: parameterize
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
						width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::InitColor()
{
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));

	// TODO: parameterize
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
						width, height, 0, GL_RGBA, GL_FLOAT, 0));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture()
{
	MemTracker::untrack(rendererID);
	GLCall(glDeleteTextures(1, &rendererID));
}

void Texture::Bind(unsigned int slot /*= 0*/) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, rendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
