struct VSInput
{
    float3 Pos;
    float3 Normal;
    float2 UV;
    float3 Color;
    float4 Tangent;
};

//全局参数
struct GlobalParams {
	float4x4 view;
	float4x4 projection;
	float3 camPos;
    float exposure;
    float gamma;
    float globalRoughness;
    float globalMetallic;
};
[[vk::binding(0, 0)]] ConstantBuffer<GlobalParams> globalParams;

//PBR IBL
[[vk::binding(0, 1)]] SamplerCube samplerIrradiance;
[[vk::binding(1, 1)]] SamplerCube prefilteredMapSampler;
[[vk::binding(2, 1)]] Sampler2D samplerBRDFLUT;
[[vk::binding(3, 1)]] SamplerCube samplerEnv;