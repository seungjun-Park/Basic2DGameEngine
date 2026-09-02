cbuffer SpriteFrameConstantBuffer : register(b0)
{
    matrix View;
    matrix Projection;
};

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 position =
        float4(
            input.Position,
            1.0f
        );

    position =
        mul(
            position,
            View
        );

    position =
        mul(
            position,
            Projection
        );

    output.Position =
        position;

    output.UV =
        input.UV;

    return output;
}