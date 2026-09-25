#version 440
layout(location=0) in vec4 vColor;
layout(location=1) in vec2 vUv;
layout(location=2) in vec2 vMaskUv;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform Params {
    mat4 matrix;
    vec4 view;
} params;
layout(binding=1) uniform sampler2D beforeTexture;
layout(binding=2) uniform sampler2D afterTexture;
void main() {
    vec2 uv=vMaskUv*gl_FragCoord.w;
    if(any(lessThan(uv,vec2(0.0))) || any(greaterThan(uv,vec2(1.0)))) {
        fragColor=vec4(0.0);
        return;
    }
    fragColor=mix(texture(beforeTexture,uv),texture(afterTexture,uv),clamp(params.view.z,0.0,1.0))*vColor;
}
