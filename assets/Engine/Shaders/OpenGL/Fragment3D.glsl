#version 330 core

in vec2 fragUV;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 outColor;

uniform sampler2D sTexture;

void main() {
    outColor = texture(sTexture, fragUV) * fragColor;
}