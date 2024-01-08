#pragma once
//vert shader source code
const char* vertexShaderSource = R"(
   #version 330 core
 layout (location = 0) in vec3 aPos;
 layout (location = 1) in vec3 aColor; // Added color attribute
 layout (location = 2) in vec3 aNormal; // Added normal attribute
 layout (location = 3) in vec2 textVert; // Texture Vertexes

 uniform mat4 model;
 uniform mat4 view;
 uniform mat4 projection;

 out vec3 FragPos; // Pass position to fragment shader
 out vec3 Normal; // Pass normal to fragment shader
 out vec3 Color; // Pass color to fragment shader
 out vec2 textFrag;

 void main()
 {
     gl_Position = projection * view * model * vec4(aPos, 1.0);
     FragPos = vec3(model * vec4(aPos, 1.0));
     Normal = mat3(transpose(inverse(model))) * aNormal;
     Color = aColor;
     textFrag = textVert;
 }
)";
// Frag shader source code
const char* fragmentShaderSource = R"(
    #version 330 core
in vec3 FragPos; // Received position from vertex shader
in vec3 Normal; // Received normal from vertex shader
in vec3 Color; // Received color from vertex shader
in vec2 textFrag;

out vec4 FragColorOutput;

uniform vec3 lightPos; // Position of the light source
uniform vec3 viewPos; // Position of the camera/viewer
uniform sampler2D texture_main;

void main()
{
    // Ambient lighting
    float ambientStrength = 1.0;
    vec3 ambient = ambientStrength * Color;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * Color;

    // Specular lighting
    float specularStrength = 1.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    // Final color
    vec3 result = ambient + diffuse + specular;
    FragColorOutput = texture(texture_main, textFrag) * vec4(result, 1.0);
    //FragColorOutput = texture(texture_main, textFrag);
}
)";