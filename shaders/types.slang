module types;

public struct VSOutput
{
    public float4 Pos : SV_POSITION;
    public float2 UV;
};

//以下定义对应于C++端的types.hpp中的枚举DescriptorSetBindIndex
public static const uint LBI_GLOBAL = 0;
public static const uint LBI_IBL = 1;
public static const uint LBI_LIGHTS = 2;
public static const uint LBI_MATERIALS = 3;
public static const uint LBI_MESH = 4;
public static const uint LBI_CUSTOM = 5;