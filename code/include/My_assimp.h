#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Mesh.h>
#include <map>
typedef struct aiNode{
    int mNumMeshes;
    int mNumChildren;
    std::vector<int> mMeshes;
    std::vector<aiNode*> mChildren;
}aiNode;

typedef struct aiFace{
    int mNumIndices;
    std::vector<unsigned int> mIndices;
}aiFace;

typedef struct aiMesh{
    int mNumFaces;
    std::vector<aiFace> mFaces;
    std::string mMaterialIndex;
}aiMesh;

typedef struct aiMaterial{
std::map <std::string,int>GetTextureCount;
std::map <std::string,std::vector<string>> GetTexture;
std::string name;
glm::vec3 ambient;
glm::vec3 diffuse;
glm::vec3 specular;
float shininess;
}aiMaterial;

typedef struct Scene{
    aiNode *mRootNode;
    std::vector<aiMesh*> mMeshes;
    // std::vector<aiMaterial *>mMaterial;
    std::map <std::string, aiMaterial *>mMaterial;
}Scene;