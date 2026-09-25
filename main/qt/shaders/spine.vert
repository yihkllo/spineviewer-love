#version 440
layout(location=0) in vec2 position;
layout(location=1) in vec4 color;
layout(location=2) in vec2 uv;
layout(location=3) in vec2 depthWeight;
layout(location=0) out vec4 vColor;
layout(location=1) out vec2 vUv;
layout(location=2) out vec2 vMaskUv;
layout(std140,binding=0) uniform Params {
    mat4 matrix;
    vec4 view;
} params;
void main() {
    gl_Position=params.matrix*vec4(position,depthWeight.x,1.0)/depthWeight.y;
    vColor=color; vUv=uv;
    vMaskUv=position/params.view.xy;
    if(params.view.w>0.5) vMaskUv.y=1.0-vMaskUv.y;
    vMaskUv*=gl_Position.w;
}
