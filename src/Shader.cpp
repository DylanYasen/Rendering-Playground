#include "Shader.h"
#include "gl.h"
#include <fstream>
#include <sstream>

Shader::Shader(const std::string& filepath)
	: filepath(filepath)
{
	const ShaderSource& shaderSrc = ParseShader(filepath);
	rendererID = CreateShader(shaderSrc);
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(rendererID));
}

static void printShaderLog(unsigned int shader)
{
	if (glIsShader(shader))
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		GLCall(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

		char* infoLog = (char*)_malloca(maxLength * sizeof(char));
		GLCall(glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog));
		if (infoLogLength > 0)
		{
			printf("%s\n", infoLog);
		}
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

ShaderSource Shader::ParseShader(const std::string& filename)
{
	enum class ShaderType
	{
		None = -1,
		Vertex = 0,
		Fragment = 1
	};

	std::ifstream stream(filename);
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::None;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::Vertex;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::Fragment;
			}
		}
		else
		{
			if (type != ShaderType::None)
			{
				ss[(int)type] << line << "\n";
			}
		}
	}

	ShaderSource src;
	src.vertex = ss[(int)ShaderType::Vertex].str();
	src.fragment = ss[(int)ShaderType::Fragment].str();

	return src;
}

unsigned int Shader::CreateShader(const struct ShaderSource& source)
{
	GLCall(unsigned int programID = glCreateProgram());

	unsigned int vertexShader = CompileShader(source.vertex.c_str(), GL_VERTEX_SHADER);
	if (vertexShader == NULL) return false;

	unsigned int fragShader = CompileShader(source.fragment.c_str(), GL_FRAGMENT_SHADER);
	if (fragShader == NULL) return false;

	GLCall(glAttachShader(programID, vertexShader));
	GLCall(glAttachShader(programID, fragShader));

	GLCall(glLinkProgram(programID));
	GLCall(glValidateProgram(programID));

	int programSuccess = GL_TRUE;
	GLCall(glGetProgramiv(programID, GL_LINK_STATUS, &programSuccess));
	if (programSuccess != GL_TRUE)
	{
		printf("Error linking program %d\n", programID);
		return 0;
	}

	return programID;
}

unsigned int Shader::CompileShader(const char* shaderSrc, unsigned int type)
{
	GLCall(unsigned int shader = glCreateShader(type));
	GLCall(glShaderSource(shader, 1, &shaderSrc, NULL));
	GLCall(glCompileShader(shader));

	int compiled = GL_FALSE;
	GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled));
	if (compiled != GL_TRUE)
	{
		printf("Unable to compile shader %d!\n", shader);
		PrintShaderLog(shader);
		return NULL;
	}
	else
	{
		return shader;
	}
}

void Shader::PrintShaderLog(unsigned int shader)
{
	if (glIsShader(shader))
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		GLCall(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

		char* infoLog = (char*)_malloca(maxLength * sizeof(char));
		GLCall(glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog));
		if (infoLogLength > 0)
		{
			printf("%s\n", infoLog);
		}
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

void Shader::Bind() const
{
	GLCall(glUseProgram(rendererID));
}

void Shader::Unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int v0)
{
	GLCall(glUniform1i(GetUniformLocation(name), v0));
}

void Shader::SetUniform1f(const std::string& name, float v0)
{
	GLCall(glUniform1f(GetUniformLocation(name), v0));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
	GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const hmm_mat4& m)
{
	// need to transpose if the matrix is row major
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &m.Elements[0][0]));
}

int Shader::GetUniformLocation(const std::string& name)
{
	int location = -1;
	if (uniformCache.find(name) != uniformCache.end())
	{
		return uniformCache[name];
	}
	else
	{
		GLCall(location = glGetUniformLocation(rendererID, name.c_str()));
	}

	if (location == -1)
	{
		printf("failed to find uniform [%s] in [%s]\n", name.c_str(), filepath.c_str());
	}

	uniformCache[name] = location;
	return location;
}
