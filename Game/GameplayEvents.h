#pragma once

#include "Engine/Scene/EntityHandle.h"


struct EnemyDefeatedEvent
{
    //
    // 이미 defeat 처리된 Enemy의 runtime identity.
    //
    // Event dispatch 시점에는 Destroy()가 이미
    // 호출되었으므로 Scene::ResolveEntity(enemy)는
    // nullptr일 수 있는 것이 정상이다.
    //
    EntityHandle enemy{};

    //
    // Enemy를 처치한 Entity.
    //
    // 현재 GameScene에서는 Player.
    //
    EntityHandle instigator{};

    //
    // Enemy가 defeat되기 직전의 world position.
    //
    // Destroy 후에는 Enemy를 resolve할 수 없으므로
    // particles / audio / floating UI 등이 필요한
    // 위치를 event value로 보존한다.
    //
    float worldX = 0.0f;
    float worldY = 0.0f;
};