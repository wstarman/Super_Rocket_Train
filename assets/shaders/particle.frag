#version 430 core

in vec3 fragColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(fragColor, 1.0); // �]�w�C��
}