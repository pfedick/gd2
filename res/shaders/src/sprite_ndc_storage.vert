#version 450

// Vertex attributes (per vertex - defines unit quad [0,0] to [1,1])
layout(location = 0) in vec2 in_position;   // Quad corner position (0-1 range)
layout(location = 1) in vec2 in_texcoord;   // UV template (0-1 range)
layout(location = 2) in vec4 in_color;      // Color modulation

// Sprite instance data in storage buffer (NDC coordinates)
struct SpriteInstance {
    vec2 pos;           // Sprite position (NDC)
    vec2 size;          // Sprite size (NDC)
    vec2 scale;         // Sprite scale factors
    float angle;        // Sprite rotation angle (radians)
    float padding;      // Explicit padding to match C++ alignment (16-byte alignment forcée par vec4 uv)
    vec4 uv;            // Sprite UV rect (x, y, w, h) normalized 0-1
    vec2 pivot;         // Sprite pivot point (Normalized 0..1 relative to size)
    vec2 offset;        // Sprite offset (Unused for now, baked into pos)
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

    // Vertex Logic with Pivot and Rotation
    // 1. Start with Vertex 0..1
    vec2 pos = in_position; 
    
    // 2. Shift so Pivot is at local (0,0)
    // sprite.pivot is expected to be normalized (0..1)
    pos -= sprite.pivot; 
    
    // 3. Scale by Size (NDC dimensions)
    // Note: size.y is negative in our C++ setup to flip Y axis
    pos *= sprite.size * sprite.scale;
    
    // 4. Rotate
    float c = cos(sprite.angle);
    float s = sin(sprite.angle);
    // Standard rotation around (0,0) - which is now our pivot
    float x = pos.x * c - pos.y * s;
    float y = pos.x * s + pos.y * c;
    pos = vec2(x, y);
    
    // 5. Translate to Target Position (NDC)
    vec2 final_pos = pos + sprite.pos;
    
    gl_Position = vec4(final_pos, 0.0, 1.0);
}
