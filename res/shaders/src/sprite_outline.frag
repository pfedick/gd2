#version 450

// Input from vertex shader
layout(location = 0) in vec2 frag_texcoord;
layout(location = 1) in vec4 frag_color;
layout(location = 2) flat in uint frag_instanceID;
layout(location = 3) flat in vec4 frag_uv_bounds;

// Sprite instance data (must match vertex shader structure and binding)
struct SpriteInstance {
    vec2 pos;           // offset 0
    vec2 m_row1;        // offset 8
    vec2 m_row2;        // offset 16
    float pos_z;        // offset 24
    float _pad1;        // offset 28
    vec4 uv;            // offset 32
    vec4 uv_bounds;     // offset 48 (u_min, v_min, u_max, v_max)
    vec2 pivot;         // offset 64
    vec2 offset;        // offset 72
    vec4 color;         // offset 80
};

layout(std430, set = 0, binding = 0) readonly buffer SpriteInstanceBuffer {
    SpriteInstance sprites[];
};

// Texture sampler - Standard batcher set = 2
layout(set = 2, binding = 0) uniform sampler2D tex_sampler;

// Output
layout(location = 0) out vec4 out_color;

void main() {
    vec2 uv_min = frag_uv_bounds.xy;
    vec2 uv_max = frag_uv_bounds.zw;

    // Sample core texture
    vec4 tex_color = texture(tex_sampler, frag_texcoord);
    
    // If the pixel is already opaque, we render the sprite normally (with modulation)
    if (tex_color.a > 0.1) {
        tex_color.rgb *= tex_color.a;
        out_color = tex_color * frag_color;
        // Selection tint
        out_color.rgb += vec3(0.1, 0.1, 0.1); 
        return;
    }

    // Determine texel size for searching neighbors
    ivec2 texSize = textureSize(tex_sampler, 0);
    vec2 texelSize = 1.0 / vec2(texSize);
    
    // Search radius - reduce slightly for tighter atlases
    int radius = 2; 
    bool is_edge = false;
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x == 0 && y == 0) continue;
            
            vec2 sample_uv = frag_texcoord + vec2(float(x), float(y)) * texelSize;
            
            // STRICT BOUNDARY CHECK (Avoid neighbor sprites in atlas)
            // Using a tiny epsilon to stay safely within the intended sprite rect
            if (sample_uv.x < uv_min.x || sample_uv.x > uv_max.x ||
                sample_uv.y < uv_min.y || sample_uv.y > uv_max.y) {
                continue;
            }

            // Sample neighbor - use textureLod to be extra sure about sampling at level 0
            if (textureLod(tex_sampler, sample_uv, 0.0).a > 0.1) {
                is_edge = true;
                break;
            }
        }
        if (is_edge) break;
    }

    if (is_edge) {
        out_color = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        discard;
    }
}
