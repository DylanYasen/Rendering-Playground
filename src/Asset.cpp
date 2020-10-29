#include "Asset.h"
#include "FileUtils.h"
#include "Math.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Scene.h"
#include "Light.h"
#include "Camera.h"

#include "FrameData.h"

Asset::Asset(const std::string &filepath)
{
    srcAsset = FileUtil::LoadModel(filepath.c_str());
    {
        const size_t lastSlashPos = filepath.find_last_of('/');
        m_rootpath = filepath.substr(0, lastSlashPos + 1);
        m_name = filepath.substr(lastSlashPos + 1);
    }
    if (srcAsset)
    {
        // process bones
        {
            if (srcAsset->HasAnimations())
            {
                printf("[%d] animation\n", srcAsset->mNumAnimations);
                for (size_t i = 0; i < srcAsset->mNumAnimations; i++)
                {
                    auto anim = srcAsset->mAnimations[i];

                    printf("anim name:[%s] duration:[%f] tick:[%f] \n",
                           anim->mName.C_Str(), anim->mDuration, anim->mTicksPerSecond);

                    printf("num bones: [%d]\n", anim->mNumChannels);
                    Animation animInfo;
                    for (size_t j = 0; j < anim->mNumChannels; j++)
                    {
                        auto animNode = anim->mChannels[j];

                        animInfo.nodeMap[animNode->mNodeName.data] = animNode;

                        printf("\tmesh channel name:[%s] keys:[%d]\n", animNode->mNodeName.C_Str(), animNode->mNumPositionKeys);
                    }
                    animations.push_back(animInfo);

                    for (size_t j = 0; j < anim->mNumMeshChannels; j++)
                    {
                        auto meshAnim = anim->mMeshChannels[j];
                        printf("\tmesh channel name:[%s] keys:[%d]\n", meshAnim->mName.C_Str(), meshAnim->mNumKeys);
                    }

                    for (size_t j = 0; j < anim->mNumMorphMeshChannels; j++)
                    {
                        auto morphAnim = anim->mMorphMeshChannels[j];
                        printf("\tmorph channel name: [%s] keys:[%d]\n", morphAnim->mName.C_Str(), morphAnim->mNumKeys);
                    }
                }
            }
        }

        ProcessNode(srcAsset, srcAsset->mRootNode, m_transform.GetWorld());
    }
}

Asset::~Asset()
{
}

namespace Anim
{
    void SlerpRotationFrames(quat &outRot, float animTime, const aiNodeAnim *animNode)
    {
        if (animNode->mNumRotationKeys == 1)
        {
            outRot = Math::aiQuatToQuat(animNode->mRotationKeys[0].mValue);
        }
        else
        {

            uint keyIndex = -1;
            for (uint i = 0; i < animNode->mNumRotationKeys - 1; i++)
            {
                // printf("frame: [%f]  key: [%f] \n", animTime, animNode->mRotationKeys[i + 1].mTime);
                if (animTime < (float)animNode->mRotationKeys[i + 1].mTime)
                {
                    keyIndex = i;
                    break;
                }
            }

            assert(keyIndex != -1);

            uint nextKeyIndex = (keyIndex + 1);
            assert(nextKeyIndex < animNode->mNumRotationKeys);

            float dt = (float)(animNode->mRotationKeys[nextKeyIndex].mTime - animNode->mRotationKeys[keyIndex].mTime);
            float factor = (animTime - (float)animNode->mRotationKeys[keyIndex].mTime) / dt;
            assert(factor >= 0.0f && factor <= 1.0f);
            const aiQuaternion &StartRotationQ = animNode->mRotationKeys[keyIndex].mValue;
            const aiQuaternion &EndRotationQ = animNode->mRotationKeys[nextKeyIndex].mValue;

            aiQuaternion rot;
            aiQuaternion::Interpolate(rot, StartRotationQ, EndRotationQ, factor);

            outRot = Math::aiQuatToQuat(rot.Normalize());
        }
    }

    void LerpTranslationFrames(vec3 &Out, float animTime, const aiNodeAnim *animNode)
    {

        if (animNode->mNumPositionKeys == 1)
        {
            Out = Math::aiVec3ToVec3(animNode->mPositionKeys[0].mValue);
        }
        else
        {
            uint keyIndex = -1;
            for (size_t i = 0; i < animNode->mNumPositionKeys - 1; i++)
            {
                if (animTime < (float)animNode->mPositionKeys[i + 1].mTime)
                {
                    keyIndex = i;
                    break;
                }
            }
            assert(keyIndex != -1);

            uint nextKeyIndex = (keyIndex + 1);
            assert(nextKeyIndex < animNode->mNumPositionKeys);

            float dt = (float)(animNode->mPositionKeys[nextKeyIndex].mTime - animNode->mPositionKeys[keyIndex].mTime);
            float factor = (animTime - (float)animNode->mPositionKeys[keyIndex].mTime) / dt;
            assert(factor >= 0.0f && factor <= 1.0f);

            const auto &beginPos = animNode->mPositionKeys[keyIndex].mValue;
            const auto &endPos = animNode->mPositionKeys[nextKeyIndex].mValue;
            const auto deltaPos = endPos - beginPos;
            Out = Math::aiVec3ToVec3(beginPos + factor * deltaPos);
        }
    }

