#include "Asset.h"
#include "FileUtils.h"
#include "Math.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

Asset::Asset(const std::string &filepath)
{
    const aiScene *scene = FileUtil::LoadModel(filepath.c_str());
    {
        const size_t lastSlashPos = filepath.find_last_of('/');
        m_rootpath = filepath.substr(0, lastSlashPos + 1);
        m_name = filepath.substr(lastSlashPos + 1);
    }
    if (scene)
    {
        ProcessNode(scene, scene->mRootNode, HMM_Mat4d(1));
    }

    // init shader
    {
        // todo: abstract out material & material instances
        Shader *shader = new Shader("resources/shaders/material.shader");

        shader->Bind();
        // light settings
        {
            shader->SetUniform3f("light.ambient", 0.5, 0.5, 0.5);
            shader->SetUniform3f("light.diffuse", 1, 1, 1);
            shader->SetUniform3f("light.specular", 1.0f, 1.0f, 1.0f);
        }

        // assign shader
        for (const auto &r : m_meshes)
        {
            r->SetShader(shader);
        }
    }
}

Asset::~Asset()
{
}

void Asset::Render(const Camera *camera, const Renderer *renderer)
{
    for (const auto &r : m_meshes)
    {
        r->Render(camera, renderer);
    }
}

void Asset::ProcessNode(const aiScene *scene, aiNode *node,
                        const mat4 &parentTransform)
{
    const mat4 &transform = Math::aiMat4toMat4(node->mTransformation);
    const mat4 &accParentTransform = parentTransform * transform;

    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(scene, node, accParentTransform, mesh));
    }

    // child nodes
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(scene, node->mChildren[i], accParentTransform);
    }
    
//    printf("mesh size:%d\n", m_meshes.size());
}

Mesh *Asset::ProcessMesh(const aiScene *scene, aiNode *node,
                         const mat4 &parentTransform, const aiMesh *mesh)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture *> textures;

    Transform transform;
    {
        aiVector3D t, s, rotAxis;
        ai_real rotAngle;

        const auto &nodeTransform = node->mTransformation;
        nodeTransform.Decompose(s, rotAxis, rotAngle, t);

        transform.pos = HMM_Vec3(t.x, t.y, t.z);
        transform.scale = HMM_Vec3(s.x, s.y, s.z);
        transform.rot =
            HMM_QuaternionFromAxisAngle(Math::aiVec3ToVec3(rotAxis), rotAngle);
        transform.parentTransform = parentTransform;
    }

    // vertices
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // position
        const aiVector3D &v = mesh->mVertices[i];
        vertex.position = HMM_Vec3(v.x, v.y, v.z);

        // normal
        const aiVector3D &n = mesh->mNormals[i];
        vertex.normal = HMM_Vec3(n.x, n.y, n.z);

        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            const aiVector3D &t = mesh->mTextureCoords[0][i];
            vertex.texCoords = HMM_Vec2(t.x, t.y);
        }
        else
        {
            vertex.texCoords = HMM_Vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = Math::aiVec3ToVec3(mesh->mTangents[i]);
            vertex.bitangent = Math::aiVec3ToVec3(mesh->mBitangents[i]);
        }

        vertices.push_back(vertex);
    }

    // indices
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace &face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // material
    if (mesh->mMaterialIndex >= 0)
    {
        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // enum aiTextureType
        static const std::string typemap[] = {"none",
                                              "material.diffuse",
                                              "material.specular",
                                              "material.ambient",
                                              "material.emissive",
                                              "material.height",
                                              "material.normal",

                                              "material.shininess",
                                              "material.opacity",
                                              "material.lightmap",
                                              "material.reflection",

                                              "material.basecolor",
                                              "material.normal_camera",
                                              "material.emission_color",
                                              "material.matalness",
                                              "material.diffuse_roughness",
                                              "material.ambient_occlusion",

                                              "material.unknown"};

        for (int i = 0; i < aiTextureType_UNKNOWN; i++)
        {
            LoadMaterialTextures(material, (aiTextureType)i,
                                 TextureTypeToString((aiTextureType)i), textures);
        }
    }

    Mesh *m = new Mesh(vertices, indices, textures,
                       transform, mesh->mName.C_Str());
    return m;
}

void Asset::LoadMaterialTextures(const aiMaterial *material, aiTextureType type,
                                 const std::string &typeName,
                                 std::vector<Texture *> &outTextures)
{
    for (size_t i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        std::string matName = str.C_Str();
        ProcessTexturePath(matName);

        // prepend model root path
        aiString fullpath(m_rootpath);
        fullpath.Append(matName.c_str());
        const char *cstr = fullpath.C_Str();

        if (m_loadedTextures.find(cstr) == m_loadedTextures.end())
        {
            Texture *tex = new Texture(cstr, typeName.c_str());
            outTextures.push_back(tex);
            m_loadedTextures.insert(cstr);
        }
    }

    // load embeded textures
    aiString textureName;
    material->Get(AI_MATKEY_TEXTURE(type, 0), textureName);
    if (textureName.length > 0)
    {
        std::string texName = textureName.C_Str();
        ProcessTexturePath(texName);

        // prepend model root path
        aiString fullpath(m_rootpath);
        fullpath.Append(texName.c_str());
        const char *cstr = fullpath.C_Str();

        if (m_loadedTextures.find(cstr) == m_loadedTextures.end())
        {
            Texture *tex = new Texture(cstr, typeName.c_str());
            outTextures.push_back(tex);
            m_loadedTextures.insert(cstr);
        }
    }
}

void Asset::ProcessTexturePath(std::string &name)
{
    // fix messed up filepath
    // expect textures are in the same folder as the model
    {
        size_t lastSlashPos = name.find_last_of('\\');
        if (lastSlashPos != std::string::npos)
        {
            name = name.substr(lastSlashPos + 1);
        }
    }
    {
        size_t lastSlashPos = name.find_last_of('/');
        if (lastSlashPos != std::string::npos)
        {
            name = name.substr(lastSlashPos + 1);
        }
    }
}

void Asset::Destroy()
{
}
