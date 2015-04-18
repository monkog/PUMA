TextureCube colorMap : register(t0);
SamplerState colorSampler : register(s0);

cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

cbuffer cbCameraPos : register(b3)
{
	float4 cameraPos;
}

cbuffer cbSurfaceColor : register(b0)
{
	float4 surfaceColor;
}

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 tex: TEXCOORD0;
};

PSInput VS_Main(VSInput i)
{
	PSInput o = (PSInput)0;
	o.pos = float4(i.pos, 1.0f);
	o.pos = mul(worldMatrix, o.pos);

	//TODO: calculate texture coordinates

	float4 incident = (o.pos - cameraPos);
	float4 normal = float4(i.norm, 0.0f);
	normal = mul(worldMatrix, normal);
	normal = normalize(normal);
	o.tex = reflect(incident, normal);

	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);
	
	return o;
}

float4 PS_Main(PSInput i) : SV_TARGET
{
	return surfaceColor*colorMap.Sample(colorSampler, normalize(i.tex));
}