    void LerpScaleFrames(vec3 &scale, float animTime, const aiNodeAnim *animNode)
    {
        if (animNode->mNumScalingKeys == 1)
        {
            scale = Math::aiVec3ToVec3(animNode->mScalingKeys[0].mValue);
        }
        else
        {
            uint keyIndex = -1;
            for (uint i = 0; i < animNode->mNumScalingKeys - 1; i++)
            {
                if (animTime < (float)animNode->mScalingKeys[i + 1].mTime)
                {
                    keyIndex = i;
                    break;
                }
            }

            uint nextKeyIndex = keyIndex + 1;
            assert(nextKeyIndex < animNode->mNumScalingKeys);
            float DeltaTime = (float)(animNode->mScalingKeys[nextKeyIndex].mTime - animNode->mScalingKeys[keyIndex].mTime);
            float Factor = (animTime - (float)animNode->mScalingKeys[keyIndex].mTime) / DeltaTime;
            assert(Factor >= 0.0f && Factor <= 1.0f);
            const aiVector3D &Start = animNode->mScalingKeys[keyIndex].mValue;
            const aiVector3D &End = animNode->mScalingKeys[nextKeyIndex].mValue;
            const aiVector3D &Delta = End - Start;
            scale = Math::aiVec3ToVec3(Start + Factor * Delta);
        }
    }

} // namespace Anim

void Asset::UpdateAnimNodes(float animTime, aiNode *node, const mat4 &parentTransform)
{
    if (!srcAsset->HasAnimations())
        return;

    // todo:
    auto anim = srcAsset->mAnimations[0];
    auto &animInfo = animations[0];

    const char *nodeName = node->mName.data;
    const auto animNode = animInfo.nodeMap[nodeName];

    mat4 localTransform = Math::aiMat4toMat4(node->mTransformation);

    if (animNode)
    {

        vec3 translation;
        Anim::LerpTranslationFrames(translation, animTime, animNode);
        const auto &transMat = glm::translate(glm::identity<mat4>(), translation);

        quat rot;
        Anim::SlerpRotationFrames(rot, animTime, animNode);
        const auto &rotMat = glm::mat4_cast(rot);

        {
            // scale is not really used, ignore for now
            // vec3 scale;
            // Anim::LerpScaleFrames(scale, animTime, animNode);
            // const mat4 &scaleMat = glm::scale(glm::identity<mat4>(), scale);
        }

        localTransform = transMat * rotMat; // * scaleMat;
    }

    const mat4 &transform = parentTransform * localTransform;

    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        UpdateAnimNodes(animTime, node->mChildren[i], transform);
    }

    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        const auto &boneInfo = boneInfoMap[nodeName];
        boneTransforms[boneInfo.boneID] = rootInverseTransform * transform * boneInfo.offsetMatrix;
    }
}

void Asset::PreRender(const Scene *scene, const Renderer *renderer, Shader *shader)
{
    {
        rootInverseTransform = glm::inverse(Math::aiMat4toMat4(srcAsset->mRootNode->mTransformation));
        float ticksPerSec = (float)(srcAsset->mAnimations[0]->mTicksPerSecond != 0.0 ? srcAsset->mAnimations[0]->mTicksPerSecond : 25.0f);
        float timeInTicks = Time::totalTimeSec * ticksPerSec;
        float duration = (float)srcAsset->mAnimations[0]->mDuration;
        float animTime = fmod(timeInTicks, duration);
        UpdateAnimNodes(animTime, srcAsset->mRootNode, m_transform.GetWorld());
    }
    {
        // todo: optimize this, for example only pass 4x3
        shader->SetUniformMat4fArray("gBones", boneTransforms, boneCount);
    }
}

void Asset::Render(const Scene *scene, const Renderer *renderer, Shader *shader)
{
    // m_transform.rotation = glm::rotate(m_transform.rotation, glm::radians(1.0f), glm::vec3(0.0, 1.0, 0.0));
    RenderNode(scene, srcAsset->mRootNode, renderer, m_transform.GetWorld(), shader); //m_transform.GetWorld()// glm::identity<mat4>()
}

