#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Assimp\Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#include "Shader.h"
#include "ModelLoader.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float fov = 125.0f;
float cameraSpeed = 2.5f;
float cameraZoomSpeed = 2.0f;
float cameraZoom = 45.0f;
float yaw = -90.0f;
float pitch = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 view;
glm::mat4 projection;

glm::vec3 lightDirection(-1.0f, -1.0f, -1.0f); // Directional light from the top-left
glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);

double lastX = 400, lastY = 300;
bool firstMouse = true;

GLFWwindow* window;
GLuint shaderProgram;



void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updateCameraVectors();
void renderPodium(GLuint& VAO, GLuint& VBO, GLuint& EBO);
void renderWall(GLuint& wallVAO, GLuint& wallVBO, GLuint& wallEBO);
void renderCan(GLuint& canVAO, GLuint& canVBO, GLuint& canEBO);

bool isOutsideArea(const glm::vec3& point);
bool isInsideCube(const glm::vec3& point);

bool loadModel(const std::string& filePath, std::vector<GLfloat>& vertices, std::vector<GLfloat>& colors) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::cout << "Loading Asset " << filePath.c_str() << "\n";

    const aiMesh* mesh = scene->mMeshes[0]; // Assuming the model has only one mesh

    //increases the size of the 3d model
    
   
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Add vertex positions
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Add vertex colors (assuming the model has colors)
        if (mesh->HasVertexColors(0)) {
            vertices.push_back(mesh->mColors[0][i].r);
            vertices.push_back(mesh->mColors[0][i].g);
            vertices.push_back(mesh->mColors[0][i].b);
        }
        else {
            // If the model doesn't have colors, you can set a default color here.
            vertices.push_back(1.0f); // Red
            vertices.push_back(1.0f); // Green
            vertices.push_back(1.0f); // Blue
        }
    }

    return true;
}

const int ColourChanels[]{ 0 , GL_R, GL_RG, GL_RGB, GL_RGBA };

