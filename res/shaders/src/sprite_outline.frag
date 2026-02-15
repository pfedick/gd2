#version 450

// Input from vertex shader
layout(location = 0) in vec2 frag_texcoord;
layout(location = 1) in vec4 frag_color;

// Texture sampler
layout(set = 2, binding = 0) uniform sampler2D tex_sampler;

// Output
layout(location = 0) out vec4 out_color;

void main() {
    // Sample core texture
    vec4 tex_color = texture(tex_sampler, frag_texcoord);
    
    // If the pixel is already opaque, we render the sprite normally (with modulation)
    if (tex_color.a > 0.1) {
        tex_color.rgb *= tex_color.a;
        out_color = tex_color * frag_color;
        // Optionally: make it a bit brighter or tinted to show selection
        out_color.rgb += vec3(0.1, 0.1, 0.1); 
        return;
    }

    // Determine texel size for searching neighbors
    ivec2 texSize = textureSize(tex_sampler, 0);
    vec2 texelSize = 1.0 / vec2(texSize);
    
    // Search radius: 4 pixels for 4K clarity
    int radius = 3;
    bool is_edge = false;
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x == 0 && y == 0) continue;
            
            // Optimization: check a box-like or cross-like area
            // We only care if ANY neighbor is opaque
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            if (texture(tex_sampler, frag_texcoord + offset).a > 0.1) {
                is_edge = true;
                break;
            }
        }
        if (is_edge) break;
    }

    if (is_edge) {
        // Solid white outline for high visibility
        out_color = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        discard;
    }
}
