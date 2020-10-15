#pragma once

#include <string>
#include <unordered_map>
#include "Math.h"

struct ShaderSource
{
	std::string vertex;
	std::string fragment;
};

class Shader
{
private:
	unsigned int rendererID;
	std::string filepath;
	std::unordered_map<std::string, int> uniformCache;

public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform1i(const std::string& name, int v0);
	void SetUniform1f(const std::string& name, float v0);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform3f(const std::string& name, const float v[3]);
	void SetUniform3f(const std::string& name, const vec3& v);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const hmm_mat4& matrix);

private:
	int GetUniformLocation(const std::string& name);

	unsigned int CreateShader(const struct ShaderSource& src);
	unsigned int CompileShader(const char* shaderSrc, unsigned int type);
	struct ShaderSource ParseShader(const std::string& filepath);
	void PrintShaderLog(unsigned int shader);

};
