#version 450

layout(location = 0) out vec2 frag_texcoord;

void main()
{
    // Generiert ein Fullscreen-Triangle basierend auf dem Vertex-Index (0, 1, 2)
    // Benötigt keinen Vertex-Input-Buffer!
    frag_texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(frag_texcoord * 2.0f - 1.0f, 0.0f, 1.0f);
    frag_texcoord.y = 1.0 - frag_texcoord.y; 
}
