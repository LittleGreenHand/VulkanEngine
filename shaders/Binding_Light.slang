struct PointLightInfo {
    float4 position;
    float4 color;
    float range;
    int attenuationMode;
};
struct DirectLightInfo {
    float4 direct;
    float4 color;
    float4x4 directLightViewProj[8];
    int cascadeCount;
    int usePCF;
};
struct LightParameters
{
    PointLightInfo pointLights[16];
    int activePointLightCount = 1;

    DirectLightInfo directLight;
};
[[vk::binding(0, 2)]] ConstantBuffer<LightParameters> lightParameter;
[[vk::binding(1, 2)]] Sampler2D directLightShadowMap;



#define SHADOW_ATTENUATION_MIN 0
float textureProj(float4 shadowCoord, float2 off)
{
    float shadowFactor = 1.0; // 非阴影区域：直接光照完全贡献
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = directLightShadowMap.Sample(shadowCoord.xy + off).r;
        // 阴影区域：直接光照贡献衰减到最小值（非零）
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadowFactor = SHADOW_ATTENUATION_MIN;
        }
    }
    return shadowFactor; // 仅影响直接光照的贡献比例
}

float filterPCF(float4 sc)
{
    int2 texDim;
    directLightShadowMap.GetDimensions(texDim.x, texDim.y);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    // 3*3 PCF
    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += textureProj(sc, float2(dx * x, dy * y));
            count++;
        }
    }
    return shadowFactor / count;
}