#version 450

// Simple vertex shader - just position and UV
layout(location = 0) in vec2 in_position;   // Screen position in pixels
layout(location = 1) in vec2 in_texcoord;   // UV coordinates

layout(push_constant) uniform PushConstants {
    mat4 projection;
} push;

layout(location = 0) out vec2 frag_texcoord;

void main() {
    gl_Position = push.projection * vec4(in_position, 0.0, 1.0);
    frag_texcoord = in_texcoord;
}
