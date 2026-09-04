#pragma once

#include <DirectXMath.h>

namespace PhysicsUnits
{
    constexpr float PixelsPerMeter =
        100.0f;

    inline float ToMeters(
        float pixels)
    {
        return
            pixels /
            PixelsPerMeter;
    }

    inline DirectX::XMFLOAT2
        ToMeters(
            const DirectX::XMFLOAT2& value)
    {
        return
        {
            ToMeters(value.x),
            ToMeters(value.y)
        };
    }


    inline float ToPixels(
        float meters)
    {
        return
            meters *
            PixelsPerMeter;
    }

    inline DirectX::XMFLOAT2
        ToPixels(
            const DirectX::XMFLOAT2& value)
    {
        return
        {
            ToPixels(value.x),
            ToPixels(value.y)
        };
    }
}