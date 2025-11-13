import types;
// 材质
struct MaterialParameters
{
    float4 baseColorFactor = float4(1.0f);
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    float anisotropicFactor = 0.0f;
    float4 tangent = float4(0.0f);
    float baseColorTextureEmpty;
    float normalTextureEmpty;
    float mergeMetallicRoughnessTexture;
    float metallicRoughnessTextureEmpty;
    float metallicTextureEmpty;
    float roughnessTextureEmpty;
    float occlusionTextureEmpty;
    float emissiveTextureEmpty;
    float AOTextureEmpty;
    float diffuseTextureEmpty;
    float specularGlossinessTextureEmpty;
};
[[vk::binding(0, LBI_MATERIALS)]] ConstantBuffer<MaterialParameters> materialParameters;
[[vk::binding(1, LBI_MATERIALS)]] Sampler2D baseColorTexture;
[[vk::binding(2, LBI_MATERIALS)]] Sampler2D normalTexture;
[[vk::binding(3, LBI_MATERIALS)]] Sampler2D metallicRoughnessTexture;
[[vk::binding(4, LBI_MATERIALS)]] Sampler2D metallicTexture;
[[vk::binding(5, LBI_MATERIALS)]] Sampler2D roughnessTexture;
[[vk::binding(6, LBI_MATERIALS)]] Sampler2D occlusionTexture;
[[vk::binding(7, LBI_MATERIALS)]] Sampler2D emissiveTexture;
[[vk::binding(8, LBI_MATERIALS)]] Sampler2D AOTexture;
[[vk::binding(9, LBI_MATERIALS)]] Sampler2D diffuseTexture;
[[vk::binding(10, LBI_MATERIALS)]] Sampler2D specularGlossinessTexture;

bool isZeroVector(float3 v) {
    float lenSq = dot(v, v);
    return lenSq < 1e-9;
}

struct NormalInfo {
    float3 T;     // 切线
    float3 B;     // 副切线
    float3 N;     // 世界空间法线（[-1,1]）
    float3x3 TBN; // TBN矩阵
};

/**
 * 计算并生成包含法线、切线、副切线及TBN矩阵的法线信息结构体
 * @param model 模型空间到世界空间的变换矩阵，用于将模型空间法线转换到世界空间
 * @param Normal 模型空间下的顶点法线（原始法线数据）
 * @param Tangent 模型空间下的顶点切线（原始切线数据，可能为零向量）
 * @param WorldPos 顶点在世界空间中的坐标位置
 * @param UV 顶点的纹理坐标，用于计算导数生成切线或采样法线贴图
 * @return 包含处理后法线、切线、副切线及TBN矩阵的NormalInfo结构体
 */
NormalInfo calculateNormal(float3 Normal, float3 Tangent, float3 WorldPos, float2 UV)
{
    NormalInfo info;
    // 1. 基础法线初始化
    info.N = normalize(Normal);
    // 2. 切线计算（优先用材质切线，其次用顶点切线，最后导数计算）
    if (materialParameters.anisotropicFactor != 0.f && !isZeroVector(materialParameters.tangent.xyz)) {
        info.T = normalize(materialParameters.tangent.xyz);
    } else {
        info.T = normalize(Tangent);
        if (isZeroVector(Tangent)) {
            float3 dPdx = ddx(WorldPos);
            float3 dPdy = ddy(WorldPos);
            float2 dUVdx = ddx(UV);
            float2 dUVdy = ddy(UV);
            info.T = normalize(dPdx * dUVdy.y - dPdy * dUVdx.y);
        }
    }
    // 3. 副切线与TBN矩阵
    info.B = normalize(cross(info.N, info.T));
    info.TBN = transpose(float3x3(info.T, info.B, info.N));
    // 4. 应用法线贴图（若有）
    if (materialParameters.normalTextureEmpty == 0) {
        float3 tangentNormal = normalTexture.Sample(UV).xyz * 2.0 - 1.0; // [0,1] → [-1,1]
        info.N = normalize(mul(info.TBN, tangentNormal));
    }
    return info;
}

void GetMetallicRoughness(float2 UV, out float metallic, out float roughness)
{
    float2 mr;
    if (materialParameters.mergeMetallicRoughnessTexture != 0)
    {
        if (materialParameters.metallicRoughnessTextureEmpty != 0)
        {
            mr = float2(1.0, 1.0);
        }
        else
        {
            mr = metallicRoughnessTexture.Sample(UV).rg;
        }
    }
    else
    {
        float metallicVal = 1.0;
        float roughnessVal = 1.0;

        if (!materialParameters.metallicTextureEmpty != 0)
        {
            metallicVal = metallicTexture.Sample(UV).r;
        }
        if (!materialParameters.roughnessTextureEmpty != 0)
        {
            roughnessVal = roughnessTexture.Sample(UV).r;
        }

        mr = float2(metallicVal, roughnessVal);
    }

    // 应用材质参数中的因子（最终值 = 纹理采样值 * 材质因子）
    // 确保数值在有效范围内（0.0-1.0），避免异常值影响渲染
    metallic = clamp(mr.x * materialParameters.metallicFactor, 0.0, 1.0);
    roughness = clamp(mr.y * materialParameters.roughnessFactor, 0.0, 1.0);
}