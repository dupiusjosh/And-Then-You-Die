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

cbuffer BoneDataCBuffer : register(b1)
{
	matrix bones[64];
}

cbuffer BoneDataCBuffer : register(b2)
{
	matrix inverseBindBones[64];
}

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 color :		EXTRA_1_;
	float4 norm :		EXTRA_2_;
	float4 tan :		EXTRA_3_;
	float4 bitan :		EXTRA_4_;
	uint4 boneIndex :	EXTRA_5_;
	float4 boneWeight : EXTRA_6_;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 orig_pos : POSITION2;
	float4 uv : TEXCOORD0;
	float4 color : COLOR;
	float4 norm : NORMAL;
	bool ignoreLights : BOOL1;
};

PixelShaderInput main(VertexShaderInput input, unsigned int instID : SV_InstanceId)
{
	PixelShaderInput output;
	float4 pos = mul(mul(input.pos, inverseBindBones[input.boneIndex.x]), bones[input.boneIndex.x]) * input.boneWeight.x;
			pos += mul(mul(input.pos, inverseBindBones[input.boneIndex.z]), bones[input.boneIndex.z]) * input.boneWeight.z;
			pos += mul(mul(input.pos, inverseBindBones[input.boneIndex.w]), bones[input.boneIndex.w]) * input.boneWeight.w;
			pos += mul(mul(input.pos, inverseBindBones[input.boneIndex.y]), bones[input.boneIndex.y]) * input.boneWeight.y;
	pos.w = 1;
	pos = mul(pos, data[instID].model);
	output.orig_pos = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.pos = pos;
	output.uv = input.color;

	output.ignoreLights = (input.color.w > -2.1f && input.color.w > -1.9f);

	float4 norm = mul(mul(input.norm, inverseBindBones[input.boneIndex.x]), bones[input.boneIndex.x]) * input.boneWeight.x;
	norm += mul(mul(input.norm, inverseBindBones[input.boneIndex.z]), bones[input.boneIndex.z]) * input.boneWeight.z;
	norm += mul(mul(input.norm, inverseBindBones[input.boneIndex.w]), bones[input.boneIndex.w]) * input.boneWeight.w;
	norm += mul(mul(input.norm, inverseBindBones[input.boneIndex.y]), bones[input.boneIndex.y]) * input.boneWeight.y;
	norm = normalize(mul(norm, data[instID].model));

	output.color = data[instID].color;
	output.norm = norm;
	return output;
}
