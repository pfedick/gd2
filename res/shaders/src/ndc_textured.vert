#version 450

// Simple textured vertex shader - NDC coordinates (no projection needed)
layout(location = 0) in vec2 in_position;   // NDC position (-1 to 1)
layout(location = 1) in vec2 in_texcoord;   // UV coordinates

layout(location = 0) out vec2 frag_texcoord;

void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);
    frag_texcoord = in_texcoord;
}
