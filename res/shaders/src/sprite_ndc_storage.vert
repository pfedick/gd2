#version 450

// Vertex attributes (per vertex - defines unit quad [0,0] to [1,1])
layout(location = 0) in vec2 in_position;   // Quad corner position (0-1 range)
layout(location = 1) in vec2 in_texcoord;   // UV template (0-1 range)
layout(location = 2) in vec4 in_color;      // Color modulation

// Sprite instance data in storage buffer (NDC coordinates)
struct SpriteInstance {
    vec2 pos;           // Sprite position (NDC) - PIVOT POINT
    vec2 m_row1;        // Transform Row 1 (m00, m01) - packed as vec2 for alignment (offset 8)
    vec2 m_row2;        // Transform Row 2 (m10, m11) - packed as vec2 for alignment (offset 16)
    vec2 _pad;          // Padding (offset 24)
    vec4 uv;            // Sprite UV rect (x, y, w, h) normalized 0-1 (offset 32)
    vec2 pivot;         // Sprite pivot point (Normalized 0..1 relative to size)
    vec2 offset;        // Unused
};

// Storage buffer for sprite instances (readonly)
layout(std430, set = 0, binding = 0) readonly buffer SpriteInstanceBuffer {
    SpriteInstance sprites[];
};

// Output to fragment shader
layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;

void main() {
    // Get instance ID (which sprite we're rendering)
    uint instanceID = gl_InstanceIndex;
    SpriteInstance sprite = sprites[instanceID];
    
    // Calculate UV coordinates
    frag_texcoord = sprite.uv.xy + in_texcoord * sprite.uv.zw;
    frag_color = in_color;

    // 1. Get local vector from pivot (in 0..1 unit space)
    vec2 local = in_position - sprite.pivot;

    // 2. Apply pre-calculated Scale+Rotate+Aspect Matrix
    // m_row1 holds m00, m01
    // m_row2 holds m10, m11
    // x_new = local.x * m00 + local.y * m01
    // y_new = local.x * m10 + local.y * m11
    vec2 rotated;
    rotated.x = local.x * sprite.m_row1.x + local.y * sprite.m_row1.y;
    rotated.y = local.x * sprite.m_row2.x + local.y * sprite.m_row2.y;
    
    // 3. Add to Pivot position (NDC)
    gl_Position = vec4(rotated + sprite.pos, 0.0, 1.0);
}
