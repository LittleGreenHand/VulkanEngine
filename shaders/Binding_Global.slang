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
    float4x4 viewProj;
    float4x4 prevViewProj;
    float4 jitter;
    float3 camPos;
    float nearPlane;
    float farPlane;
    float exposure;
    float gamma;
};
[[vk::binding(0, LBI_GLOBAL)]] ConstantBuffer<GlobalParams> globalParams;

//GBuffer纹理
[[vk::binding(1, LBI_GLOBAL)]] Sampler2D G_AlbedoAlpha;
[[vk::binding(2, LBI_GLOBAL)]] Sampler2D G_Normal;
[[vk::binding(3, LBI_GLOBAL)]] Sampler2D G_Tangent;
[[vk::binding(4, LBI_GLOBAL)]] Sampler2D G_Material;
[[vk::binding(5, LBI_GLOBAL)]] Sampler2D G_Position;
[[vk::binding(6, LBI_GLOBAL)]] Sampler2D G_MotionVector;