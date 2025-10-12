import types;

[shader("vertex")]
VSOutput vertexMain(uint VertexIndex: SV_VertexID)
{
    VSOutput output;

    // 如果三角形被背面剔除，可以尝试修改顶点顺序
    const float2 positions[3] = {
        float2(-1.0f, -1.0f), // 左下
        float2(-1.0f, 3.0f),  // 左上（超出屏幕）
        float2(3.0f, -1.0f)   // 右下（超出屏幕）
    };

    output.UV = (positions[VertexIndex] + 1.0f) * 0.5f;
    output.Pos = float4(positions[VertexIndex], 0.0f, 1.0f);

    return output;
}