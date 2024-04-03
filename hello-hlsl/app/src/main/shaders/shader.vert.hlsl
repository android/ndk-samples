static const float2 positions[3] = {
    float2(0.0f, -0.5f),
    float2(0.5f, 0.5f),
    float2(-0.5f, 0.5f)
};

static const float3 colors[3] = {
    float3(0.67f, 0.1f, 0.2f),
    float3(0.67f, 0.1f, 0.2f),
    float3(0.67f, 0.1f, 0.2f)
};

cbuffer UniformBufferObject : register(b0, space0)
{
    float4x4 uboMVP : packoffset(c0);
};

struct VSOutput
{
    float3 color : TEXCOORD0;
    float4 position : SV_Position;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = mul(float4(positions[id], 0.0f, 1.0f), uboMVP);
    output.color = colors[id];
    return output;
}
