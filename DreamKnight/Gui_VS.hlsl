struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 uv : UV;
};

struct VS_INPUT
{
	float4 position :	POSITION;
	float4 color :		EXTRA_1_;
	float4 norm :		EXTRA_2_;
	float4 tan :		EXTRA_3_;
	float4 bitan :		EXTRA_4_;
	uint4 boneIndex :	EXTRA_5_;
	float4 boneWeight : EXTRA_6_;
};

cbuffer BUFFER
{
	float4 position;//x, y, width, height
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	float2 charpos = position.xy;
	float2 sizePos = input.position.xy * position.zw;
	float2 pos = charpos + sizePos;

	output.position = float4(pos.x, 1 - pos.y, 0, 1);
	output.uv = float4(input.position.x, input.position.y, 0, 0);
	//output.rect = currentData.rect;

	return output;
}