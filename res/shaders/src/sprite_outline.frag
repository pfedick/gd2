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

    // Sample core texture with LOD 0 for sharp sprite rendering
    vec4 tex_color = textureLod(tex_sampler, frag_texcoord, 0.0);
    
    // 1. Render core sprite if it's solid enough
    // We use a smooth transition to avoid flickering at the edge of the sprite itself
    float sprite_alpha = smoothstep(0.45, 0.55, tex_color.a);
    if (sprite_alpha > 0.9) {
        tex_color.rgb *= tex_color.a;
        out_color = (tex_color * frag_color) + vec4(0.1, 0.1, 0.1, 0.0);
        return;
    }

    // 2. Search for outlines in the neighborhood
    vec2 dX = dFdx(frag_texcoord);
    vec2 dY = dFdy(frag_texcoord);
    
    float max_a = 0.0;
    
    // We use a stable 12-tap sampling pattern
    // Using a slightly higher LOD (1.0) for the search makes the detection 
    // much more stable against sub-pixel flickering and minification.
    const vec2 taps[12] = {
        vec2(-1.5, -1.5), vec2(0.0, -2.0), vec2(1.5, -1.5),
        vec2(-2.0,  0.0),                  vec2(2.0,  0.0),
        vec2(-1.5,  1.5), vec2(0.0,  2.0), vec2(1.5,  1.5),
        vec2(-0.8, -0.8), vec2(0.8, -0.8), vec2(-0.8, 0.8), vec2(0.8, 0.8)
    };

    for (int i = 0; i < 12; i++) {
        vec2 sample_uv = frag_texcoord + taps[i].x * dX + taps[i].y * dY;
        
        // STRICT BOUNDARY CHECK with tiny epsilon to prevent flickering at edges
        if (sample_uv.x >= uv_min.x - 0.00001 && sample_uv.x <= uv_max.x + 0.00001 &&
            sample_uv.y >= uv_min.y - 0.00001 && sample_uv.y <= uv_max.y + 0.00001) {
            
            // Sample alpha from a slightly blurred mipmap level for stability
            // Note: This requires mipmaps to be generated for the atlas!
            max_a = max(max_a, textureLod(tex_sampler, sample_uv, 1.0).a);
        }
    }

    // 3. Smooth the outline alpha to eliminate popping/flickering
    float outline_alpha = smoothstep(0.1, 0.5, max_a);
    
    // Combine sprite and outline (simple max for selection look)
    float final_a = max(sprite_alpha, outline_alpha);

    if (final_a > 0.01) {
        // Output white for the outline, or the sprite pixel
        // For selection, we favor a solid white outline
        out_color = vec4(final_a, final_a, final_a, final_a); // White PMA
        
        // If we are touching the sprite, blend with the slightly tinted sprite color
        if (sprite_alpha > 0.01) {
            vec4 sprite_col = (tex_color * frag_color) + vec4(0.1, 0.1, 0.1, 0.0);
            sprite_col.rgb *= tex_color.a;
            out_color = mix(out_color, sprite_col, sprite_alpha);
        }
    } else {
        discard;
    }
}
