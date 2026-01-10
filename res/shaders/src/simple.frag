#version 450

layout(location = 0) in vec2 frag_texcoord;
layout(binding = 0) uniform sampler2D tex_sampler;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex_sampler, frag_texcoord);
}
