import "Binding_Global.slang";
import "Binding_Light.slang";
import "Binding_Material.slang";
import "Binding_Mesh.slang";

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos;
    float3 ViewPos;
    float3 Normal;
    float2 UV;
    float3 Tangent;
    float3 Color;
};

#define PI 3.1415926535897932384626433832795
//#define ALBEDO(uv) pow(baseColorTexture.Sample(uv).rgb, float3(2.2, 2.2, 2.2))
//#define ALBEDO(uv) pow(materialParameters.baseColorTextureEmpty ? float3(1.0, 0, 0) : baseColorTexture.Sample(uv).rgb, float3(2.2, 2.2, 2.2))

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
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// 法线分布函数(D)：GGX模型
// 描述微平面的朝向分布，决定高光的形状和集中度
// dotNH: 法向量与半程向量的点积
// roughness: 粗糙度，值越大表面越粗糙
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom);
}

// 几何函数(G)：Schlick-Smith GGX模型
// 模拟微平面之间的相互遮挡，影响高光的衰减
// dotNL: 法向量与光线方向的点积
// dotNV: 法向量与观察方向的点积
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// 菲涅尔函数(F)：Schlick近似
// 描述不同角度下光线的反射率变化，边缘通常反射更强
// cosTheta: 入射角的余弦值（观察方向与法向量的夹角）
// F0: 基础反射率（0度入射角时的反射率）
float3 F_Schlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// 改进的菲涅尔函数，考虑粗糙度影响
float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// 预过滤环境反射采样
// 根据粗糙度选择不同级别的mipmap，实现模糊的反射效果
// R: 反射方向向量
// roughness: 粗糙度，控制反射模糊程度
float3 prefilteredReflection(float3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
    float lodc = ceil(lod);
    float3 a = prefilteredMapSampler.SampleLevel(R, lodf).rgb;
    float3 b = prefilteredMapSampler.SampleLevel(R, lodc).rgb;
	return lerp(a, b, lod - lodf);
}

// 计算光照衰减
// 支持三种衰减模式：线性、平方反比、物理衰减
float calculateAttenuation(float distance, float lightRange, int attenuationMode)
{
    // 确保距离不小于0
    distance = max(distance, 0.0);
    
    // 如果距离超过光源范围，衰减为0
    if (distance > lightRange)
        return 0.0;
        
    switch(attenuationMode)
    {
        // 线性衰减：(1 - 距离/范围)
        case 0:
            return 1.0 - (distance / lightRange);
            
        // 平方反比衰减：1 / (1 + d + d²) - 物理上更准确
        case 1:
        {
            float d = distance / lightRange;
            return 1.0 / (1.0 + d + d * d);
        }
            
        // 物理衰减：基于真实光照衰减公式，更柔和
        case 2:
        default:
        {
            float d = distance / lightRange;
            float d2 = d * d;
            float d3 = d2 * d;
            return 1.0 / (1.0 + 2.0 * d + 2.0 * d2 + d3);
        }
    }
}

