//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localTangent : TANGENT;
	float3 localBinormal : BINORMAL;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition : SV_Position;
	float4 worldPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 worldTangent : TANGENT;
	float4 worldBinormal : BINORMAL;
	float4 worldNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 WorldEyePosition;
	int NormalMode;
	int SpecularMode;
	float SpecularIntensity;
	float SpecularPower;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D specGlossEmitTexture : register(t2);

//------------------------------------------------------------------------------------------------
SamplerState samplerState : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 worldPosition = mul(ModelMatrix, float4(input.localPosition, 1));
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);

	v2p_t v2p;
	v2p.clipPosition = clipPosition;
	v2p.worldPosition = worldPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldTangent = mul(ModelMatrix, float4(input.localTangent, 0));
	v2p.worldBinormal = mul(ModelMatrix, float4(input.localBinormal, 0));
	v2p.worldNormal = mul(ModelMatrix, float4(input.localNormal, 0));
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(samplerState, input.uv);

	float3 worldNormal;
	if (NormalMode == 0)
	{
		float3x3 TBNMatrix = float3x3(normalize(input.worldTangent.xyz), normalize(input.worldBinormal.xyz), normalize(input.worldNormal.xyz));
		float3 tangentNormal = 2.0f * normalTexture.Sample(samplerState, input.uv).rgb - 1.0f;
		worldNormal = mul(tangentNormal, TBNMatrix);
	}
	else
	{
		worldNormal = normalize(input.worldNormal.xyz);
	}

	float specularIntensity = 0.0f;
	float specularPower = 0.0f;
	if (SpecularMode == 0)
	{
		float3 specGlossEmit = specGlossEmitTexture.Sample(samplerState, input.uv).rgb;
		specularIntensity = specGlossEmit.r;
		specularPower = 31.0f * specGlossEmit.g + 1.0f;
	}
	else
	{
		specularIntensity = SpecularIntensity;
		specularPower = SpecularPower;
	}

	float3 worldViewDirection = normalize(WorldEyePosition - input.worldPosition.xyz);
	float3 worldHalfVector = normalize(-SunDirection + worldViewDirection);
	float ndotH = saturate(dot(worldNormal, worldHalfVector));
	float specular = pow(ndotH, specularPower) * specularIntensity;

	float ambient = AmbientIntensity;
	float directional = SunIntensity * saturate(dot(normalize(worldNormal), -SunDirection));
	float4 lightColor = saturate(float4((ambient + directional + specular).xxx, 1));
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 color = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);
	return color;
}
