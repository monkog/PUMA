Texture2D cloudMap : register(t0);
Texture2D opacityMap : register(t1);
SamplerState colorSampler : register(s0);

cbuffer cbView : register(b0) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b0) //Geometry Shader constant buffer slot 2
{
	matrix projMatrix;
};

struct VSInput
{
	float3 pos : POSITION;
	float age : TEXCOORD0;
	float angle : TEXCOORD1;
	float size : TEXCOORD2;
};

struct GSInput
{
	float4 pos : POSITION;
	float age : TEXCOORD0;
	float angle : TEXCOORD1;
	float size : TEXCOORD2;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

static const float TimeToLive = 4.0f;

GSInput VS_Main(VSInput i)
{
	GSInput o = (GSInput)0;
	o.pos = float4(i.pos, 1.0f);
	o.pos = mul(viewMatrix, o.pos);
	o.age = i.age;
	o.angle = i.angle;
	o.size = i.size;
	return o;
}

[maxvertexcount(4)]
void GS_Main(point GSInput inArray[1], inout TriangleStream<PSInput> ostream)
{
	GSInput i = inArray[0];
	float sina, cosa;
	sincos(i.angle, sina, cosa);
	float dx = (cosa - sina) * 0.5 * i.size;
	float dy = (cosa + sina) * 0.5 * i.size;
	float tex2x = i.age / TimeToLive;
	PSInput o = (PSInput)0;

	//TODO: Initialize o for 4 vertices to make a bilboard and append them to the ostream

	// Right up corner
	o = (PSInput)0;
	o.pos = float4(i.pos.x + dx, i.pos.y + dy, i.pos.z, 1.0);
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(1, 0);
	o.tex2 = float2(tex2x, 0.5f);
	ostream.Append(o);

	// Left down corner
	o = (PSInput)0;
	o.pos = float4(i.pos.x - dx, i.pos.y - dy, i.pos.z, 1.0);
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(0, 1);
	o.tex2 = float2(tex2x, 0.5f);
	ostream.Append(o);

	// Left up corner
	o = (PSInput)0;
	o.pos = float4(i.pos.x - dy, i.pos.y + dx, i.pos.z, 1.0);
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(0, 0);
	o.tex2 = float2(tex2x, 0.5f);
	ostream.Append(o);

	// Right Down corner
	o = (PSInput)0;
	o.pos = float4(i.pos.x + dy, i.pos.y - dx, i.pos.z, 1.0);
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(1, 1);
	o.tex2 = float2(tex2x, 0.5f);
	ostream.Append(o);

	ostream.RestartStrip();
}

float4 PS_Main(PSInput i) : SV_TARGET
{
	float4 color = cloudMap.Sample(colorSampler, i.tex1);
	float4 opacity = opacityMap.Sample(colorSampler, i.tex2);
	float alpha = color.a * opacity.a * 0.3f;
	if (alpha == 0.0f)
		discard;
	return float4(color.xyz, alpha);
}