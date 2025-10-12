import types;
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
	float4x4 inverseView;
	float4x4 projection;
    float3 camPos;
    float exposure;
    float gamma;
};
[[vk::binding(0, LBI_GLOBAL)]] ConstantBuffer<GlobalParams> globalParams;