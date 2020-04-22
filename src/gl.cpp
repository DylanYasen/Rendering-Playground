#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>

#include "gl.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Renderer.h"
#include "Texture.h"
#include "FileUtils.h"

// todo: move these typedefs to math header
#include "HandmadeMath.h"
typedef hmm_m4 mat4;
typedef hmm_v2 vec2;
typedef hmm_v3 vec3;
typedef hmm_quaternion quaternion;

const int WIDTH = 800;
const int HEIGHT = 600;

void glClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool glCheckError(const char* func, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		printf("[OpenGL Error] code:[%d] file:[%s] func:[%s] line:[%d]\n", error, file, func, line);
		return false;
	}
	return true;
}

namespace GL
{
	VertexBuffer vb;
	IndexBuffer ib;
	VertexArray* va;
	Shader* shader;
	Renderer* renderer;
	Texture* texture;

	bool bInit = false;
	SDL_Window* window = NULL;
	int location = -1;

	namespace triangle
	{
		bool init()
		{
			shader = new Shader("resources/shaders/basic.shader");

			GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

			const float vertexData[] =
			{
				-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.0f,  0.5f, 0.0f, 0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertexData, 5 * 3 * sizeof(float));

				VertexBufferLayout layout;
				layout.Push<float>(2); // pos
				layout.Push<float>(3); // color

				va->AddBuffer(vb, layout);
			}

			const unsigned int indexData[] = { 0,1,2 };
			ib = IndexBuffer(indexData, 3);

			return true;
		}

		float r = 0.0f;
		float increment = 0.05f;

