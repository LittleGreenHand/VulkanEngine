import types;
struct MeshInfo {
    float4x4 model;//当前帧的模型矩阵
    float4x4 prevModel;//上一帧的模型矩阵
    float4x4 jointMatrix[64];
    float jointcount;
};

[[vk::binding(0, LBI_MESH)]] ConstantBuffer<MeshInfo> meshInfo;