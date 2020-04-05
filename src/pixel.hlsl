
struct vs_out
{
	float4 position : SV_POSITION;
	float3 color : COL;
};

float4 frag(vs_out input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}