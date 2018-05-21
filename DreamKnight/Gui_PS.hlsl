Texture2D g_MeshTexture : register(t0);

SamplerState MeshTextureSampler : register(s0);

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 uv : UV;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(1, 0, 0, 1);
	float4 tex = g_MeshTexture.Sample(MeshTextureSampler, input.uv.xy);
	if (tex.a == 0.0f)
        discard;
	return tex;
}