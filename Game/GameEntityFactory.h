#pragma once

#include "Engine/Scene/EntityHandle.h"

#include <string>

class CharacterAnimationSet;
class Entity;
class PhysicsSystem;
class ResourceManager;
class Scene;

struct SerializedEntity;

class GameEntityFactory
{
public:
    GameEntityFactory(
        Scene& scene,
        ResourceManager& resources,
        PhysicsSystem& physics,
        const CharacterAnimationSet& enemyAnimations
    );

    Entity* Create(
        const SerializedEntity& data,
        EntityHandle playerTarget
    );

    bool Serialize(
        const Entity& entity,
        SerializedEntity& outData
    ) const;

    bool IsSupportedType(
        const std::string& type
    ) const noexcept;

private:
    bool ApplySerializedState(
        Entity& entity,
        const SerializedEntity& data
    );

    bool ApplySerializedAnimation(
        Entity& entity,
        const SerializedEntity& data
    );

private:
    Scene& m_scene;

    ResourceManager& m_resources;

    PhysicsSystem& m_physics;

    const CharacterAnimationSet&
        m_enemyAnimations;
};