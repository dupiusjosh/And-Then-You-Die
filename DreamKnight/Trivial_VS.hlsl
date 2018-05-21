struct PerObjectBatchData
{
	matrix model;
	float4 color;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	PerObjectBatchData data[32];
	matrix view;
	matrix projection;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
    float4 orig_pos : POSITION;
	float4 color : COLOR;
    float4 norm : NORMAL;
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

VS_OUTPUT main(VS_INPUT input, unsigned int instID : SV_InstanceId)
{
	VS_OUTPUT output;

	float4 pos = input.position;
	pos.w = 1;
	pos = mul(pos, data[instID].model);
    output.orig_pos = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	output.color = data[instID].color;
	if (output.color.a < 0.0f)
	{
	    output.color = input.norm;
		output.color.a = 1.0f;
	}

    output.norm = mul(input.norm, data[instID].model);
	
	return output;
}
