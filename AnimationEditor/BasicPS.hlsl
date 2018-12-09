struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 posW : POSITION;
	float4 norW : NORMAL;
};

cbuffer PER_FRAME : register(b0)
{
	float4 cameraPosition;
};

float4 PS_main(VS_OUT input) : SV_TARGET
{
	float4 normal = normalize(input.norW);
	const float AMBIENT_FACTOR = 0.2f;
	const float3 color = float3(0.8, 0.8, 0.8);
	float3 finalColor = AMBIENT_FACTOR * color;

	float3 lightVector = normalize(cameraPosition.xyz - input.posW.xyz);
	float diffuseFactor = max(dot(input.norW.xyz, lightVector), 0.0); //no attenuation
	return float4(finalColor + (color * diffuseFactor), 1.0f);
}