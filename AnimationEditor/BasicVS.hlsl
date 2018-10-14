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
	float3 col : COLOR;
};

cbuffer PerFrame : register(b0)
{
	row_major float4x4 viewProj;
	float4 camPosition; 
}

cbuffer PerObject : register(b1)
{
	row_major float4x4 world;
}

VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 worldPos = mul(float4(input.pos, 1.0f), world);
	float4 worldNor = mul(float4(input.nor, 1.0f), world);
	output.pos = mul(worldPos, viewProj);
	output.col = mul(worldNor, viewProj);
	return output;
}