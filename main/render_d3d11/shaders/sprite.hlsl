cbuffer ViewConstants : register(b0)
{
	float2 viewportSize;
	float2 padding;
};

struct VSInput
{
	float3 position : POSITION;
	float rhw : TEXCOORD0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD1;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
	PSInput output;
	float2 ndc;
	ndc.x = input.position.x / viewportSize.x * 2.0f - 1.0f;
	ndc.y = 1.0f - input.position.y / viewportSize.y * 2.0f;
	output.position = float4(ndc, input.position.z, 1.0f);
	output.color = input.color;
	output.uv = input.uv;
	return output;
}

Texture2D spriteTexture : register(t0);
SamplerState spriteSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
	return spriteTexture.Sample(spriteSampler, input.uv) * input.color;
}
