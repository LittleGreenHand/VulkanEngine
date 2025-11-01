import "MotionVector.slang";
// 预计算的泊松采样核（可在CPU生成后传入，减少Shader计算量）
#ifndef POISSON_KERNEL_SIZE
#define POISSON_KERNEL_SIZE 16  // 采样数，可动态调整
#endif

// 全局常量与结构体
struct MotionBlurConstants {
    float2  screenSize;       // 屏幕分辨率 (width, height)
    float2  ndcToPixelScale;  // NDC到像素空间的缩放因子 (screenSize.xy / 2.0)
    float   maxBlurRadius;    // 最大模糊半径（像素），避免过度模糊
    float   depthThreshold;   // 深度一致性阈值，控制跨物体模糊
    float   alphaWeightScale; // Alpha权重缩放（0.0~1.0，控制透明像素的贡献度）
};

[[vk::binding(0, 0)]] ConstantBuffer<MotionBlurConstants> constants;
[[vk::binding(1, 0)]] Sampler2D currentColor;
[[vk::binding(2, 0)]] Sampler2D motionVectors;
[[vk::binding(3, 0)]] Sampler2D DepthTexture;

// 输出资源
[[vk::binding(4, 0)]] RWTexture2D<float4> outputTexture;

[shader("compute")]
[numthreads(16, 16, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixelPos = dispatchThreadID.xy;
    if (pixelPos.x >= constants.screenSize.x || pixelPos.y >= constants.screenSize.y)
        return;

    float2 texCoord = float2(pixelPos) / constants.screenSize;

    // 解码运动向量
    bool hasPixelAnim;
    float3 velocityNDC = DecodeVelocity(motionVectors.Sample(texCoord), hasPixelAnim);
    float2 velocityXY_NDC = velocityNDC.xy;

    // 转换到位移像素空间并限制最大模糊
    float2 velocityXY_Pixel = velocityXY_NDC * constants.ndcToPixelScale;
    float blurLength = length(velocityXY_Pixel);
    float2 dir = blurLength > 0.001f ? normalize(velocityXY_Pixel) : float2(0, 0);

    // 自适应采样数
    int sampleCount = POISSON_KERNEL_SIZE;
    if (blurLength < 2.0f)
        sampleCount = max(4, sampleCount / 4);
    else if (blurLength < 8.0f)
        sampleCount = max(8, sampleCount / 2);

    blurLength = min(blurLength, constants.maxBlurRadius);
    velocityXY_Pixel = dir * blurLength;

    // 当前像素深度和Alpha（用于权重计算）
    float centerDepth = DepthTexture.Sample(texCoord).r;
    float centerAlpha = currentColor.Sample(texCoord).a; // 中心像素Alpha

    // 颜色和Alpha累积（带权重）
    float3 totalColor = 0.0f;
    float totalAlpha = 0.0f; // 累积Alpha
    float totalWeight = 0.0f;

    // 泊松采样核（同前）
    static const float2 poissonKernel[POISSON_KERNEL_SIZE] = {
        float2(0.5381, 0.1856), float2(-0.2460, 0.4837),
        float2(-0.4653, -0.3610), float2(0.3739, -0.4873),
        float2(0.1331, 0.6959), float2(-0.0667, -0.6345),
        float2(0.7139, -0.1012),  float2(-0.6532, 0.0150),
        float2(0.0100, 0.8362), float2(-0.7653, -0.2038),
        float2(0.4426, 0.6925),   float2(-0.1838, -0.7154),
        float2(0.6413, 0.3723), float2(-0.3866, 0.6038),
        float2(-0.5134, -0.5985), float2(0.2614, -0.8017)
    };

    for (int i = 0; i < sampleCount; i++)
    {
        float t = (i / (float)(sampleCount - 1)) - 0.5f;
        float2 poissonOffset = poissonKernel[i] * 0.5f;
        float2 sampleOffset_Pixel = velocityXY_Pixel * t + poissonOffset;
        float2 sampleTexCoord = texCoord + sampleOffset_Pixel / constants.screenSize;

        // 边界检查
        if (sampleTexCoord.x < 0.0f || sampleTexCoord.x > 1.0f ||
            sampleTexCoord.y < 0.0f || sampleTexCoord.y > 1.0f)
            continue;

        // 采样当前点的颜色（含Alpha）和深度
        float4 sampleColor = currentColor.Sample(sampleTexCoord);
        float sampleDepth = DepthTexture.Sample(sampleTexCoord).r;

        // 1. 深度权重（同前）
        float depthDiff = abs(sampleDepth - centerDepth);
        float depthWeight = 1.0f - smoothstep(0.0f, constants.depthThreshold, depthDiff);

        // 2. 距离权重（中心权重更高）
        float distWeight = 1.0f - abs(t);

        // 3. Alpha权重（关键：透明像素贡献度降低）
        // 若采样点Alpha低，权重降低；完全透明(alpha=0)则无贡献
        float alphaWeight = lerp(1.0f, sampleColor.a, constants.alphaWeightScale);

        // 总权重 = 深度权重 * 距离权重 * Alpha权重
        float weight = depthWeight * distWeight * alphaWeight;

        // 累积颜色和Alpha
        totalColor += sampleColor.rgb * weight;
        totalAlpha += sampleColor.a * weight; // 累积Alpha（带权重）
        totalWeight += weight;
    }

    // 处理权重为0的边界情况（直接用中心像素值）
    if (totalWeight <= 0.0f)
    {
        outputTexture[pixelPos] = currentColor.Sample(texCoord);
        return;
    }

    // 归一化并输出（RGB和Alpha分别归一化）
    float3 finalColor = totalColor / totalWeight;
    float finalAlpha = totalAlpha / totalWeight;

    // 对于不透明物体（中心Alpha=1），确保模糊后Alpha仍为1（避免半透明化）
    finalAlpha = lerp(finalAlpha, 1.0f, centerAlpha * 0.9f); // 柔和过渡

    outputTexture[pixelPos] = float4(finalColor, finalAlpha);
}