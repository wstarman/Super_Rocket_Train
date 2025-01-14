#version 430 core
layout (location = 0) in vec4 infoIn;

layout (std140) uniform Matrices{
    mat4 view;
    mat4 projection;
};

out vec3 position;
out float alpha;

uniform vec3 eyePosition;

void main() {
    vec4 globalPos = projection * view * vec4(infoIn.xyz, 1);
    gl_Position = globalPos;
    gl_PointSize = 500.0/float(length(globalPos.xyz));
    if(projection[3]==vec4(0,0,0,1))    // is Ortho
        gl_PointSize = 2;
    position = globalPos.xyz;
    alpha = infoIn.w;
}