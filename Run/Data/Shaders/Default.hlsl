// Default HLSL


cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};


struct vs_input_t
{
	float3	localPosition : POSITION;
	float4	color		  : COLOR;
	float2	uv			  : TEXCOORD;
};

struct v2p_t
{
	float4	position : SV_Position;
	float4	color	 : COLOR;
	float2	uv		 : TEXCOORD;
};

// 2D Texture assigned to texture register slot 0
Texture2D diffuseTexture : register(t0);

// Sampler state assigned to sampler register slot 0
SamplerState diffuseSampler : register(s0);


v2p_t VertexMain(vs_input_t input)
{
	float4 position = float4(input.localPosition, 1.0f);
	float4 worldPosition = mul(ModelMatrix, position);		// model to world transform
	float4 viewPosition = mul(ViewMatrix, worldPosition);		// world to view transform
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);  // view to render & render to clip space transform (because render matrix is appended with projection matrix)

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;

	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 outputColor = textureColor * vertexColor * modelColor;

	return outputColor;
}