texture zbufferTexture;

sampler zbufferSampler = sampler_state
{
	texture = <RAWZdepth>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MINFILTER = POINT;
	MAGFILTER = POINT;
};

struct VSOUT
{
	float4 vertPos : POSITION;
	float2 UVCoord : TEXCOORD0;
};

struct VSIN
{
	float4 vertPos : POSITION0;
	float2 UVCoord : TEXCOORD0;
};

VSOUT DummyVS(VSIN IN)
{
	VSOUT OUT = (VSOUT)0.0f;	// initialize to zero, avoid complaints.

	OUT.vertPos = IN.vertPos;
	OUT.UVCoord = IN.UVCoord;

	return OUT;
}


float4 TransferPS(VSOUT IN) : COLOR0
{
	return tex2D(currentFrameSampler, IN.UVCoord);
}

technique t0
{
	pass p0
	{
		VertexShader = compile vs_3_0 DummyVS();
		PixelShader = compile ps_3_0 TransferPS();
	}
}
