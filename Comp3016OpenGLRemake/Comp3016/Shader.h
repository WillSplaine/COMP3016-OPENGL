#pragma once

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec4 vertexColor;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        vertexColor = vec4(aColor, 1.0);
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