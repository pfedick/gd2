// Vertical Blur Fragment Shader
#version 450

// Texture Input -> Set 2, Binding 0
layout(set = 2, binding = 0) uniform sampler2D inputTexture;
// Params -> Set 3, Binding 0
// Hinweis: Wir nutzen Binding 0, das müssen wir im C++ Code bei PushGPUFragmentUniformData beachten!
layout(set = 3, binding = 0, std140) uniform BlurParams {
    float blurStrength;  // 0.0 - 1.0
    float _padding;
    vec2 texelSize;
};

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

void main() {
    vec4 color = vec4(0.0);
    float weights[9] = float[](
        0.13298, 0.125858, 0.106701, 0.081129, 0.055274, 0.03373, 0.018476, 0.009081, 0.003998
    );
    
    color += texture(inputTexture, texCoord) * weights[0];
    
    for(int i = 1; i < 9; ++i) {
        vec2 offset = vec2(0.0, texelSize.y * i * blurStrength);
        color += texture(inputTexture, texCoord + offset) * weights[i];
        color += texture(inputTexture, texCoord - offset) * weights[i];
    }
    
    fragColor = color;
}