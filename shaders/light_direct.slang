/* Copyright (c) 2025, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */
struct PushConstants {
    float4x4 MVP;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants;

[shader("vertex")]
float4 vertexMain(float3 Pos) : SV_POSITION
{
    return mul(pushConstants.MVP, float4(Pos, 1.0));
}

[shader("fragment")]
float4 fragmentMain()
{
	return float4(1.0, 0.0, 0.0, 1.0);
}