// 材质
struct MaterialParameters
{
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    float4 baseColorFactor = float4(1.0f);
    bool baseColorTextureEmpty;
    bool normalTextureEmpty;
    bool mergeMetallicRoughnessTexture;
    bool metallicRoughnessTextureEmpty;
    bool metallicTextureEmpty;
    bool roughnessTextureEmpty;
    bool occlusionTextureEmpty;
    bool emissiveTextureEmpty;
    bool AOTextureEmpty;
    bool diffuseTextureEmpty;
    bool specularGlossinessTextureEmpty;
};
[[vk::binding(0, 3)]] ConstantBuffer<MaterialParameters> materialParameters;
[[vk::binding(1, 3)]] Sampler2D baseColorTexture;
[[vk::binding(2, 3)]] Sampler2D normalTexture;
[[vk::binding(3, 3)]] Sampler2D metallicRoughnessTexture;
[[vk::binding(4, 3)]] Sampler2D metallicTexture;
[[vk::binding(5, 3)]] Sampler2D roughnessTexture;
[[vk::binding(6, 3)]] Sampler2D occlusionTexture;
[[vk::binding(7, 3)]] Sampler2D emissiveTexture;
[[vk::binding(8, 3)]] Sampler2D AOTexture;
[[vk::binding(9, 3)]] Sampler2D diffuseTexture;
[[vk::binding(10, 3)]] Sampler2D specularGlossinessTexture;