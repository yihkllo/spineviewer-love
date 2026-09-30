#version 440

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_clip;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 projection;
    mat4 maskMatrix;
    vec4 baseColor;
    vec4 multiplyColor;
    vec4 screenColor;
    vec4 channelFlag;
    vec4 params;
};

layout(binding = 1) uniform sampler2D mainTexture;
layout(binding = 2) uniform sampler2D maskTexture;

void main()
{
    vec4 texColor = texture(mainTexture, v_uv);
    vec2 clip = v_clip.xy / v_clip.w;
    if (params.x < 0.5) {
        float isInside = step(baseColor.x, clip.x) * step(baseColor.y, clip.y) * step(clip.x, baseColor.z) * step(clip.y, baseColor.w);
        fragColor = channelFlag * texColor.a * isInside;
        return;
    }
    texColor.rgb = texColor.rgb * multiplyColor.rgb;
    if (params.y > 0.5)
        texColor.rgb = (texColor.rgb + screenColor.rgb * texColor.a) - (texColor.rgb * screenColor.rgb);
    else
        texColor.rgb = (texColor.rgb + screenColor.rgb) - (texColor.rgb * screenColor.rgb);
    vec4 color = texColor * baseColor;
    if (params.y < 0.5)
        color.rgb *= color.a;
    if (params.x > 1.5) {
        vec2 maskUv = vec2(clip.x * 0.5 + 0.5, params.z > 0.5 ? clip.y * 0.5 + 0.5 : 0.5 - clip.y * 0.5);
        vec4 clipMask = (1.0 - texture(maskTexture, maskUv)) * channelFlag;
        float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;
        color = color * (params.x > 2.5 ? 1.0 - maskVal : maskVal);
    }
    fragColor = color;
}
