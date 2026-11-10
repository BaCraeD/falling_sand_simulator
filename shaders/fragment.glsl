#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;

void main() {
    // Direct nearest-neighbor sample keeps simulation pixels sharp
    FragColor = texture(uTexture, TexCoord);
}
