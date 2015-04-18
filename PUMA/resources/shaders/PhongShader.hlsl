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

cbuffer cbLights : register(b3)
{
	float4 lightPos[2];
};

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
	float3 norm : NORMAL;
	float3 viewVec : TEXCOORD0;
	float3 lightVec0 : TEXCOORD1;
	float3 lightVec1 : TEXCOORD2;
};

PSInput VS_Main(VSInput i)
{
	PSInput o = (PSInput)0;
	matrix worldView = mul(viewMatrix, worldMatrix);
	float4 viewPos = float4(i.pos, 1.0f);
	viewPos = mul(worldView, viewPos);
	o.pos = mul(projMatrix, viewPos);

	o.norm = mul(worldView, float4(i.norm, 0.0f)).xyz;
	o.norm = normalize(o.norm);
	
	o.viewVec = normalize(-viewPos.xyz);
	o.lightVec0 = normalize((mul(viewMatrix, lightPos[0]) - viewPos).xyz);
	o.lightVec1 = normalize((mul(viewMatrix, lightPos[1]) - viewPos).xyz);
	return o;
}

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float3 kd = 0.5, ks = 0.2f, m = 100.0f;

float4 PS_Main(PSInput i) : SV_TARGET
{
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 lightVec = normalize(i.lightVec0);
	float3 halfVec = normalize(viewVec + lightVec);
	float3 color = surfaceColor * ambientColor;
	color += lightColor * surfaceColor * kd * saturate(dot(normal, lightVec)) +
			 lightColor * ks * pow(saturate(dot(normal, halfVec)), m);
	lightVec = normalize(i.lightVec1);
	halfVec = normalize(viewVec + lightVec);
	color += lightColor * surfaceColor * kd * saturate(dot(normal, lightVec)) +
			 lightColor * ks * pow(saturate(dot(normal, halfVec)), m);
	return float4(saturate(color), surfaceColor.a);
}