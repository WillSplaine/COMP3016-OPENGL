//#include "modelloader.h"
//#include <iostream>
//#include <fstream>
//
//
//ModelLoader::ModelLoader(const char* filePath) : filePath(filePath), bonsaiVAO(0), bonsaiVBO(0), bonsaiEBO(0) {}
//
//ModelLoader::~ModelLoader() {
//    // You may want to add cleanup code here if needed
//}
//
//bool ModelLoader::loadModel() {
//    Assimp::Importer importer;
//    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
//
//    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
//        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
//        return false;
//    }
//
//    aiMesh* mesh = scene->mMeshes[0]; // Assuming there's only one mesh in the model
//    std::vector<float> vertices;
//    std::vector<unsigned int> indices;
//
//    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
//        vertices.push_back(mesh->mVertices[i].x);
//        vertices.push_back(mesh->mVertices[i].y);
//        vertices.push_back(mesh->mVertices[i].z);
//    }
//
//    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
//            indices.push_back(face.mIndices[j]);
//        }
//    }
//
//    vertexCount = mesh->mNumVertices;
//    indexCount = indices.size();
//
//    glGenVertexArrays(1, &bonsaiVAO);
//    glGenBuffers(1, &bonsaiVBO);
//    glGenBuffers(1, &bonsaiEBO);
//
//    glBindVertexArray(bonsaiVAO);
//
//    glBindBuffer(GL_ARRAY_BUFFER, bonsaiVBO);
//    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bonsaiEBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
//    glEnableVertexAttribArray(0);
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    return true;
//}
