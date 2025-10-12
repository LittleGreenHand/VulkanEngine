import types;
// PBR IBL
[[vk::binding(0, LBI_IBL)]] SamplerCube samplerIrradiance;
[[vk::binding(1, LBI_IBL)]] SamplerCube prefilteredMapSampler;
[[vk::binding(2, LBI_IBL)]] Sampler2D samplerBRDFLUT;
[[vk::binding(3, LBI_IBL)]] SamplerCube samplerEnv;