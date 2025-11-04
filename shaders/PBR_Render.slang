import "Binding_Global.slang";
import "Binding_IBL.slang";
import "Binding_Light.slang";
import "Binding_Material.slang";
import "Binding_Mesh.slang";
import "MotionVector.slang";
import "BRDF.slang";

struct VSOutput
{
    float4 Pos : SV_POSITION; // 顶点的裁剪空间位置，带有SV_POSITION语义，因此在片段着色器中不会被插值
    float3 WorldPos;
    float3 ViewPos;
    float3 Normal;
    float2 UV;
    float3 Tangent;
    float3 Color;
    float4 currentClipPos;
    float4 prevClipPos;
};
struct FSOutput
{
    float4 color : SV_TARGET0;
    float4 motionVector : SV_TARGET1;
};
// 0：不透明  1：遮罩  2：半透明
[SpecializationConstant] const int ALPHAMODE = 0;
#define PI 3.1415926535897932384626433832795

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

// 计算各向异性镜面反射贡献
float3 anisSpecularContribution(float3 albedoColor, float3 lightColor, float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness, float attenuation, 
                          float3 X, float3 Y,  float anisotropy)
{
    float3 H = normalize(V + L);

    // 计算各种点积
    float dotNV = dot(N, V);
    float dotNL = dot(N, L);
    float dotVL = dot(V, L);

    float dotNX = dot(N, X); // 通常为0，因为X与N正交
    float dotNY = dot(N, Y); // 通常为0，因为Y与N正交
    float dotLX = dot(L, X);
    float dotLY = dot(L, Y);

    float dotXV = dot(X, V);
    float dotYV = dot(Y, V);
    float dotXL = dot(X, L);
    float dotYL = dot(Y, L);

    float InvLenH = rsqrt(2 + 2 * dotVL);
    float dotNH = clamp((dotNL + dotNV) * InvLenH, 0.0, 1.0);
    float dotVH = clamp(InvLenH + InvLenH * dotVL, 0.0, 1.0);
    float dotXH = (dotXL + dotXV) * InvLenH;
    float dotYH = (dotYL + dotYV) * InvLenH;

    float3 color = float3(0.0, 0.0, 0.0);
    if (dotNL > 0.0) {
        // 计算各向异性粗糙度参数
        float ax, ay;
        getAnisotropicRoughness(roughness * roughness, anisotropy, ax, ay);

        // 各向异性NDF 
        float D = D_GGXaniso(dotNH, dotXH, dotYH, ax, ay);
        // 各向异性几何函数
        float G = G_SmithJointAniso(ax, ay, dotNV, dotNL, dotXV, dotXL, dotYV, dotYL);
        // 菲涅尔函数
        float3 F = F_Schlick(dotVH, F0);
         
        // 计算镜面反射项
        float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);

        // 计算漫反射系数
        float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);

        // 累加光照贡献
        color += (kD * albedoColor / PI + spec) * dotNL * lightColor * attenuation;
    }

    return color;
}
bool isZeroVector(float3 v) {
    float lenSq = dot(v, v);
    return lenSq < 1e-9;
}

struct NormalInfo
{
    float3 T; // 切线
    float3 B; // 副切线
    float3 N; // 法线
    float3x3 TBN; // TBN矩阵
};
// 计算世界空间中的法向量（考虑法线贴图）
// 从法线纹理中采样并通过TBN矩阵转换到世界空间
NormalInfo calculateNormal(VSOutput input)
{
    // 世界空间法向量
	float3 N = normalize(input.Normal);
    // 世界空间切线向量
    float3 T;
    if (materialParameters.anisotropicFactor != 0.f && !isZeroVector(materialParameters.tangent.xyz))
    {
        T = normalize(materialParameters.tangent.xyz);
    }
    else
    {
        T = normalize(input.Tangent);
        if (isZeroVector(input.Tangent))
        {
            // 计算世界空间位置和纹理坐标的导数
            float3 dPdx = ddx(input.WorldPos);
            float3 dPdy = ddy(input.WorldPos);
            float2 dUVdx = ddx(input.UV);
            float2 dUVdy = ddy(input.UV);

            // 求解切线向量
            T = normalize(dPdx * dUVdy.y - dPdy * dUVdx.y);
        }
    }

    // 计算副切线向量
	float3 B = normalize(cross(N, T));
    // 构建TBN矩阵（切线-副切线-法线矩阵）
	float3x3 TBN = transpose(float3x3(T, B, N));

    NormalInfo normalInfo;
    normalInfo.T = T;
    normalInfo.B = B;
    normalInfo.N = N;
    normalInfo.TBN = TBN;

    if (materialParameters.normalTextureEmpty == 0)
    {
        // 从法线纹理采样并转换范围：[0,1] → [-1,1]
        float3 tangentNormal = normalTexture.Sample(input.UV).xyz * 2.0 - 1.0;
        normalInfo.N = normalize(mul(TBN, tangentNormal));
    }
    return normalInfo;
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
    output.Pos = mul(globalParams.viewProj, float4(output.WorldPos, 1.0));
    output.currentClipPos = output.Pos;
    output.prevClipPos = mul(globalParams.prevViewProj, mul(meshInfo.prevModel, float4(input.Pos.xyz, 1.0)));
    output.Color = input.Color;
    return output;
}

