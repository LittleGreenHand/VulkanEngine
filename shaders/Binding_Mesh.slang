import types;
struct MeshInfo {
    float4x4 model;
    float4x4 jointMatrix[64];
    float jointcount;
};

[[vk::binding(0, LBI_MESH)]] ConstantBuffer<MeshInfo> meshInfo;