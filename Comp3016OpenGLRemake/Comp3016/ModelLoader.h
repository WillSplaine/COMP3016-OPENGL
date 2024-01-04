#pragma once
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

class ModelLoader {
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    const aiScene* loadModel(const char* filePath);
};