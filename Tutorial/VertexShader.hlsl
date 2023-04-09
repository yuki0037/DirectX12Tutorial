struct VS_IN
{
	float4 mPosition : POSITION;
	float4 mColor : COLOR;
};

struct VS_OUT
{
	float4 mPosition : SV_POSITION;
	float4 mColor : COLOR;
};

VS_OUT main(VS_IN vsIn)
{
	VS_OUT vsOut;
	vsOut.mPosition = vsIn.mPosition;
	vsOut.mColor = vsIn.mColor;
	return vsOut;
}