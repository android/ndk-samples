struct PSInput
{
    float3 color : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}
