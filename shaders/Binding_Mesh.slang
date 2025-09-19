struct MeshInfo {
    float4x4 model;
    float4x4 jointMatrix[64];
    float jointcount;
};

[[vk::binding(0, 4)]] ConstantBuffer<MeshInfo> meshInfo;