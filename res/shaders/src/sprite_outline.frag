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

    // 1. Sprite selbst (scharf mit LOD 0)
    vec4 tex_color = textureLod(tex_sampler, frag_texcoord, 0.0);
    float sprite_alpha = smoothstep(0.45, 0.55, tex_color.a);

    // 2. Umriss-Suche (Dichte-basiert für AA)
    vec2 dX = dFdx(frag_texcoord);
    vec2 dY = dFdy(frag_texcoord);
    
    float density = 0.0;
    const float range = 2.0; // Radius der Outline in Pixeln

    // Wir tasten ein 4x4 Gitter ab für optimale Glättung
    for (float y = -0.75; y <= 0.75; y += 0.5) {
        for (float x = -0.75; x <= 0.75; x += 0.5) {
            // Wir skalieren die taps auf den Ziel-Radius
            vec2 offset = vec2(x, y) * (range / 0.75);
            vec2 sample_uv = frag_texcoord + offset.x * dX + offset.y * dY;
            
            // Boundary Check
            if (sample_uv.x >= uv_min.x && sample_uv.x <= uv_max.x &&
                sample_uv.y >= uv_min.y && sample_uv.y <= uv_max.y) {
                
                // Wir nutzen hier LOD 0 für maximale Präzision bei AA
                density += textureLod(tex_sampler, sample_uv, 0.0).a;
            }
        }
    }
    density /= 16.0; // Durchschnittliche Dichte (0.0 bis 1.0)

    // 3. Berechnung der Outline-Stärke mit Anti-Aliasing
    // Ein kleiner Bereich (0.01 bis 0.1) sorgt für eine extrem glatte Außenkante
    float outline_alpha = smoothstep(0.01, 0.1, density);
    
    // Kombinieren
    float final_alpha = max(sprite_alpha, outline_alpha);

    if (final_alpha > 0.01) {
        // Ergebnis: Weißer Umriss (PMA)
        out_color = vec4(final_alpha, final_alpha, final_alpha, final_alpha);
        
        // Innerhalb des Sprites: Sprite-Farbe + Selektions-Tint
        if (sprite_alpha > 0.01) {
            vec4 sprite_col = (tex_color * frag_color) + vec4(0.12, 0.12, 0.12, 0.0);
            sprite_col.rgb *= tex_color.a; // PMA
            out_color = mix(out_color, sprite_col, sprite_alpha);
        }
    } else {
        discard;
    }
}
