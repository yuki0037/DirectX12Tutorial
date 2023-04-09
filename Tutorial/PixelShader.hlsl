struct PS_IN
{
	float4 mPosition : SV_POSITION;
	float4 mColor : COLOR;
};

float4 main(PS_IN psIn) : SV_TARGET
{
	return psIn.mColor;
}