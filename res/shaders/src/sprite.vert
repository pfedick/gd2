#version 450

// Vertex attributes (per vertex - defines unit quad [0,0] to [1,1])
layout(location = 0) in vec2 in_position;   // Quad corner position (0-1 range)
layout(location = 1) in vec2 in_texcoord;   // UV template (0-1 range)
layout(location = 2) in vec4 in_color;      // Color modulation

// Instance attributes (per sprite)
layout(location = 3) in vec2 in_sprite_pos;     // Sprite world position (pixels)
layout(location = 4) in vec2 in_sprite_size;    // Sprite size in pixels (w, h)
layout(location = 5) in vec2 in_sprite_scale;   // Sprite scale factors
layout(location = 6) in float in_sprite_angle;  // Sprite rotation angle (radians)
layout(location = 7) in vec4 in_sprite_uv;      // Sprite UV rect (normalized 0-1)
layout(location = 8) in vec2 in_sprite_pivot;   // Sprite pivot point (pixels)
layout(location = 9) in vec2 in_sprite_offset;  // Sprite offset (pixels)

// Push constants for matrices
layout(push_constant) uniform PushConstants {
    mat4 projection;    // Orthographic projection matrix
    mat4 view;          // View/camera matrix
} push;

// Output to fragment shader
layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;

void main() {
    // Calculate UV coordinates from sprite's UV rect (already normalized 0-1)
    vec2 uv = in_sprite_uv.xy + in_texcoord * in_sprite_uv.zw;
    
    // Convert unit quad position to sprite pixel size
    // in_position is [0,0] to [1,1], multiply by actual sprite size in pixels
    vec2 pixel_pos = in_position * in_sprite_size;
    
    // Apply offset (move sprite relative to its bounding box)
    vec2 offset_pos = pixel_pos + in_sprite_offset;
    
    // Apply pivot offset (pivot point in pixels, relative to top-left)
    vec2 pivoted_pos = offset_pos - in_sprite_pivot;
    
    // Apply rotation around pivot
    float s = sin(in_sprite_angle);
    float c = cos(in_sprite_angle);
    vec2 rotated_pos = vec2(
        pivoted_pos.x * c - pivoted_pos.y * s,
        pivoted_pos.x * s + pivoted_pos.y * c
    );
    
    // Apply scale
    vec2 scaled_pos = rotated_pos * in_sprite_scale;
    
    // Translate to world position (in pixels)
    vec2 world_pos = scaled_pos + in_sprite_pos;
    
    // Apply view and projection matrices
    gl_Position = push.projection * push.view * vec4(world_pos, 0.0, 1.0);
    
    // Pass data to fragment shader
    frag_texcoord = uv;
    frag_color = in_color;
}
