#include "Collision.h"

#include "Engine/Scene/Entity.h"

#include <cmath>

bool Collision::CheckAABB(
    const Entity& a,
    const Entity& b)
{
    if (!a.collider.enabled ||
        !b.collider.enabled)
    {
        return false;
    }

    float ax =
        a.transform.position.x +
        a.collider.offset.x;

    float ay =
        a.transform.position.y +
        a.collider.offset.y;

    float bx =
        b.transform.position.x +
        b.collider.offset.x;

    float by =
        b.transform.position.y +
        b.collider.offset.y;

    float aHalfWidth =
        a.collider.size.x * 0.5f;

    float aHalfHeight =
        a.collider.size.y * 0.5f;

    float bHalfWidth =
        b.collider.size.x * 0.5f;

    float bHalfHeight =
        b.collider.size.y * 0.5f;

    return
        std::abs(ax - bx)
        <= aHalfWidth + bHalfWidth
        &&
        std::abs(ay - by)
        <= aHalfHeight + bHalfHeight;
}