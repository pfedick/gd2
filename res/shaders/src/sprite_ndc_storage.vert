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
    float pos_z;        // Z-Depth (offset 24)
    float _pad;         // Padding (offset 28)
    vec4 uv;            // Sprite UV rect (x, y, w, h) normalized 0-1 (offset 32)
    vec4 uv_bounds;     // u_min, v_min, u_max, v_max (offset 48)
    vec2 pivot;         // Sprite pivot point (Normalized 0..1 relative to size)
    vec2 offset;        // Unused
    vec4 color;         // Color Modulation (offset 80)
};

// Storage buffer for sprite instances (readonly)
layout(std430, set = 0, binding = 0) readonly buffer SpriteInstanceBuffer {
    SpriteInstance sprites[];
};

// Output to fragment shader
layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;
layout(location = 2) flat out uint frag_instanceID;
layout(location = 3) flat out vec4 frag_uv_bounds;

void main() {
    // Get instance ID (which sprite we're rendering)
    uint instanceID = gl_InstanceIndex;
    SpriteInstance sprite = sprites[instanceID];
    frag_instanceID = instanceID;
    frag_uv_bounds = sprite.uv_bounds;
    
    // Calculate UV coordinates
    frag_texcoord = sprite.uv.xy + in_texcoord * sprite.uv.zw;
    
    // Pass color from instance data
    frag_color = sprite.color;

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
    gl_Position = vec4(rotated + sprite.pos, sprite.pos_z, 1.0);
}
