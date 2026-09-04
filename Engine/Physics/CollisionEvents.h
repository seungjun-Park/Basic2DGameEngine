#pragma once

#include "Engine/Scene/EntityHandle.h"

struct CollisionEnterEvent
{
    EntityHandle entityA{};
    EntityHandle entityB{};
};

struct CollisionExitEvent
{
    EntityHandle entityA{};
    EntityHandle entityB{};
};