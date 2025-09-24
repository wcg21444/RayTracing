#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 Dir;

void main() {

    mat4 rotView = mat4(mat3(view));

    vec4 pos = projection * rotView * vec4(aPos, 1.0);

    gl_Position = pos.xyzw;

    Dir = aPos; 
}