//#pragma once
//
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <vector>
//#include <GL/glew.h>
//#include <iostream>
//
//class ModelLoader {
//public:
//    ModelLoader(const char* filePath);
//    ~ModelLoader();
//
//    bool loadModel();
//
//    GLuint getVAO() const { return bonsaiVAO; }
//    GLuint getVBO() const { return bonsaiVBO; }
//    GLuint getEBO() const { return bonsaiEBO; }
//    GLsizei getVertexCount() const;
//    GLsizei getIndexCount() const;
//
//public:
//    const char* filePath;
//    GLuint bonsaiVAO, bonsaiVBO, bonsaiEBO;
//    GLsizei vertexCount;  // New variable to store the vertex count
//    GLsizei indexCount;   // New variable to store the index count
//    
//};