[shader("fragment")]
FSOutput fragmentMain(VSOutput input)
{
    NormalInfo normalInfo = calculateNormal(input);
    float3 N = normalInfo.N;
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
    albedoColor.rgb *= materialParameters.baseColorFactor.rgb;
    if(ALPHAMODE == 1)
    {
        if (albedoColor.a < materialParameters.alphaCutoff)
        {
            //遮罩模式，alpha值为0，丢弃当前片段
            discard;
        }
    }
    float3 emission = emissiveTexture.Sample(input.UV).rgb;

    // 计算基础反射率F0
    // 非金属材质使用0.04的默认值，金属材质使用反照率颜色
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedoColor.rgb, metallic);

    // 累积点光源贡献
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < lightParameter.activePointLightCount; i++) {
        if (lightParameter.pointLights[i].isRnder == 1)
        {
            // 计算表面点到光源的向量和距离
            float3 lightDir = lightParameter.pointLights[i].position.xyz - input.WorldPos;
            float distance = length(lightDir);
            float3 L = normalize(lightDir);

            // 计算阴影因子
            const float EPSILON = 0.15;
            float sampledDist = pointLightShadowCubeMap.Sample(float4(-L, i)).r;
            float shadowFactor = (distance <= sampledDist + EPSILON) ? 1.0 : 0;

            // 计算衰减因子
            float attenuation = calculateAttenuation(distance, lightParameter.pointLights[i].range, lightParameter.pointLights[i].attenuationMode);

            // 累加每个光源的贡献（已包含衰减）
            if(materialParameters.anisotropicFactor != 0.0)
            {
                Lo += shadowFactor * anisSpecularContribution(albedoColor.rgb, lightParameter.pointLights[i].color.xyz, L, V, N, F0, metallic, roughness, attenuation,
                                                        normalInfo.T, normalInfo.B, materialParameters.anisotropicFactor);
            }
            else
            {
                Lo += shadowFactor * specularContribution(albedoColor.rgb, lightParameter.pointLights[i].color.xyz, L, V, N, F0, metallic, roughness, attenuation);        
            }
        }
    }
    uint cascadeIndex = 0;
    //计算定向光贡献
    if (lightParameter.directLight.isRnder == 1)
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
        if (materialParameters.anisotropicFactor != 0.0)
        {
            Lo += shadow * anisSpecularContribution(albedoColor.rgb, lightParameter.directLight.color.xyz, L, V, N, F0, metallic, roughness, 1.0,
                                                normalInfo.T, normalInfo.B, materialParameters.anisotropicFactor);            
        }
        else
        {
            Lo += shadow * specularContribution(albedoColor.rgb, lightParameter.directLight.color.xyz, L, V, N, F0, metallic, roughness, 1.0);
        }
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
    
    if (lightParameter.directLight.colorCascades == 1) {
        color *= getCascadeColor(cascadeIndex);
    }
    
    FSOutput output;
    output.color = float4(color, albedoColor.a);
    //output.color = float4(normalInfo.T, albedoColor.a);
    output.motionVector = Calculate3DVelocity(input.currentClipPos, input.prevClipPos, globalParams.jitter);

    return output;
}