// 计算镜面反射贡献
// 综合运用D、G、F函数计算PBR中的直接光照镜面反射部分
// albedoColor: fragment颜色
// lightColor: 光源颜色
// L: 光线方向
// V: 观察方向
// N: 法向量
// F0: 基础反射率
// metallic: 金属度
// roughness: 粗糙度
float3 specularContribution(float3 albedoColor, float3 lightColor, float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness, float attenuation)
{
    // 计算半程向量（光线方向与观察方向的中间向量）
	float3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    float3 color = float3(0.0, 0.0, 0.0);

    // 只有当光线朝向表面时才计算光照贡献
    if (dotNL > 0.0) {
        // D = 正态分布函数（微平面分布）
        float D = D_GGX(dotNH, roughness);
        // G = 几何阴影函数（微平面遮挡）
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
        // F = 菲涅尔函数（反射率随角度变化）
        float3 F = F_Schlick(dotNV, F0);
        // 计算镜面反射项
        float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        // 计算漫反射系数：非金属表面才有漫反射
        float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
        // 累加光照贡献：漫反射 + 镜面反射
        color += (kD * (albedoColor) / PI + spec) * dotNL * lightColor * attenuation;
	}

	return color;
}
bool isZeroVector(float3 v) {
    float lenSq = dot(v, v);
    return lenSq < 1e-9;
}
// 计算世界空间中的法向量（考虑法线贴图）
// 从法线纹理中采样并通过TBN矩阵转换到世界空间
float3 calculateNormal(VSOutput input)
{
    if(materialParameters.normalTextureEmpty != 0)
    {
        return normalize(input.Normal);
    }

    // 从法线纹理采样并转换范围：[0,1] → [-1,1]
    float3 tangentNormal = normalTexture.Sample(input.UV).xyz * 2.0 - 1.0;

    // 构建TBN矩阵（切线-副切线-法线矩阵）
    // 世界空间法向量
	float3 N = normalize(input.Normal);
    // 世界空间切线向量
    float3 T = normalize(input.Tangent);
    if (isZeroVector(input.Tangent))
    {
        // 计算世界空间位置和纹理坐标的导数
        float3 dPdx = ddx(input.WorldPos);
        float3 dPdy = ddy(input.WorldPos);
        float2 dUVdx = ddx(input.UV);
        float2 dUVdy = ddy(input.UV);

        // 求解切线向量
        float3 T = normalize(dPdx * dUVdy.y - dPdy * dUVdx.y);
    }
    // 计算副切线向量
	float3 B = normalize(cross(N, T));
	float3x3 TBN = transpose(float3x3(T, B, N));

    // 将切线空间的法线转换到世界空间
	return normalize(mul(TBN, tangentNormal));
}

