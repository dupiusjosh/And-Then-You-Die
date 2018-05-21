struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 uv : UV;
	float4 rect : RECT;
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

struct CharData
{
	float4 position;
	float4 size;//width, height, offset x, offset y
	float4 rect;//width, height, x, y
};

cbuffer BUFFER {
	CharData chardata[64];
}; 

VS_OUTPUT main(VS_INPUT input, unsigned int instID : SV_InstanceId)
{
	VS_OUTPUT output;

	CharData currentData = chardata[instID];
	float2 charpos = currentData.position.xy;
	float2 sizePos = input.position.xy * currentData.size.xy;
	float2 offsetPos = currentData.size.zw;
	//float2 offset = currentData.
	float2 pos = charpos + sizePos + offsetPos;

	output.position = float4(pos.x, 1-pos.y, 0, 1);
	output.uv = float4(input.position.x, input.position.y, 0, 0);
	output.rect = currentData.rect;

	return output;
}