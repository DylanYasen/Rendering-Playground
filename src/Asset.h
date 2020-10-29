#pragma once

#include "Renderable.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include <assimp/material.h>

class Mesh;

class aiScene;
class aiNode;
class aiMesh;
class aiMaterial;
class aiNode;
class aiNodeAnim;
class aiBone;

class Scene;

class Asset : public Renderable
{

    struct VertexBoneInfo
    {
        VertexBoneInfo(unsigned int boneID, float weight)
            : boneID(boneID), weight(weight) {}

        unsigned int boneID;
        float weight;
    };

    struct BoneInfo
    {
        BoneInfo() {}
        BoneInfo(unsigned int boneID, const mat4 &offsetMatrix)
            : boneID(boneID), offsetMatrix(offsetMatrix) {}

        unsigned int boneID;
        mat4 offsetMatrix;
    };

    struct Animation
    {
        std::unordered_map<std::string, aiNodeAnim *> nodeMap;
    };

public:
    Asset(const std::string &filepath);
    ~Asset();

    virtual void PreRender(const Scene *scene, const Renderer *renderer, class Shader *shader) override;
    virtual void Render(const Scene *scene, const Renderer *renderer, Shader *shader) override;

    virtual void Destroy() override;

private:
    void RenderNode(const Scene *scene, const aiNode *node, const Renderer *renderer, const mat4 &accTransform, Shader* shader);

    void ProcessNode();

    void ProcessTexturePath(std::string &name);
    Mesh *ProcessMesh(const aiScene *scene, aiNode *node,
                      const mat4 &parentTransform, const aiMesh *mesh);

    void ProcessNode(const aiScene *scene, aiNode *node,
                     const mat4 &parentTransform);

    void LoadMaterialTextures(const aiMaterial *material, aiTextureType type,
                              const std::string &typeName,
                              std::vector<Texture *> &outTextures);

    void UpdateAnimNodes(float animTime, aiNode *node, const mat4 &parentTransform);

private:
    std::vector<Mesh *> m_meshes;

    std::unordered_set<std::string> m_loadedTextures;

    std::string m_rootpath;

    const aiScene *srcAsset;

    // instead of reconsuting a scene graph, hijack asimp's node graph for now
    std::unordered_multimap<const aiNode *, Mesh *> sceneGraphMap;

    mat4 rootInverseTransform;

    std::unordered_map<std::string, BoneInfo> boneInfoMap;
    mat4 boneTransforms[100] = {glm::identity<mat4>()};

    std::vector<Animation> animations;

    unsigned int boneCount = 0;
};
