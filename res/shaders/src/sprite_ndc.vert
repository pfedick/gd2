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
    // Test: Draw directly in NDC coordinates (center of screen, 0.5x0.5 size)
    vec2 ndc_pos = (in_position - 0.5) * 0.5;  // Range: -0.25 to +0.25 (centered square)
    
    // Output directly in NDC (ignore matrices completely)
    gl_Position = vec4(ndc_pos, 0.0, 1.0);
    
    // Pass data to fragment shader
    frag_texcoord = in_texcoord;
    frag_color = in_color;
}
