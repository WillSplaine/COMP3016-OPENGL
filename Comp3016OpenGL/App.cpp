#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor; // New vertex attribute for color

    out vec4 vertexColor; // New varying variable

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        vertexColor = vec4(aColor, 1.0); // Pass the color to the fragment shader
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec4 vertexColor;
    out vec4 FragColor;

    void main()
    {
        FragColor = vertexColor;
    }
)";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float cameraSpeed = 2.5f;
float yaw = -90.0f;
float pitch = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 view;
glm::mat4 projection;

double lastX = 400, lastY = 300;
bool firstMouse = true;

GLFWwindow* window;
GLuint shaderProgram;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updateCameraVectors();

bool isInsideCube(const glm::vec3& point);


int main() {
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

    GLuint shaderProgram = glCreateProgram();
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

    // Cube vertices with colors
    float cubeVertices[] = {

        // Front face
-0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 1, purple color
 0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 2, purple color
 0.5f,  0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 3, purple color
-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 4, green color

// Left face
-0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 5, purple color
-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 6, green color
-0.5f,  0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 7, purple color
-0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 8, purple color

// Right face
 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 9, green color
 0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 10, purple color
 0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 11, purple color
 0.5f,  0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 12, purple color

 // Top face
 -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 13, purple color
  0.5f,  0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 14, purple color
  0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 15, purple color
 -0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 16, purple color

 // Bottom face
 -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 17, purple color
  0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 18, purple color
  0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 19, purple color
 -0.5f, -0.5f,  0.5f, 0.6f, 0.3f, 0.8f, // Vertex 20, purple color

 // Back face
  0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 21, purple color
 -0.5f, -0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 22, purple color
 -0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f, // Vertex 23, purple color
  0.5f,  0.5f, -0.5f, 0.6f, 0.3f, 0.8f  // Vertex 24, purple color


    };

    // Cube indices for EBO (Element Buffer Object)
    unsigned int cubeIndices[] = {
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
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Position attribute
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Color attribute
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

        // Update model matrix for cube
        glm::mat4 model = glm::mat4(0.5f);
        model = glm::scale(model, glm::vec3(0.15f));  // Scale the cube by half
        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


        // Update projection matrix
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
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
    // Define the cube boundaries with a buffer zone
    float buffer = -0.3;  // Adjust the buffer zone as needed
    float cubeMinX = -0.5f - buffer;
    float cubeMaxX = 0.5f + buffer;
    float cubeMinY = -0.5f - buffer;
    float cubeMaxY = 0.5f + buffer;
    float cubeMinZ = -0.5f - buffer;
    float cubeMaxZ = 0.5f + buffer;

    // Check if the point is inside the cube
    return (point.x >= cubeMinX && point.x <= cubeMaxX &&
        point.y >= cubeMinY && point.y <= cubeMaxY &&
        point.z >= cubeMinZ && point.z <= cubeMaxZ);
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
    // Calculate the cameraFront vector without changing vertical position
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);
}





