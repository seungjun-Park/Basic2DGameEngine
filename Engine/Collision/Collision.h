#pragma once

class Entity;

namespace Collision
{
    bool CheckAABB(
        const Entity& a,
        const Entity& b
    );
}