void Asset::RenderNode(const Scene *scene, const aiNode *node, const Renderer *renderer, const mat4 &accTransform, Shader *shader)
{
    const auto &localTransform = Math::aiMat4toMat4(node->mTransformation);
    const auto &worldTransform = accTransform * localTransform;

    // render this nodes
    {
        auto range = sceneGraphMap.equal_range(node);
        for (auto e = range.first; e != range.second; ++e)
        {
            auto childMesh = e->second;
            childMesh->m_transform.localToWorld = accTransform;
            childMesh->Render(scene, renderer, shader);
        }
    }

    // render child nodes
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        auto childNode = node->mChildren[i];
        RenderNode(scene, childNode, renderer, worldTransform, shader);
    }
}

void Asset::ProcessNode(const aiScene *scene, aiNode *node,
                        const mat4 &parentTransform)
{
    const mat4 &transform = Math::aiMat4toMat4(node->mTransformation);

    printf("mesh count [%d]\n", node->mNumMeshes);
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        const int meshIndex = node->mMeshes[i];
        const aiMesh *mesh = scene->mMeshes[meshIndex];
        m_meshes.push_back(ProcessMesh(scene, node, parentTransform, mesh));
    }

    // child nodes
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(scene, node->mChildren[i], parentTransform * transform);
    }
}

Mesh *Asset::ProcessMesh(const aiScene *scene, aiNode *node,
                         const mat4 &parentTransform, const aiMesh *mesh)
{
    std::vector<Vertex, Allocator<Vertex>> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture *> textures;

    Transform transform;
    {
        aiVector3D t, s, rotAxis;
        ai_real rotAngle;

        const auto &nodeTransform = node->mTransformation;
        nodeTransform.Decompose(s, rotAxis, rotAngle, t);

        transform.position = vec3(t.x, t.y, t.z);
        transform.scale = vec3(s.x, s.y, s.z);
        transform.rotation =
            glm::angleAxis(rotAngle, Math::aiVec3ToVec3(rotAxis));
        transform.localToWorld = parentTransform;
    }

    std::unordered_multimap<unsigned int, VertexBoneInfo> vertexBoneMap; // vertex id => bone
    {
        // printf("mesh [%s] verts: [%d] bones: [%d]\n", mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumBones);
        boneCount = mesh->mNumBones;
        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            auto bone = mesh->mBones[boneIndex];
            const char *boneName = bone->mName.C_Str();

            BoneInfo boneInfo((unsigned int)boneIndex, Math::aiMat4toMat4(bone->mOffsetMatrix));
            boneInfoMap[boneName] = boneInfo;

            for (size_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
            {
                const auto &weightInfo = bone->mWeights[weightIndex];
                VertexBoneInfo vboneInfo(boneIndex, weightInfo.mWeight);
                vertexBoneMap.emplace(weightInfo.mVertexId, vboneInfo);
            }
        }
    }

    // todo: instead of extracting/copying out the geo info, directly use the assimp resource as buffers
    // only things is that the assimp face/index array needs to be flatten
    // vertices
    m_vertices.reserve(mesh->mNumAnimMeshes);
    for (size_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        Vertex vertex;

        // position
        const aiVector3D &v = mesh->mVertices[vertexIndex];
        vertex.position = vec3(v.x, v.y, v.z);

        // normal
        const aiVector3D &n = mesh->mNormals[vertexIndex];
        vertex.normal = vec3(n.x, n.y, n.z);

        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            const aiVector3D &t = mesh->mTextureCoords[0][vertexIndex];
            vertex.texCoords = vec2(t.x, t.y);
        }
        else
        {
            vertex.texCoords = vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = Math::aiVec3ToVec3(mesh->mTangents[vertexIndex]);
            vertex.bitangent = Math::aiVec3ToVec3(mesh->mBitangents[vertexIndex]);
        }

        // populate per vertex bone info
        {
            int index = 0;
            auto range = vertexBoneMap.equal_range(vertexIndex);
            for (auto e = range.first; e != range.second; ++e)
            {
                if (index >= MAX_BONE_PER_VERT)
                {
                    printf("Warning: only support 4 bones per vert\n");
                    break;
                }

                const auto &vertexInfo = e->second;
                {
                    vertex.boneIDs[index] = vertexInfo.boneID;
                    vertex.weights[index] = vertexInfo.weight;
                    index++;
                }
            }
        }

        vertices.push_back(vertex);
    }

    // indices
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace &face = mesh->mFaces[i];

        if (face.mNumIndices != 3)
        {
            printf("ERROR:unsupported primitive: %d\n", face.mNumIndices);
            continue;
        }

        for (size_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // material
    if (mesh->mMaterialIndex >= 0)
    {
        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        for (int i = 0; i < aiTextureType_UNKNOWN; i++)
        {
            LoadMaterialTextures(material, (aiTextureType)i,
                                 TextureTypeToString((aiTextureType)i), textures);
        }
    }

    Mesh *m = new (EResourceType::Geometry) Mesh(vertices, indices, textures,
                                                 transform, mesh->mName.C_Str());

    sceneGraphMap.emplace(node, m);
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
