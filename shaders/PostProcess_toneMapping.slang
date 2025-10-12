import "Binding_Global.slang";
import types;
[[vk::binding(0, LBI_CUSTOM)]] Sampler2D colorTexture;

// From http://filmicgames.com/archives/75
// 色调映射函数：Uncharted 2游戏使用的HDR到LDR转换算法
// 作用是将高动态范围的渲染结果压缩到适合显示器的低动态范围
float3 Uncharted2Tonemap(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float4 color = colorTexture.Sample(input.UV);
    // 应用色调映射（HDR → LDR转换）
    color.xyz = Uncharted2Tonemap(color.xyz * globalParams.exposure);
    color.xyz = color.xyz * (1.0f / Uncharted2Tonemap((11.2f).xxx));
    // 应用伽马校正（线性空间 → sRGB空间）
    color.xyz = pow(color.xyz, (1.0f / globalParams.gamma).xxx);
    return color;
}