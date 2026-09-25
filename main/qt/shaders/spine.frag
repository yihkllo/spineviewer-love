#version 440
layout(location=0) in vec4 vColor;
layout(location=1) in vec2 vUv;
layout(location=2) in vec2 vMaskUv;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform Params {
    mat4 matrix;
    vec4 view;
} params;
layout(binding=1) uniform sampler2D spriteTexture;
layout(binding=2) uniform sampler2D maskTexture;
void main() {
    vec4 pixel=texture(spriteTexture,vUv)*vColor;
    if(params.view.z>0.5) {
        float a=texture(maskTexture,vMaskUv*gl_FragCoord.w).a;
        pixel*=params.view.z>1.5?1.0-a:a;
    }
    fragColor=pixel;
}
