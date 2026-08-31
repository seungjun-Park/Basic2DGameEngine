Texture2D SpriteTexture : register(t0);

SamplerState SpriteSampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer UVBuffer : register(b1)
{
    float4 gUVRect;
};

float4 main(PSInput input) : SV_TARGET
{
    return SpriteTexture.Sample(
        SpriteSampler,
        input.UV
    );
}