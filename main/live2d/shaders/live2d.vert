#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_clip;

layout(std140, binding = 0) uniform buf {
    mat4 projection;
    mat4 maskMatrix;
    vec4 baseColor;
    vec4 multiplyColor;
    vec4 screenColor;
    vec4 channelFlag;
    vec4 params;
};

void main()
{
    v_uv = vec2(texcoord.x, 1.0 - texcoord.y);
    v_clip = maskMatrix * vec4(position, 0.0, 1.0);
    gl_Position = projection * vec4(position, 0.0, 1.0);
}
