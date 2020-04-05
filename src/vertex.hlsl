struct vs_in
{
	float2 position : POS;
	float3 color : COL;
};

struct vs_out
{
	float4 position : SV_POSITION;
	float3 color : COL;
};

vs_out vert(vs_in input)
{
	vs_out output = (vs_out)0;
	output.position = float4(input.position, 0.0f, 1.0f);
	output.color = float3(input.color);
	
	return output;
}