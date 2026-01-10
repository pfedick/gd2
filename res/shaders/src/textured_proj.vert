#version 450

// Uniform buffer for projection matrix (not push constant!)
layout(binding = 0, set = 1) uniform UniformBlock {
    mat4 projection;
} ubo;

layout(location = 0) in vec2 in_position;   // Pixel coordinates
layout(location = 1) in vec2 in_texcoord;

layout(location = 0) out vec2 frag_texcoord;

void main() {
    gl_Position = ubo.projection * vec4(in_position, 0.0, 1.0);
    frag_texcoord = in_texcoord;
}
