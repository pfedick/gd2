#version 450

// Input from vertex shader
layout(location = 0) in vec2 frag_texcoord;
layout(location = 1) in vec4 frag_color;

// Texture sampler - Use Set 2 Binding 0 to avoid collision with Storage Buffer (Set 0 Binding 0)
layout(set = 2, binding = 0) uniform sampler2D tex_sampler;

// Output
layout(location = 0) out vec4 out_color;

void main() {
    // Sample texture
    vec4 tex_color = texture(tex_sampler, frag_texcoord);
    
    // Apply color modulation
    vec4 final_color = tex_color * frag_color;

    if (final_color.a < 0.1) {
        discard;
    }

    out_color = final_color;
}