// 偏置矩阵，用于将坐标从[-1, 1]范围（标准化设备坐标 NDC）转换为[0, 1]范围（纹理采样坐标）
static const float4x4 biasMat = float4x4(
    0.5, 0.0, 0.0, 0.5,
    0.0, 0.5, 0.0, 0.5,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    output.WorldPos = mul(meshInfo.model, float4(input.Pos, 1.0)).xyz;
    output.ViewPos = mul(globalParams.view, float4(output.WorldPos.xyz, 1.0)).xyz;
    output.Normal = mul((float3x3)meshInfo.model, input.Normal);
    output.Tangent = mul((float3x3)meshInfo.model, input.Tangent.xyz);
	output.UV = input.UV;
    output.Pos = mul(globalParams.projection, mul(globalParams.view, float4(output.WorldPos, 1.0)));
    output.Color = input.Color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
	float3 N = calculateNormal(input);
	float3 V = normalize(globalParams.camPos - input.WorldPos);
    float3 R = reflect(-V, N);
    // 初始化默认金属度和粗糙度（1.0为默认值）
    float metallic = 1.0;
    float roughness = 1.0;

    float2 mr;
    if (materialParameters.mergeMetallicRoughnessTexture != 0)
    {
        if (materialParameters.metallicRoughnessTextureEmpty != 0)
        {
            mr = float2(1.0, 1.0);
        }
        else
        {
            mr = metallicRoughnessTexture.Sample(input.UV).rg;
        }
    }
    else
    {
        float metallicVal = 1.0;
        float roughnessVal = 1.0;

        if (!materialParameters.metallicTextureEmpty != 0)
        {
            metallicVal = metallicTexture.Sample(input.UV).r;
        }
        if (!materialParameters.roughnessTextureEmpty != 0)
        {
            roughnessVal = roughnessTexture.Sample(input.UV).r;
        }

        mr = float2(metallicVal, roughnessVal);
    }

    // 应用材质参数中的因子（最终值 = 纹理采样值 * 材质因子）
    metallic = mr.x * materialParameters.metallicFactor;
    roughness = mr.y * materialParameters.roughnessFactor;
    // 确保数值在有效范围内（0.0-1.0），避免异常值影响渲染
    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.0, 1.0);

    float4 albedoColor;
    if (materialParameters.baseColorTextureEmpty != 0) {
        albedoColor = float4(input.Color, 1.0);
    }
    else {
        albedoColor = baseColorTexture.Sample(input.UV);
        albedoColor.rgb = pow(albedoColor.rgb, float3(2.2, 2.2, 2.2));
    }
    float3 emission = emissiveTexture.Sample(input.UV).rgb;

    // 计算基础反射率F0
    // 非金属材质使用0.04的默认值，金属材质使用反照率颜色
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedoColor.rgb, metallic);

    // 累积点光源贡献
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < lightParameter.activePointLightCount; i++) {
        // 计算表面点到光源的向量和距离
        float3 lightDir = lightParameter.pointLights[i].position.xyz - input.WorldPos;
        float distance = length(lightDir);
        float3 L = normalize(lightDir);

        // 计算衰减因子
        float attenuation = calculateAttenuation(distance, lightParameter.pointLights[i].range, lightParameter.pointLights[i].attenuationMode);

        // 累加每个光源的贡献（已包含衰减）
        Lo += specularContribution(albedoColor.rgb, lightParameter.pointLights[i].color.xyz, L, V, N, F0,  metallic, roughness, attenuation);
    }

    uint cascadeIndex = 0;
    //计算定向光贡献
    {
        // 获取级联阴影贴图级别
        for (uint i = 0; i < lightParameter.directLight.cascadeCount - 1; ++i) {
            if (input.ViewPos.z < lightParameter.directLight.cascadeSplits[i]) {
                cascadeIndex = i + 1;
            }
        }

        //将世界位置转换到定向光坐标空间，并通过biasMat转换到NDC空间得到阴影采样坐标
        float4 shadowCoord = mul(biasMat, mul(lightParameter.directLight.ViewProj[cascadeIndex], float4(input.WorldPos, 1.0)));
        float shadow = 0;
        if (lightParameter.directLight.usePCF == 1) {
            shadow = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex);
        } else {
            shadow = textureProj(shadowCoord / shadowCoord.w, float2(0.0, 0.0), cascadeIndex);
        }

        float3 lightDir = lightParameter.directLight.direct.xyz;
        float3 L = normalize(lightDir);
        Lo += shadow * specularContribution(albedoColor.rgb, lightParameter.directLight.color.xyz, L, V, N, F0, metallic, roughness, 1.0);
    }

    // 采样BRDF查找表，获取环境镜面反射的预计算值
    // LUT 的横轴是dot(N, V)（视角与法线的夹角，反映 “观察方向的倾斜程度”），
    // 纵轴是粗糙度，存储的是 “BRDF 中除了环境光辐射亮度Li之外的、与材质和视角相关的积分结果”
    float2 brdf = samplerBRDFLUT.Sample(float2(max(dot(N, V), 0.0), roughness)).rg;

    // 采样环境反射（根据粗糙度获取不同模糊程度的反射）
    float3 reflection = prefilteredReflection(R, roughness).rgb;

    // 采样环境辐照度（用于环境漫反射）
    float3 irradiance = samplerIrradiance.Sample(N).rgb;

    // 计算环境漫反射贡献
    float3 diffuse = irradiance * albedoColor.rgb;

    // 计算环境光的菲涅尔项
    float3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

    // 计算环境镜面反射贡献
    float3 specular = reflection * (F * brdf.x + brdf.y);

    //获取环境光遮蔽
    float3 ao = materialParameters.AOTextureEmpty != 0 ? float3(1.0, 1.0, 1.0) : AOTexture.Sample(input.UV).rrr;

    // 计算环境光贡献
    // kD：漫反射系数，金属度越高漫反射越小
    float3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    float3 ambient = (kD * diffuse + specular) * ao;

    float3 color = ambient + Lo + emission;

    // 应用色调映射（HDR → LDR转换）
    color = Uncharted2Tonemap(color * globalParams.exposure);
    color = color * (1.0f / Uncharted2Tonemap((11.2f).xxx));
    // 应用伽马校正（线性空间 → sRGB空间）
    color = pow(color, (1.0f / globalParams.gamma).xxx);

    if (lightParameter.directLight.colorCascades == 1) {
        color *= getCascadeColor(cascadeIndex);
    }

    return float4(color, albedoColor.a);
}