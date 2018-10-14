struct VS_IN
{
	float3 pos : POSITION;
	float3 col : COLOR
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
	output.col = input.col;
	return output;
}