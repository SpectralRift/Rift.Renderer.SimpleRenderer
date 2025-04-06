#version 330 core
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;

out vec2 fragUV;
out vec3 fragNormal;
out vec4 fragColor;

uniform mat4 ufModelMatrix;
uniform mat4 ufViewMatrix;
uniform mat4 ufProjMatrix;

void main() {
    gl_Position = ufProjMatrix * ufViewMatrix * ufModelMatrix * vec4(inPosition, 1.0);
    fragUV = vec2(inUV.x, 1.0 - inUV.y);
    fragNormal = inNormal;
    fragColor = inColor;
}