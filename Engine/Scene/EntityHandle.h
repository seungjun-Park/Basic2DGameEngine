#pragma once

#include <cstdint>

struct EntityHandle
{
    using ValueType = std::uint64_t;

    static constexpr ValueType InvalidValue = 0;

    ValueType value = InvalidValue;

    constexpr bool IsValid() const noexcept
    {
        return value != InvalidValue;
    }

    constexpr explicit operator bool() const noexcept
    {
        return IsValid();
    }

    constexpr bool operator==(
        const EntityHandle&
        ) const noexcept = default;
};