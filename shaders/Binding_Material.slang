import types;
// 材质
struct MaterialParameters
{
    float4 baseColorFactor = float4(1.0f);
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
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