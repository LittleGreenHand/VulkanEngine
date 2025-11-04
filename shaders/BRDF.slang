
#define PI 3.1415926535897932384626433832795

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

// 各向异性GGX法线分布函数(D)
// 描述微平面的朝向分布，考虑切线和副切线方向的差异
float D_GGXaniso(float dotNH, float dotXH, float dotYH, float ax, float ay)
{
    float a2 = ax * ay;
    float3 V = float3(ay * dotXH, ax * dotYH, a2 * dotNH);
    float S = dot(V, V);
    return (a2 / PI) * (a2 / (S * S));

    //float d = dotXH * dotXH / (ax * ax) + dotYH * dotYH / (ay * ay) + dotNH * dotNH;
    //return 1.0f / (PI * ax * ay * d * d);
}

// 各向异性Smith几何函数(G)
float G_SmithJointAniso(float ax, float ay, float NoV, float NoL, float XoV, float XoL, float YoV, float YoL)
{
    float Vis_SmithV = NoL * length(float3(ax * XoV, ay * YoV, NoV));
    float Vis_SmithL = NoV * length(float3(ax * XoL, ay * YoL, NoL));
    return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
}

// 将全局粗糙度Roughness和各向异性系数Anisotropy转为ax/ay
void getAnisotropicRoughness(float roughness, float anisotropy, out float ax, out float ay)
{
    anisotropy = clamp(anisotropy, -1.0, 1.0);
    ax = max(roughness * sqrt(1.0 + anisotropy), 0.001);
    ay = max(roughness * sqrt(1.0 - anisotropy), 0.001);
}