void loadTexture(GLuint& texture, std::string texturepath)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	GLint width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(texturepath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, ColourChanels[nrChannels], GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		std::cout << "Loaded texture " << texturepath.c_str() << std::endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

int main() {

    glfwSetScrollCallback(window, scroll_callback);

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "OpenGL Cube and Camera Movement", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        });

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed\n";
        glfwTerminate();
        return -1;
    }


    std::vector<GLfloat> roomVertices, roomColors;
    if (!loadModel("S:/3rd Year CW/Comp3016/CW2/COMP3016-OPENGL/Comp3016OpenGL/Models/shop2.obj", roomVertices, roomColors)) {
        std::cerr << "Failed to load model" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::vector<GLfloat> bonsaiVertices, bonsaiColors;
    if (!loadModel("S:/3rd Year CW/Comp3016/CW2/COMP3016-OPENGL/Comp3016OpenGL/Models/Bonsai.obj", bonsaiVertices, bonsaiColors)) {
        std::cerr << "Failed to load model" << std::endl;
        glfwTerminate();
        return -1;
    }
   
 

    GLuint shopVAO, shopVBO;
    GLuint shopTexture;

    loadTexture(shopTexture, "S:/3rd Year CW/Comp3016/CW2/COMP3016-OPENGL/Comp3016OpenGL/Models/Textures/brick.jpg");

    glGenVertexArrays(1, &shopVAO);
    glGenBuffers(1, &shopVBO);

    glBindVertexArray(shopVAO);

    glBindBuffer(GL_ARRAY_BUFFER, shopVBO);
    glBufferData(GL_ARRAY_BUFFER, roomVertices.size() * sizeof(GLfloat), roomVertices.data(), GL_STATIC_DRAW);

    // Vertex attribute for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Vertex attribute for color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO to prevent accidental changes
    glBindVertexArray(0);

    GLuint bonsaiVAO, bonsaiVBO;
	GLuint bonsaiTexture;

	//loadTexture(bonsaiTexture, "S:/3rd Year CW/Comp3016/CW2/COMP3016-OPENGL/Comp3016OpenGL/Models/shop2.obj");

    glGenVertexArrays(1, &bonsaiVAO);
    glGenBuffers(1, &bonsaiVBO);

    glBindVertexArray(bonsaiVAO);

    glBindBuffer(GL_ARRAY_BUFFER, bonsaiVBO);
    glBufferData(GL_ARRAY_BUFFER, bonsaiVertices.size() * sizeof(GLfloat), bonsaiVertices.data(), GL_STATIC_DRAW);

    // Vertex attribute for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Vertex attribute for color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind the VAO to prevent accidental changes
    glBindVertexArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_CULL_FACE);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        glfwTerminate();
        return -1;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        glfwTerminate();
        return -1;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glfwTerminate();
        return -1;
    }



    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    cameraPos = glm::vec3(-0.35f, 0.0f, 0.0f);
    GLuint roomLoc = glGetUniformLocation(shaderProgram, "room");
    GLuint bonsaiLoc = glGetUniformLocation(shaderProgram, "plant");
   
    //podium

    GLuint VBO, VAO, EBO;
    renderPodium(VAO, VBO, EBO);


    //watering can

    GLuint canVBO, canVAO, canEBO;
    renderCan(canVAO, canVBO, canEBO);


    //wall

    GLuint wallVBO, wallVAO, wallEBO;
    renderWall(wallVAO, wallVBO, wallEBO);

   
    

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 canModel;


    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // rendering loop 
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Update view matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // Update projection matrix
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirection));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightAmbient"), 1, glm::value_ptr(lightAmbient));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Update model matrix for cube
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


        glm::mat4 roommodelMatrix = glm::translate(glm::mat4(-1.0f), glm::vec3(1.0f, 2.0f, 1.0f));
        glm::mat4 plantmodelMatrix = glm::translate(glm::mat4(-1.0f), glm::vec3(2.0f, 1.0f, 2.0f));
        glm::mat4 bonsaiplantMatrix = glm::translate(glm::mat4(-1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

        glUniformMatrix4fv(roomLoc, 1, GL_FALSE, glm::value_ptr(roommodelMatrix));
        glUniformMatrix4fv(bonsaiLoc, 1, GL_FALSE, glm::value_ptr(plantmodelMatrix));

        

        glBindVertexArray(canVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        float distanceFromCamera = 0.1f; // Adjust this value to bring the cube closer or farther
        glm::vec3 yellowCubePosition = cameraPos;

        glm::mat4 canModel = glm::mat4(1.0f);  // Initialize the canModel matrix
        canModel = glm::translate(canModel, yellowCubePosition);
        canModel = glm::rotate(canModel, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "canModel"), 1, GL_FALSE, glm::value_ptr(canModel));

        glBindVertexArray(wallVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        glm::mat4 wallModel = glm::mat4(1);
        wallModel = glm::scale(wallModel, glm::vec3(0.1));
        /* glm::mat4 wallModel = glm::mat4(1.0f);  */// Initialize the canModel matrix
        wallModel = glm::translate(wallModel, glm::vec3(3.0f, 0.0f, 0.0f));
        wallModel = glm::rotate(wallModel, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "wallModel"), 1, GL_FALSE, glm::value_ptr(wallModel));

       
        glBindVertexArray(bonsaiVAO);
        glDrawArrays(GL_TRIANGLES, 0, bonsaiVertices.size() / 3);
        glBindVertexArray(0);

        glBindVertexArray(shopVAO);
        glActiveTexture(0);
        glBindTexture(GL_TEXTURE_2D, shopTexture);
        glDrawArrays(GL_TRIANGLES, 0, roomVertices.size() / 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        glUseProgram(0);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;

}

void renderPodium(GLuint& podiumVAO, GLuint& podiumVBO, GLuint& podiumEBO) {

    float podiumVertices[] = {
        // Front face
        -0.5f, -2.0f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 1, purple color
         0.5f, -2.0f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 2, purple color
         0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 3, purple color
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 4, green color

        // Left face
        -0.5f, -2.0f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 5, purple color
        -0.5f, -2.0f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 6, green color
        -0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 7, purple color
        -0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 8, purple color

        // Right face
         0.5f, -2.0f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 9, green color
         0.5f, -2.0f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 10, purple color
         0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 11, purple color
         0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 12, purple color

         // Top face
         -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 13, green color
          0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 14, purple color
          0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 15, purple color
         -0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 16, purple color

         // Bottom face
         -10, -2,  10, 0.0f, 1.0f, 0.0f, // Vertex 17, green color
          10, -2,  10, 0.6f, 0.3f, 0.8f, // Vertex 18, purple color
          10, -2.0f, -10, 0.6f, 0.3f, 0.8f, // Vertex 19, purple color
         -10, -2.0f, -10, 0.6f, 0.3f, 0.8f, // Vertex 20, purple color

         // Back face
          0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 21, green color
         -0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 22, purple color
         -0.5f, -2.0f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 23, purple color
          0.5f, -2.0f,  0.3f, 0.6f, 0.3f, 0.8f  // Vertex 24, purple color
    };

    // Cube indices for EBO (Element Buffer Object)
    unsigned int podiumIndices[] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };

    glGenVertexArrays(1, &podiumVAO);
    glGenBuffers(1, &podiumVBO);
    glGenBuffers(1, &podiumEBO);

    glBindVertexArray(podiumVAO);

    glBindBuffer(GL_ARRAY_BUFFER, podiumVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(podiumVertices), podiumVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, podiumEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(podiumIndices), podiumIndices, GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Position attribute
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Color attribute
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void renderCan(GLuint& canVAO, GLuint& canVBO, GLuint& canEBO) {

    float canVertices[] = {
        // Front face
   -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 1, yellow color
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 2, yellow color
    0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 3, yellow color
   -0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 4, yellow color

   // Left face
   -0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 5, yellow color
   -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 6, yellow color
   -0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 7, yellow color
   -0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 8, yellow color

   // Right face
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 9, yellow color
    0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 10, yellow color
    0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 11, yellow color
    0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 12, yellow color

    // Top face
    -0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 13, yellow color
     0.5f, 1.0f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 14, yellow color
     0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 15, yellow color
    -0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 16, yellow color

    // Bottom face
    -0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 17, yellow color
     0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 18, yellow color
     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 19, yellow color
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 20, yellow color

    // Back face
     0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 21, yellow color
    -0.5f, 1.0f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 22, yellow color
    -0.5f, 0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Vertex 23, yellow color
     0.5f, 0.5f,  0.3f, 1.0f, 1.0f, 0.0f  // Vertex 24, yellow color

    };

    unsigned int canIndices[] = {
        // Front face
    0, 1, 2,
    2, 3, 0,

    // Left face
    4, 5, 6,
    6, 7, 4,

    // Right face
    8, 9, 10,
    10, 11, 8,

    // Top face
    12, 13, 14,
    14, 15, 12,

    // Bottom face
    16, 17, 18,
    18, 19, 16,

    // Back face
    20, 21, 22,
    22, 23, 20


    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &canVAO);
    glGenBuffers(1, &canVBO);
    glGenBuffers(1, &canEBO);

    glBindVertexArray(canVAO);

    glBindBuffer(GL_ARRAY_BUFFER, canVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(canVertices), canVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, canEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(canIndices), canIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Position attribute
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Color attribute
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderWall(GLuint& wallVAO, GLuint& wallVBO, GLuint& wallEBO) {

    glUseProgram(shaderProgram);

    glm::mat4 wall = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "wall"), 1, GL_FALSE, glm::value_ptr(wall));

    float wallVertices[] = {
    -10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 1, yellow color
    10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 2, yellow color
    10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 3, yellow color
   -10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 4, yellow color

   // Left face
   -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 5, yellow color
   10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 6, yellow color
   10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 7, yellow color
   -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 8, yellow color

   // Right face
    10, 10, -10, 10, 1.0f, 0.0f, // Vertex 9, yellow color
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 10, yellow color
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 11, yellow color
    10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 12, yellow color

    // Top face
    -10, 10, -10, 10, 1.0f, 0.0f, // Vertex 13, yellow color
     10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 14, yellow color
     10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 15, yellow color
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 16, yellow color

    // Bottom face
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 17, yellow color
     10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 18, yellow color
     10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 19, yellow color
    -10, 10, -10, 1.0f, 1.0f, 0.0f, // Vertex 20, yellow color

    // Back face
     10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 21, yellow color
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 22, yellow color
    -10, 10,  10, 1.0f, 1.0f, 0.0f, // Vertex 23, yellow color
     10, 10,  10, 1.0f, 1.0f, 0.0f  // Vertex 24, yellow color
    };

    // Cube indices for EBO (Element Buffer Object)
    unsigned int wallIndices[] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);

    glBindVertexArray(wallVAO);

    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIndices), wallIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Position attribute
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Color attribute
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;

    projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glfwSetScrollCallback(window, scroll_callback);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float baseCameraSpeed = 2.25f;
    float modifiedCameraSpeed = baseCameraSpeed * deltaTime / 3.0f;

    glm::vec3 newCameraPos = cameraPos;  // Store the new position for collision detection

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        newCameraPos += modifiedCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        newCameraPos -= modifiedCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        newCameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * modifiedCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        newCameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * modifiedCameraSpeed;


    // Check if the new position is inside the cube
    if (!isInsideCube(newCameraPos)) {
        // Update the camera position only if it's not inside the cube
        cameraPos = newCameraPos;
    }

    // Ensure the camera stays at the same height
    cameraPos.y = 0.0f;
}

bool isInsideCube(const glm::vec3& point) {
    // Defines where the cube is and adds a buffer to prevent clipping)
    float buffer = -0.35f;  // Adjust the buffer 
    float cubeMinX = -1.5f - buffer;
    float cubeMaxX = -0.5f + buffer;
    float cubeMinY = -0.5f - buffer;
    float cubeMaxY = 0.5f + buffer;
    float cubeMinZ = -0.5f - buffer;
    float cubeMaxZ = 0.5f + buffer;
   
    
    glm::mat4 inverseModelMatrix = glm::inverse(glm::mat4(1.0f));  

    // Transform the camera position into local space
    glm::vec4 localPoint = inverseModelMatrix * glm::vec4(point, 2.0f);

    // Check if the point is inside the transformed cube
    return (localPoint.x >= cubeMinX && localPoint.x <= cubeMaxX &&
        localPoint.y >= cubeMinY && localPoint.y <= cubeMaxY &&
        localPoint.z >= cubeMinZ && localPoint.z <= cubeMaxZ);
}


bool isOutsideArea(const glm::vec3& point) {
    // Define the boundaries of the allowed area
    float areaMinX = -1;
    float areaMaxX = 1; /* Set your maximum X value */;
    float areaMinY = -1/* Set your minimum Y value */;
    float areaMaxY = 1/* Set your maximum Y value */;
    float areaMinZ = 1/* Set your minimum Z value */;
    float areaMaxZ = 1/* Set your maximum Z value */;

    // Transform the point into local space (if needed)
    glm::mat4 inverseModelMatrix = glm::inverse(glm::mat4(1.0f));
    glm::vec4 localPoint = inverseModelMatrix * glm::vec4(point, 1.0f);

    // Check if the point is outside the allowed area
    return (localPoint.x < areaMinX || localPoint.x > areaMaxX ||
        localPoint.y < areaMinY || localPoint.y > areaMaxY ||
        localPoint.z < areaMinZ || localPoint.z > areaMaxZ);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch to avoid flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}



void updateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}





