#version 450

// Vertex attributes (per vertex - defines unit quad [0,0] to [1,1])
layout(location = 0) in vec2 in_position;   // Quad corner position (0-1 range)
layout(location = 1) in vec2 in_texcoord;   // UV template (0-1 range)
layout(location = 2) in vec4 in_color;      // Color modulation

// Sprite instance data in storage buffer
struct SpriteInstance {
    vec2 pos;           // Sprite world position (pixels)
    vec2 size;          // Sprite size in pixels (w, h)
    vec2 scale;         // Sprite scale factors
    float angle;        // Sprite rotation angle (radians)
    vec4 uv;            // Sprite UV rect (x, y, w, h) normalized 0-1
    vec2 pivot;         // Sprite pivot point (pixels)
    vec2 offset;        // Sprite offset (pixels)
};

// Storage buffer for sprite instances (readonly)
layout(std430, set = 0, binding = 0) readonly buffer SpriteInstanceBuffer {
    SpriteInstance sprites[];
};

// Push constants for matrices
layout(push_constant) uniform PushConstants {
    mat4 projection;    // Orthographic projection matrix
    mat4 view;          // View/camera matrix
} push;

// Output to fragment shader
layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;

void main() {
    // Get instance ID (which sprite we're rendering)
    uint instanceID = gl_InstanceIndex;
    SpriteInstance sprite = sprites[instanceID];
    
    // Calculate UV coordinates from sprite's UV rect (already normalized 0-1)
    vec2 uv = sprite.uv.xy + in_texcoord * sprite.uv.zw;
    
    // Convert unit quad position to sprite pixel size
    // in_position is [0,0] to [1,1], multiply by actual sprite size in pixels
    vec2 pixel_pos = in_position * sprite.size;
    
    // Apply offset (move sprite relative to its bounding box)
    vec2 offset_pos = pixel_pos + sprite.offset;
    
    // Apply pivot offset (pivot point in pixels, relative to top-left)
    vec2 pivoted_pos = offset_pos - sprite.pivot;
    
    // Apply rotation around pivot
    float s = sin(sprite.angle);
    float c = cos(sprite.angle);
    vec2 rotated_pos = vec2(
        pivoted_pos.x * c - pivoted_pos.y * s,
        pivoted_pos.x * s + pivoted_pos.y * c
    );
    
    // Apply scale
    vec2 scaled_pos = rotated_pos * sprite.scale;
    
    // Translate to world position (in pixels)
    vec2 world_pos = scaled_pos + sprite.pos;
    
    // Apply view and projection matrices
    gl_Position = push.projection * push.view * vec4(world_pos, 0.0, 1.0);
    
    // Pass data to fragment shader
    frag_texcoord = uv;
    frag_color = in_color;
}