		void draw()
		{
			renderer->Clear();

			shader->Bind();
			shader->SetUniform4f("u_color", r, 0.1f, 0.1f, 1.0f);

			renderer->Draw(*va, ib, *shader);

			if (r > 1.0f)
			{
				increment = -0.05f;
			}
			else if (r < 0.0f)
			{
				increment = 0.05f;
			}
			r += increment;
		}
	}

	namespace rect
	{
		bool init()
		{
			GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

			const float vertexData[] =
			{
				100.0f, 100.0f, 0.0f, 0.0f,
				200.0f, 100.0f, 1.0f, 0.0f,
				200.0f, 200.0f, 1.0f, 1.0f,
				100.0f, 200.0f, 0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertexData, sizeof(vertexData));

				VertexBufferLayout layout;
				layout.Push<float>(2);
				layout.Push<float>(2);
				va->AddBuffer(vb, layout);
			}

			const unsigned int indexData[] = { 0,1,2,2,3,0 };
			ib = IndexBuffer(indexData, 6);

			hmm_mat4 proj = HMM_Orthographic(0.0f, WIDTH, 0, HEIGHT, -10, 10);
			hmm_mat4 view = HMM_Translate(HMM_Vec3(-100, 0, 0));
			hmm_mat4 model = HMM_Translate(HMM_Vec3(200, 200, 0));

			hmm_mat4 mvp = proj * view * model;

			shader = new Shader("resources/shaders/sprite.shader");
			shader->Bind();
			shader->SetUniformMat4f("u_mvp", mvp);

			texture = new Texture("resources/textures/mob.PNG");
			texture->Bind();
			shader->SetUniform1i("u_texture", 0);

			return true;
		}

		void draw()
		{
			renderer->Clear();
			renderer->Draw(*va, ib, *shader);
		}
	}

	namespace cube
	{
		hmm_mat4 model;
		hmm_mat4 view;
		hmm_mat4 projection;

		bool init()
		{
			model = HMM_Scale(HMM_Vec3(1, 1, 1)) * HMM_Rotate(30, HMM_Vec3(0, 1, 0)) * HMM_Translate(HMM_Vec3(0, 0, 0));
			view = HMM_Translate(HMM_Vec3(0.0f, 0.0f, -5.0f));
			projection = HMM_Perspective(60.0f, WIDTH / HEIGHT, 0.1f, 1000.0f);

			shader = new Shader("resources/shaders/cube.shader");
			shader->Bind();
			shader->SetUniformMat4f("u_m", model);
			shader->SetUniformMat4f("u_v", view);
			shader->SetUniformMat4f("u_p", projection);

			texture = new Texture("resources/textures/cube.PNG");
			texture->Bind();
			shader->SetUniform1i("u_tex", 0);

			const float vertices[] = {
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertices, sizeof(vertices));

				VertexBufferLayout layout;
				layout.Push<float>(3);
				layout.Push<float>(2);
				va->AddBuffer(vb, layout);
			}

			glEnable(GL_DEPTH_TEST);

			return true;
		}

		void draw()
		{
			renderer->Clear();

			va->Bind();
			shader->Bind();

			// rotate
			{
				model = model * HMM_Rotate(1.0f, HMM_Vec3(1, 1, 0));
				shader->SetUniformMat4f("u_m", model);
			}

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	namespace model
	{
		struct Vertex
		{
			vec3 position;
			vec3 normal;
			vec2 texCoords;
		};

		struct Transform
		{
			vec3 pos;
			float rotAngle;
			vec3 rotAxis;
			vec3 scale;
			quaternion rot;
			mat4 parentTransform;

			Transform() : pos(HMM_Vec3(0, 0, 0)),  rotAngle(0),
				rotAxis(HMM_Vec3(0,0,0)), scale(HMM_Vec3(1, 1, 1)), 
				rot(HMM_Quaternion(0,0,0,0)), parentTransform(HMM_Mat4d(1))
			{
			}

			mat4 getMatrix()
			{
				return HMM_Scale(scale) * HMM_Rotate(rotAngle, rotAxis) * HMM_Translate(pos);
			}

			mat4 getWorldMatrix()
			{
				return parentTransform * getMatrix();
			}
		};

		struct Mesh
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture*> textures;
			Transform transform;
			const char* name;

			VertexArray* vao;
			VertexBuffer vbo;
			IndexBuffer ibo;

			mat4 model;
			mat4 view;
			mat4 projection;

			unsigned int VAO, VBO, EBO;

			Mesh(const std::vector<Vertex>& v, const std::vector<unsigned int>& i,
				const std::vector<Texture*>& t, Transform trans, const char* name)
				: vertices(v), indices(i), textures(t), transform(trans), name(name)
			{
				//model = HMM_Scale(HMM_Vec3(1, 1, 1)) * HMM_Rotate(0, HMM_Vec3(1, 0, 0)) * HMM_Translate(HMM_Vec3(0, 0, 0));
				model = transform.getWorldMatrix();
				view = HMM_Rotate(30, HMM_Vec3(1, 0, 0)) * HMM_Translate(HMM_Vec3(0.0f, -800.0f, -800.f));
				projection = HMM_Perspective(60.0f, WIDTH / HEIGHT, 0.1f, 2000.0f);

				shader = new Shader("resources/shaders/model.shader");

				float stride = sizeof(Vertex);
				vao = new VertexArray();
				{
					vbo = VertexBuffer(&vertices[0], sizeof(Vertex) * vertices.size());

					VertexBufferLayout layout;
					layout.Push<float>(3); // pos
					layout.Push<float>(3); // normal
					layout.Push<float>(2); // tex coords
					vao->AddBuffer(vbo, layout);
				}
				ibo = IndexBuffer(&indices[0], indices.size());
			}

			void draw(const mat4& parentTransform)
			{
				unsigned int diffuseNr = 1;
				unsigned int specularNr = 1;

				vao->Bind();
				ibo.Bind();
				shader->Bind();

				/*for (size_t i = 0; i < textures.size(); i++)
				{
					std::string uniformName = "u_tex" + std::to_string(i);
					textures[i]->Bind();
					shader->SetUniform1i(uniformName, 0);
				}*/
				if (textures.size() > 0)
				{
					textures[0]->Bind();
					shader->SetUniform1i("u_tex", 0);
				}

				model = transform.getWorldMatrix();
				shader->SetUniformMat4f("u_m",  parentTransform * model);
				shader->SetUniformMat4f("u_v", view);
				shader->SetUniformMat4f("u_p", projection);
				renderer->Draw(*vao, ibo, *shader);
			}
		};

		std::unordered_set<const char*> loadedTextures;
		std::string name = "";
		std::string rootpath = "";
		std::vector<Mesh*> meshes;
		Transform transform;

		void loadMaterialTextures(const aiMaterial* material, aiTextureType type,
			const std::string& typeName, std::vector<Texture*>& outTextures)
		{
			for (size_t i = 0; i < material->GetTextureCount(type); i++)
			{
				aiString str;
				material->GetTexture(type, i, &str);

				std::string matName = str.C_Str();

				// fix messed up filepath
				// expect textures are in the same folder as the model
				{
					size_t lastSlashPos = matName.find_last_of('\\');
					if (lastSlashPos != std::string::npos)
					{
						matName = matName.substr(lastSlashPos + 1);
					}
				}
				{
					size_t lastSlashPos = matName.find_last_of('/');
					if (lastSlashPos != std::string::npos)
					{
						matName = matName.substr(lastSlashPos + 1);
					}
				}
				
				// prepend model root path
				aiString fullpath(rootpath);
				fullpath.Append(matName.c_str());
				const char* cstr = fullpath.C_Str();

				if (loadedTextures.find(cstr) == loadedTextures.end())
				{
					Texture* tex = new Texture(cstr, typeName.c_str());
					outTextures.push_back(tex);
					loadedTextures.insert(cstr);
				}
			}
		}

		Mesh* processMesh(const aiScene* scene, aiNode* node, const mat4& parentTransform, const aiMesh* mesh)
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture*> textures;

			Transform transform;
			{
				aiVector3D t, s, rotAxis;
				ai_real rotAngle;

				const auto& nodeTransform = node->mTransformation;
				nodeTransform.Decompose(s, rotAxis, rotAngle, t);

		/*		aiQuaternion rot;
				node->mTransformation.Decompose(s, rot, t);
		*/
				transform.pos = HMM_Vec3(t.x, t.y, t.z);
				transform.scale = HMM_Vec3(s.x, s.y, s.z);
				transform.rotAngle = rotAngle;
				transform.rotAxis = HMM_Vec3(rotAxis.x, rotAxis.y, rotAxis.z);
				
				transform.parentTransform = parentTransform;
			}

			// vertices
			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;

				// position
				const aiVector3D& v = mesh->mVertices[i];
				vertex.position = HMM_Vec3(v.x, v.y, v.z);

				// normal
				const aiVector3D& n = mesh->mNormals[i];
				vertex.normal = HMM_Vec3(n.x, n.y, n.z);

				// texture coordinates
				if (mesh->mTextureCoords[0])
				{
					const aiVector3D& t = mesh->mTextureCoords[0][i];
					vertex.texCoords = HMM_Vec2(t.x, t.y);
				}
				else
				{
					vertex.texCoords = HMM_Vec2(0.0f, 0.0f);
				}

				vertices.push_back(vertex);
			}

			// indices
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				const aiFace& face = mesh->mFaces[i];
				for (size_t j = 0; j < face.mNumIndices; j++)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			// material
			if (mesh->mMaterialIndex >= 0)
			{
				const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				std::vector<Texture*> outDiffuseMaps;
				loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse", outDiffuseMaps);
				textures.insert(textures.end(), outDiffuseMaps.begin(), outDiffuseMaps.end());

				std::vector<Texture*> outSpecularMaps;
				loadMaterialTextures(material, aiTextureType_SPECULAR, "specular", outSpecularMaps);
				textures.insert(textures.end(), outSpecularMaps.begin(), outSpecularMaps.end());

				std::vector<Texture*> outNormalMaps;
				loadMaterialTextures(material, aiTextureType_NORMALS, "normal", outNormalMaps);
				textures.insert(textures.end(), outNormalMaps.begin(), outNormalMaps.end());

				std::vector<Texture*> outHeightMaps;
				loadMaterialTextures(material, aiTextureType_HEIGHT, "height", outHeightMaps);
				textures.insert(textures.end(), outHeightMaps.begin(), outHeightMaps.end());
			}

			Mesh* m = new Mesh(vertices, indices, textures, transform, mesh->mName.C_Str());
			return m;
		}

		mat4 aiMat4toMat4(const aiMatrix4x4& aimat4)
		{
			mat4 result;

			result.Elements[0][0] = aimat4.a1;
			result.Elements[0][1] = aimat4.b1;
			result.Elements[0][2] = aimat4.c1;
			result.Elements[0][3] = aimat4.d1;

			result.Elements[1][0] = aimat4.a2;
			result.Elements[1][1] = aimat4.b2;
			result.Elements[1][2] = aimat4.c2;
			result.Elements[1][3] = aimat4.d2;

			result.Elements[2][0] = aimat4.a3;
			result.Elements[2][1] = aimat4.b3;
			result.Elements[2][2] = aimat4.c3;
			result.Elements[2][3] = aimat4.d3;

			result.Elements[3][0] = aimat4.a4;
			result.Elements[3][1] = aimat4.b4;
			result.Elements[3][2] = aimat4.c4;
			result.Elements[3][3] = aimat4.d4;

			return result;
		}

		void processNode(const aiScene* scene, aiNode* node, const mat4& parentTransform)
		{
			const mat4& transform = aiMat4toMat4(node->mTransformation);
			const mat4& accParentTransform = parentTransform * transform;
			
			for (size_t i = 0; i < node->mNumMeshes; i++)
			{
				const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(scene, node, accParentTransform, mesh));
			}

			// child nodes
			for (size_t i = 0; i < node->mNumChildren; i++) {
				processNode(scene, node->mChildren[i], accParentTransform);
			}
		}

		bool init()
		{
			//            std::string filepath = "resources/models/lighthouse/source/Cotman_Sam.fbx";
			//            std::string filepath = "resources/models/nanosuit/source/suit.fbx";
			//std::string filepath = "resources/models/gilnean-chapel/gilneas.fbx";
			std::string filepath = "resources/models/junkrat/junkrat.fbx";
			//std::string filepath = "resources/models/chaman-ti-pche/model.fbx";

			const aiScene* scene = FileUtil::LoadModel(filepath.c_str());
			const size_t lastSlashPos = filepath.find_last_of('/');
			rootpath = filepath.substr(0, lastSlashPos + 1);
			name = filepath.substr(lastSlashPos + 1);
			if (scene) processNode(scene, scene->mRootNode, HMM_Mat4d(1));

			GLCall(glEnable(GL_DEPTH_TEST));
			//GLCall(glClearColor(1.f, 1.f, 1.f, 1.f));

			return true;
		}

		float angle = 0;

		void draw()
		{
			renderer->Clear();

			angle += 0.5f;
			const auto& mat = transform.getMatrix() * HMM_Rotate(angle, HMM_Vec3(0, 1,0));
 			for (auto mesh : meshes) {
				mesh->draw(mat);
			}
		}
	}

	bool initGL()
	{
		renderer = new Renderer();

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		//return triangle::init();
		//return rect::init();
//		return cube::init();
		return model::init();
	}

	void init(SDL_Window* win)
	{
		if (!win) return;
		window = win;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GLContext context = SDL_GL_CreateContext(window);
		if (context == NULL)
		{
			printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
			return;
		}

		if (!gladLoadGL())
		{
			printf("gladLoadGL failed");
			return;
		}

		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n", glGetString(GL_VERSION));

		// Vsync
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}

		if (!initGL())
		{
			printf("Unable to initialize OpenGL!\n");
		}
		else
		{
			bInit = true;
		}
	}

	void render()
	{
		if (!bInit) return;


		//triangle::draw();
		//rect::draw();
//		cube::draw();
		model::draw();

		SDL_GL_SwapWindow(window);
	}
}
