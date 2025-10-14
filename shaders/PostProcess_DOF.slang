import "Binding_Global.slang";
import types;
#define PI 3.1415926535897932384626433832795
struct DOFParams {
    float zNear;
    float zFar;
    float focusDistance; // 焦点距离
    float focusRange;    // 焦点范围（清晰区域）
    float maxBlurRadius; // 最大模糊半径
    float aperture;      // 光圈大小（影响模糊强度）
    float2 texelSize;   // 纹理单元大小（1/输出纹理宽度, 1/输出纹理高度）
};

[[vk::binding(0, LBI_CUSTOM)]] ConstantBuffer<DOFParams> dofParams;
[[vk::binding(1, LBI_CUSTOM)]] Sampler2D colorTexture;
[[vk::binding(2, LBI_CUSTOM)]] Sampler2D depthTexture;

// 线性化深度值（将[0,1]范围的深度值转换为线性视图空间深度）
float LinearizeDepth(float depth, float nearPlane, float farPlane) {
    float z = depth * 2.0 - 1.0; // 转换为NDC
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

// 计算模糊半径（CoC - Circle of Confusion）
float CalculateCoC(float depth, float focusDistance, float focusRange, float maxBlurRadius, float aperture) {
    // 计算与焦点距离的偏差
    float delta = abs(depth - focusDistance);

    // 如果在焦点范围内，不模糊
    if (delta < focusRange * 0.5) {
        return 0.0;
    }

    // 计算模糊强度，基于距离焦点的距离和光圈大小
    float blurStrength = (delta - focusRange * 0.5) * aperture;

    // 限制最大模糊半径
    return clamp(blurStrength, 0.0, maxBlurRadius);
}

// 生成高斯权重（用于更自然的模糊）
float GaussianWeight(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (sqrt(2.0 * PI) * sigma);
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    // 采样当前像素的深度
    float depth = depthTexture.Sample(input.UV).x;
    float linearDepth = LinearizeDepth(depth, dofParams.zNear, dofParams.zFar);

    // 计算当前像素的模糊半径
    float coc = CalculateCoC(linearDepth, dofParams.focusDistance, dofParams.focusRange, 
                            dofParams.maxBlurRadius, dofParams.aperture);

    // 如果不需要模糊，直接返回原始颜色
    if (coc <= 0.0) {
        return colorTexture.Sample(input.UV);
    }
    uint colorTextureWidth, colorTextureHeight;
    colorTexture.GetDimensions(colorTextureWidth, colorTextureHeight);
    // 计算实际采样半径（转换为纹理坐标单位）
    int sampleRadius = int(ceil(coc * dofParams.texelSize.x * colorTextureWidth));
    sampleRadius = clamp(sampleRadius, 1, 16); // 限制最大采样半径，平衡性能

    // 高斯模糊参数
    float sigma = float(sampleRadius) * 0.5;
    float weightSum = 0.0;
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);

    // 采样周围像素（反向映射：从周围像素向中心采样）
    for (int y = -sampleRadius; y <= sampleRadius; y++) {
        for (int x = -sampleRadius; x <= sampleRadius; x++) {
            // 计算采样偏移
            float2 offset = float2(x, y) * dofParams.texelSize;
            float2 sampleUV = input.UV + offset;

            // 防止采样超出纹理范围
            if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) {
                continue;
            }

            // 采样周围像素的深度和颜色
            float sampleDepth = depthTexture.Sample(sampleUV).x;
            float sampleLinearDepth = LinearizeDepth(sampleDepth, dofParams.zNear, dofParams.zFar);

            // 计算采样点的模糊半径
            float sampleCoC = CalculateCoC(sampleLinearDepth, dofParams.focusDistance, 
                                          dofParams.focusRange, dofParams.maxBlurRadius, dofParams.aperture);

            // 反向映射逻辑：只有当采样点的模糊半径大于当前偏移距离时才采用该采样
            float distance = length(float2(x, y));
            if (sampleCoC >= distance) {
                // 计算高斯权重
                float weight = GaussianWeight(distance, sigma);

                // 采样颜色并累积
                float4 sampleColor = colorTexture.Sample(sampleUV);
                finalColor += sampleColor * weight;
                weightSum += weight;
            }
        }
    }

    // 归一化并返回结果
    if (weightSum > 0.0) {
        finalColor /= weightSum;
    } else {
        finalColor = colorTexture.Sample(input.UV);
    }

    return finalColor;
}