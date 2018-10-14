struct VS_OUT
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

float4 PS_main(VS_OUT input)
{
	return float4(input.col, 1.0f);
}