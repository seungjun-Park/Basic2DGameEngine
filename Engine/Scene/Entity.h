#pragma once

#include "Engine/Components/Transform.h"
#include "Engine/Components/Sprite.h"
#include "Engine/Collision/Collider.h"

class SpriteRenderer;

class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    virtual void Initialize()
    {
    }

    virtual void Update(
        float deltaTime
    )
    {
    }

    virtual void Render(
        SpriteRenderer& renderer
    );

    void Destroy();

    bool IsDestroyed() const;

public:
    Transform transform;
    Sprite sprite;
    Collider collider;

    bool m_destroyed = false;
    bool active = true;
};
