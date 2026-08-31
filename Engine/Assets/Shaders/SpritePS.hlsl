Texture2D SpriteTexture : register(t0);

SamplerState SpriteSampler : register(s0);

cbuffer SpriteUVConstantBuffer : register(b0)
{
    float4 UVRect;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 atlasUV;

    atlasUV.x =
        lerp(
            UVRect.x,
            UVRect.z,
            input.UV.x
        );

    atlasUV.y =
        lerp(
            UVRect.y,
            UVRect.w,
            input.UV.y
        );

    return SpriteTexture.Sample(
        SpriteSampler,
        atlasUV
    );
}