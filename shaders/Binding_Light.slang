struct LightInfo {
    float4 position;
    float3 color;
    float range;
    int attenuationMode;
};
struct LightParameters
{
    LightInfo lights[16];
    int activeLightCount = 4;
};
[[vk::binding(0, 2)]] ConstantBuffer<LightParameters> lightParameter;