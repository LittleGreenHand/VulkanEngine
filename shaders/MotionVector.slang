
float4 EncodeVeclocity(float3 V, bool bHasPixelAnimation)
{
    //应用伽马编码，对运动向量的xy分量进行非线性压缩：小幅度运动保留更多精度，大幅度运动适当压缩
    V.xy = sign(V.xy) * sqrt(abs(V.xy)) * (2.0 / sqrt(2.0));

    // 编码XY分量（适配-2,2范围，避开0值）
    float4 EncodedVelocity;
    // 缩放因子：0.499*0.5 将原始[-2,2]范围映射到[-0.499, 0.499]，0.499是为了避开清屏颜色(0,0)
    // 偏移量：32767/65535 ≈ 0.5，最终范围[0.001, 0.999]
    EncodedVelocity.xy = V.xy * (0.499f * 0.5f) + 32767.0f / 65535.0f;

    //编码Z分量和像素动画标志（深度编码）
    uint Vz = asuint(V.z);
    // 高16位存储到Z通道：映射到[0,1]范围，加微小偏移避免0值
    EncodedVelocity.z = saturate(float((Vz >> 16) & 0xFFFF) * rcp(65535.0f) + (0.1 / 65535.0f));
    // 低16位存储到W通道，最低位携带bHasPixelAnimation标志
    EncodedVelocity.w = saturate(float(((Vz >> 0) & 0xFFFE) | uint(bHasPixelAnimation)) * rcp(65535.0f) + (0.1 / 65535.0f));

    return EncodedVelocity;
}

// 解码由Calculate3DVelocityWithEncoding编码的运动向量
// encoded: 编码后的float4向量
// outHasPixelAnimation: 输出是否存在像素动画
float3 DecodeVelocity(float4 encoded, out bool outHasPixelAnimation)
{
    float3 Velocity;

    // 解码XY分量
    float2 DecodedXY = (encoded.xy - 32767.0f / 65535.0f) / (0.499f * 0.5f);
    // 伽马编码的逆操作
    DecodedXY = sign(DecodedXY) * pow(abs(DecodedXY) * (sqrt(2.0) / 2.0), 2.0);
    Velocity.xy = DecodedXY;

    // 解码Z分量和动画标志
    // 从Z通道恢复高16位，W通道恢复低16位
    uint High16 = uint(encoded.z * 65535.0f) & 0xFFFF;
    uint Low16 = uint(encoded.w * 65535.0f) & 0xFFFF;

    // 拼接32位整数，转换回浮点Z分量
    uint VelocityZAsUInt = (High16 << 16) | (Low16 & 0xFFFE); // 清除标志位
    Velocity.z = asfloat(VelocityZAsUInt);

    // 提取动画标志（最低位）
    outHasPixelAnimation = (Low16 & 0x1) != 0;

    return Velocity;
}

// 计算3D运动向量并进行编码
float4 Calculate3DVelocity(float4 PackedVelocityA, float4 PackedVelocityC, float4 jitter)
{
    float2 ScreenPos = PackedVelocityA.xy / PackedVelocityA.w - jitter.xy;
    float2 PrevScreenPos = PackedVelocityC.xy / PackedVelocityC.w - jitter.zw;

    float DeviceZ = PackedVelocityA.z / PackedVelocityA.w;
    float PrevDeviceZ = PackedVelocityC.z / PackedVelocityC.w;

    // 包括相机与物体运动
    float3 Velocity = float3(ScreenPos - PrevScreenPos, DeviceZ - PrevDeviceZ);    
    return EncodeVeclocity(Velocity, false);
}