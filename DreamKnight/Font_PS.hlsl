Texture2D g_MeshTexture : register(t0);

SamplerState MeshTextureSampler : register(s0);

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 uv : UV;
	float4 rect : RECT;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(1, 0, 0, 1);
	float2 pos = input.rect.xy + input.uv.xy * input.rect.zw;
	float4 tex = g_MeshTexture.Sample(MeshTextureSampler, pos);
	if (tex.a == 0.0f)
        discard;
	return tex;
}