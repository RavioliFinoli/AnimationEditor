struct VS_IN
{
	float3 pos : POSITION;
	float2 uv : UV;
	float3 nor : NORMAL;
	float3 tan : TANGENT;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 posW : POSITION;
	float4 norW : NORMAL;
};

cbuffer PerFrame : register(b0)
{
	row_major float4x4 viewProj;
}

cbuffer PerObject : register(b1)
{
	row_major float4x4 world;
}

VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 worldPos = mul(float4(input.pos, 1.0f), world);
	output.posW = worldPos;
	float4 worldNor = mul(float4(input.nor, 1.0f), world);
	output.norW = worldNor;
	output.pos = mul(worldPos, viewProj);
	return output;
}