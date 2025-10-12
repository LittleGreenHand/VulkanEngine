import "Binding_Global.slang";
import "Binding_IBL.slang";
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 UVW;
};

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
	return float4(color, 1.0);
}