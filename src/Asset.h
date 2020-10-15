#pragma once

#include "Renderable.h"
#include <vector>
#include <string>
#include <unordered_set>

#include <assimp/material.h>

class Mesh;

class aiScene;
class aiNode;
class aiMesh;
class aiMaterial;

class Scene;

class Asset : public Renderable
{

public:
    Asset(const std::string &filepath);
    ~Asset();

    virtual void Render(const Scene* scene, const Renderer* renderer) override;
    virtual void Destroy() override;

private:
    void ProcessNode();

    void ProcessTexturePath(std::string &name);
    Mesh *ProcessMesh(const aiScene *scene, aiNode *node,
                      const mat4 &parentTransform, const aiMesh *mesh);

    void ProcessNode(const aiScene *scene, aiNode *node,
                     const mat4 &parentTransform);

    void LoadMaterialTextures(const aiMaterial *material, aiTextureType type,
                              const std::string &typeName,
                              std::vector<Texture *> &outTextures);

private:
    std::vector<Mesh *> m_meshes;

    std::unordered_set<std::string> m_loadedTextures;

    std::string m_rootpath;
};
