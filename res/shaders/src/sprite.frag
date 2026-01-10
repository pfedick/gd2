#version 450

// Input from vertex shader
layout(location = 0) in vec2 frag_texcoord;
layout(location = 1) in vec4 frag_color;

// Texture sampler
layout(binding = 0) uniform sampler2D tex_sampler;

// Output
layout(location = 0) out vec4 out_color;

void main() {
    // Sample texture
    vec4 tex_color = texture(tex_sampler, frag_texcoord);
    
    // Apply color modulation
    out_color = tex_color * frag_color;
}
