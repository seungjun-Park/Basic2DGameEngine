#pragma once

#include <string>

class Scene;
class Entity;
class Player;
class ResourceManager;
class PhysicsSystem;

struct SerializedEntity;
class CharacterAnimationSet;

class GameEntityFactory
{
public:
    GameEntityFactory(
        Scene& scene,
        ResourceManager& resources,
        PhysicsSystem& physics,
        const CharacterAnimationSet& enemyAnimations
    );

    bool IsSupportedType(
        const std::string& type
    ) const noexcept;

    Entity* Create(
        const SerializedEntity& data,
        Player* playerTarget
    );

    bool Serialize(
        const Entity& entity,
        SerializedEntity& outData
    ) const;

private:
    bool ApplySerializedState(
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