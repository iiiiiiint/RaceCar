#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

#include <Mesh.h>
#include <Shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <My_assimp.h>
#include <map>

using namespace std;

class Model {
public:

    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec2> mTextureCoords;
    std::vector<Vertex> Vertices;
    int HasNormals;
    int HasTexture;
    Model(std::string path) {
        HasNormals = 0;
        HasTexture = 0;
        loadModel(path);
    }
    void Draw(Shader &shader) {
        for (unsigned int i = 0; i < meshes.size(); i++){
            // std::cout<<"Draw!"<<std::endl;
            meshes[i].Draw(shader);
        }
    }

private:
    void loadModel(const std::string& path) {
        std::ifstream file(path);
        // std::cout<< "begin loadmodel!"<<std::endl;
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return;
        }
        Scene scene;
        scene.mRootNode = new aiNode;
        scene.mRootNode->mNumMeshes=0;
        scene.mRootNode->mNumChildren=0;
        aiMesh *current_aiMesh;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            // std::cout<<type<<std::endl;
            if (type == "v") {
                glm::vec3 vertex;
                float x,y,z;
                iss >> x >> y >> z;
                // cout<< vertex.x<< " " <<vertex.y<<" "<<vertex.z<<endl;
                vertex = glm::vec3(x, y, z);
                Vertex v;
                v.Position = vertex;
                Vertices.push_back(v);
            } else if (type == "vt") {
                HasTexture = 1;
                glm::vec2 texture;
                float x, y;
                iss >> x >> y;
                texture = glm::vec2(x, y);
                mTextureCoords.push_back(texture);
            } else if (type == "vn") {
                HasNormals = 1;
                glm::vec3 normal;
                float x, y, z;
                iss >> x >> y >> z;
                normal = glm::vec3(x, y, z);
                mNormals.push_back(normal);
            } else if (type == "f") {
                std::istringstream s(line.substr(2));
                std::string splitted;
                std::vector<unsigned int> temp_indice;
                std::vector<unsigned int> temp_vt;
                std::vector<unsigned int> temp_vn;
                while(std::getline(s, splitted, ' ')){
                    unsigned int index,vt,vn;
                    char zifu;
                    std::istringstream temp_iss(splitted);
                    temp_iss >> index >> zifu >> vt >>zifu >>vn;
                    // std::cout<<index<<" "<<vt<<" "<<vn<<std::endl;
                    temp_indice.push_back(index- 1);
                    temp_vt.push_back(vt - 1);
                    temp_vn.push_back(vn - 1);
                }
                for(int i = 2;i<temp_indice.size();i++){
                    aiFace face;
                    // std::cout<<&face<<std::endl;
                    face.mNumIndices = 0;
                    // cout << "temp"<<temp_indice[0] << " "<<temp_indice[i-1]<<" "<<temp_indice[i]<<endl;
                    // exit(1);
                    face.mIndices.push_back(temp_indice[0]);
                    face.mIndices.push_back(temp_indice[i-1]);
                    face.mIndices.push_back(temp_indice[i]);
                    face.mNumIndices += 3;
                    current_aiMesh->mFaces.push_back(face);
                    current_aiMesh->mNumFaces ++;
                    if(HasTexture==1){
                        Vertices[temp_indice[0]].TexCoords = mTextureCoords[temp_vt[0]];
                    }else{
                        Vertices[temp_indice[0]].TexCoords = glm::vec2(0.0f, 0.0f);
                    }
                    if(HasNormals==1) 
                        Vertices[temp_indice[0]].Normal = mNormals[temp_vn[0]];
                    else 
                        Vertices[temp_indice[0]].Normal = glm::vec3(0.0f, 0.0f, 0.0f);

                    if(HasTexture)
                        Vertices[temp_indice[i-1]].TexCoords = mTextureCoords[temp_vt[i-1]];
                    else
                        Vertices[temp_indice[i-1]].TexCoords = glm::vec2(0.0f, 0.0f);
                    if(HasNormals)
                        Vertices[temp_indice[i-1]].Normal = mNormals[temp_vn[i-1]];
                    else
                        Vertices[temp_indice[i-1]].Normal = glm::vec3(0.0f, 0.0f, 0.0f);

                    if(HasTexture)
                        Vertices[temp_indice[i]].TexCoords = mTextureCoords[temp_vt[i]];
                    else
                        Vertices[temp_indice[i]].TexCoords = glm::vec2(0.0f, 0.0f);
                    if(HasNormals)
                        Vertices[temp_indice[i]].Normal = mNormals[temp_vn[i]];
                    else 
                        Vertices[temp_indice[i]].Normal = glm::vec3(0.0f, 0.0f, 0.0f);
                }
                // std::cout<<faces.size()<<std::endl;
            }else if (type == "mtllib") {
                // cout<<"there is mtllib"<<endl;
                std::string mtlPath;
                iss >> mtlPath;
                // cout<<mtlPath<<endl;
                std::string temp = path.substr(0, path.find_last_of('/'));
                // cout<<temp<<endl;
                LoadMaterials(temp +'/' +mtlPath, &scene);
            }else if(type == "usemtl"){
                // cout<<"there is o"<<endl;
                if(scene.mRootNode->mNumMeshes > 0){
                    // current_aiMesh->mMaterialIndex = scene.mMeshes.size();
                    scene.mMeshes.push_back(current_aiMesh);
                    scene.mRootNode->mMeshes.push_back(scene.mMeshes.size()-1);
                }
                current_aiMesh = new aiMesh;
                current_aiMesh->mNumFaces = 0;
                scene.mRootNode->mNumMeshes ++;
                std::string materialindex;
                iss >> materialindex;
                // std::cout<<"index = "<<materialindex <<std::endl;
                current_aiMesh->mMaterialIndex = materialindex ;
            }
        }

        // current_aiMesh->mMaterialIndex = scene.mMeshes.size();
        scene.mMeshes.push_back(current_aiMesh);
        scene.mRootNode->mMeshes.push_back(scene.mMeshes.size()-1);
        file.close();
        // cout<<"current_aiMesh->mMaterialIndex ="<<current_aiMesh->mMaterialIndex<<endl;
        std::cout<<"Read Finished!"<<std::endl;
        this->directory = path.substr(0, path.find_last_of('/'));

        processNode(scene.mRootNode, &scene);
        cout<<"finish process Node"<<endl;
    }
    void processNode(aiNode *node, const Scene *scene){
                        // cout<<"processNode"<<endl;
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {   
            // cout<<"cnt="<<node->mMeshes[i]<<endl;
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
    Mesh processMesh(aiMesh *mesh, const Scene *scene){
        vector<unsigned int> indices;
        vector<Texture> textures;

        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++){
                indices.push_back(face.mIndices[j]);
            }
                
        }
        std::map <std::string, aiMaterial*>temp = scene->mMaterial;
        aiMaterial* material = temp[(mesh->mMaterialIndex)];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        int cnt=0;
        int cnt_i=0;
        // return a mesh object created from the extracted mesh data
        return Mesh(Vertices, indices, textures);
    }
    vector<Texture> loadMaterialTextures(aiMaterial *mat, string typeName)
    {
        vector<Texture> textures;
        if(mat->GetTextureCount.count(typeName)==0) return textures;
        for(unsigned int i = 0; i < mat->GetTextureCount[typeName]; i++)
        {
            string str = mat->GetTexture[typeName][i];
            // cout<<str<<endl;
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(strcmp(textures_loaded[j].path.data(), str.c_str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str, this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
    // processes textures after loading the model
unsigned int TextureFromFile(const string path, const string &directory)
{
    string filename = directory + '/' + path;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
    void LoadMaterials(const std::string& mtlPath, Scene *scene) {
        std::ifstream file(mtlPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << mtlPath << std::endl;
            return;
        }
        std::string line;
        aiMaterial *currentMaterial;
        int cnt = 0;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;

            // cout<<type<<endl;

            if (type == "newmtl") {
                if(cnt > 0){
                    scene->mMaterial[currentMaterial->name] = currentMaterial;
                }
                currentMaterial = new aiMaterial;
                string n;
                iss >> n;
                currentMaterial->name = n;
                cnt ++;
            } else if (type == "Ka") {
                iss >> currentMaterial->ambient.x >> currentMaterial->ambient.y >> currentMaterial->ambient.z;
            } else if (type == "Kd") {
                iss >> currentMaterial->diffuse.x >> currentMaterial->diffuse.y >> currentMaterial->diffuse.z;
            } else if (type == "Ks") {
                iss >> currentMaterial->specular.x >> currentMaterial->specular.y >> currentMaterial->specular.z;
            } else if (type == "Ns") {
                iss >> currentMaterial->shininess;
            }else if(type == "map_Kd"){
                string path,path1;
                iss >> path;
                currentMaterial->GetTextureCount["texture_diffuse"] = 1;
                currentMaterial->GetTexture["texture_diffuse"].push_back(path);
                while(1){
                    iss >>path1;
                    if(path==path1 | path1 == "") break;
                    currentMaterial->GetTextureCount["texture_diffuse"] ++;
                    currentMaterial->GetTexture["texture_diffuse"].push_back(path1);
                }
            }else if(type == "map_Ka"){
                string path,path1;
                iss >> path;
                string text = "texture_ambient";
                currentMaterial->GetTextureCount[text] = 1;
                currentMaterial->GetTexture[text].push_back(path);
                while(1){
                    iss >>path1;
                    if(path==path1 | path1 == "") break;
                    currentMaterial->GetTextureCount[text] ++;
                    currentMaterial->GetTexture[text].push_back(path1);
                }
            }else if(type == "map_Ks"){
                string path,path1;
                iss >> path;
                currentMaterial->GetTextureCount["texture_specular"] = 1;
                currentMaterial->GetTexture["texture_specular"].push_back(path);
                while(1){
                    iss >>path1;
                    if(path==path1 | path1 == "") break;
                    currentMaterial->GetTextureCount["texture_specular"] ++;
                    currentMaterial->GetTexture["texture_specular"].push_back(path1);
                }
            }
        }
        scene->mMaterial[currentMaterial->name] = currentMaterial;
        cout<<"LoadMaterials Done!"<<endl;
        // Add the last material to the materials vector
        file.close();
    }
};

#endif