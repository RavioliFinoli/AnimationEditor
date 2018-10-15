cbuffer PerFrame : register(b0)
{
	row_major float4x4 viewProj;
	float4 camPosition;
}

cbuffer PerObject : register(b1)
{
	row_major float4x4 world;
	row_major float4x4 skinningMatrices[128];
}

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : UV;
	float3 nor : NORMAL;
	float3 tan: TANGENT;
	uint4 boneIndices : JOINTINFLUENCES;
	float4 weights : JOINTWEIGHTS;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

VS_OUTPUT VS_main(VS_INPUT input)
{

	VS_OUTPUT output;

	//init array
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weights.x;
	weights[1] = input.weights.y;
	weights[2] = input.weights.z;
	weights[3] = input.weights.w;


	//Blend verts
	float3 position = float3(0.0f, 0.0f, 0.0f);
	float3 normal = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		if (input.boneIndices[i] < 129) //unused bone indices are negative
		{
			//TODO: cbuffer gWorld
			position += weights[i] * mul(float4(input.pos.xyz, 1.0f), skinningMatrices[input.boneIndices[i] - 1]).xyz;
			normal += weights[i] * mul(input.nor,
				skinningMatrices[input.boneIndices[i] - 1]).xyz;
		}
	}

	normal = normalize(normal);

	output.pos = mul(float4(position, 1.0f), mul(world, viewProj));
	output.col = mul(float4(normal, 0.0f), world).xyz;
	return output;
}