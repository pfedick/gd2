#version 450

// Vertex attributes (per vertex - defines unit quad [0,0] to [1,1])
layout(location = 0) in vec2 in_position;   // Quad corner position (0-1 range)
layout(location = 1) in vec2 in_texcoord;   // UV template (0-1 range)
layout(location = 2) in vec4 in_color;      // Color modulation

// Push constants for matrices
layout(push_constant) uniform PushConstants {
    mat4 projection;    // Orthographic projection matrix
    mat4 view;          // View/camera matrix
} push;

// Output to fragment shader
layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;

void main() {
    // Hardcoded test: Draw a 100x100 sprite at position (100, 100)
    vec2 sprite_pos = vec2(100.0, 100.0);
    vec2 sprite_size = vec2(100.0, 100.0);
    
    // Convert unit quad position to sprite pixel size
    vec2 pixel_pos = in_position * sprite_size;
    
    // Translate to world position (in pixels)
    vec2 world_pos = pixel_pos + sprite_pos;
    
    // Apply view and projection matrices
    gl_Position = push.projection * push.view * vec4(world_pos, 0.0, 1.0);
    
    // Pass data to fragment shader
    frag_texcoord = in_texcoord;
    frag_color = in_color;
}
