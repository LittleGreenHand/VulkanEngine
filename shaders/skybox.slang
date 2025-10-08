#include "Binding_Global.slang"

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 UVW;
};

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
float3 Uncharted2Tonemap(float3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    float4x4 model = globalParams.view;
    //移除位置信息
    model[0][3] = 0;
    model[1][3] = 0;
    model[2][3] = 0;
    VSOutput output;
    output.UVW = input.Pos;
    // output.UVW.y = -output.UVW.y;
    output.Pos = mul(globalParams.projection, mul(model, float4(input.Pos.xyz, 1.0)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 color = samplerEnv.Sample(input.UVW).rgb;

    // Tone mapping
    color = Uncharted2Tonemap(color * globalParams.exposure);
	color = color * (1.0f / Uncharted2Tonemap((11.2f).xxx));
    // Gamma correction
    color = pow(color, (1.0f / globalParams.gamma).xxx);

	return float4(color, 1.0);
}