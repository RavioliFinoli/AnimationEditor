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

VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;
	output.pos = float4(input.pos, 1.0f);
	output.col = input.nor;
	return output;
}