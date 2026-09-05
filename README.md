# Basic2DGameEngine

A personal 2D game engine built with **C++20, WinAPI, and DirectX 11**.

V1.1 established the runtime architecture.
V1.2 focused on evolving it into a **data-driven engine with an integrated authoring workflow**, while preserving explicit ownership, lifetime, and runtime boundaries.

---

## V1.1 → V1.2

| Area          | V1.1                        | V1.2                                         |
| ------------- | --------------------------- | -------------------------------------------- |
| Editor        | Runtime/debug ImGui         | Integrated authoring workspace               |
| Configuration | C++-driven                  | `project.json` + Project Settings            |
| Scene         | Runtime serialization       | Editable/versioned Scene Document            |
| Assets        | Runtime resource loading    | Asset Browser + Inspector assignment         |
| Animation     | Runtime playback            | Clip authoring + persistent assignment       |
| TileMap       | Runtime rendering/collision | Tileset + Palette + Painting + Runtime Apply |
| Play Mode     | Direct simulation           | Edit Snapshot → Play → Restore               |
| Asset Binding | Gameplay-side paths         | Serialized data / semantic bindings          |
| Serialization | Basic JSON scene            | Backward-compatible SceneData V1 → V4        |

---

## Key Engineering Points

### Authoring Pipeline

```text
Editor
   ↓
Draft Data
   ↓
JSON
   ↓
Scene / ResourceManager
   ↓
Runtime
```

Disk state and runtime state are intentionally separated.

```text
            Draft
           /     \
        Save     Apply
         ↓         ↓
        JSON     Runtime
```

Therefore:

```text
Dirty != Runtime In Sync
```

---

### Edit / Play State

```text
Edit State
    ↓
Memory Snapshot
    ↓
Play Simulation
    ↓
Stop
    ↓
Restore Edit State
```

Stop restores the in-memory authored state instead of reloading from disk.

This preserves unsaved Inspector edits and runtime-applied TileMap changes while discarding simulation-only state.

---

### Stable Resource Editing

Runtime resources remain owned by `ResourceManager`.

```text
ResourceManager
├─ Texture
├─ AnimationClip
├─ Tileset
├─ TileMap
└─ AudioClip
```

Live editing updates resource contents without invalidating existing observers.

```text
Cached Resource*
      ↓
ReplaceContents(...)
      ↓
Same Address
New Contents
```

TileMap rebuilds follow:

```text
Build
  ↓
Validate
  ↓
Commit
```

so failed reconstruction does not corrupt the active runtime state.

---

### Runtime Identity

Persistent editor selection uses the existing runtime identity model.

```text
EntityHandle
    ↓
Scene::ResolveEntity()
    ↓
temporary Entity*
```

Long-lived raw `Entity*` pointers are not stored by editor state.

---

### Data-Driven Asset Binding

Gameplay asset paths were moved out of gameplay C++ and into serialized data.

```text
project.json
     ↓
 startScene
     ↓
 SceneData V4
 ┌────┼─────────────┐
 ↓    ↓             ↓
TileMap Animation   Audio
        Assets      Bindings
```

SceneData V4 also persists entity-level `AnimationClip` assignments.

Serializers store data without knowing gameplay semantics such as:

```text
Enemy.Idle.Down
Gameplay.BGM
```

Those meanings remain owned by the game layer.

SceneData V1–V3 remain load-compatible, while migration to V4 occurs only on explicit Save.

---

## Implemented Systems

* DirectX 11 sprite rendering
* RenderQueue / sprite batching
* Fixed-timestep simulation
* Box2D physics
* Animation
* TileMap rendering / collision
* XAudio2 audio
* EventBus
* ResourceManager
* EntityHandle lifetime model
* JSON scene serialization
* Dear ImGui editor workspace
* Project Settings / Asset Browser
* Scene Hierarchy / Inspector
* Animation authoring
* Tileset / Tile Palette / TileMap Editor
* Dirty / Save / Load / Revert document workflow
* Edit / Play snapshot restoration
* Versioned SceneData V1–V4
* Data-driven gameplay asset bindings
* CPU profiler / runtime statistics

---

## Technology

`C++20` · `WinAPI` · `DirectX 11` · `DirectXMath` · `Box2D` · `XAudio2 2.9` · `Dear ImGui` · `nlohmann/json` · `vcpkg`

Validated with:

```text
Debug x64
Release x64
```

---

## Architecture

```text
Application
├─ WinWindow
├─ Engine
│  ├─ Scene
│  ├─ PhysicsSystem
│  ├─ AudioSystem
│  ├─ EventBus
│  ├─ ResourceManager
│  ├─ RenderQueue
│  ├─ SpriteRenderer
│  ├─ DX11Renderer
│  ├─ GuiSystem
│  └─ CpuProfiler
│
└─ EditorSystem
   ├─ Project Settings
   ├─ Asset Browser
   ├─ Hierarchy / Inspector
   ├─ Animation Editor
   └─ Tileset / TileMap Authoring
```

The editor remains outside the Engine subsystem ownership graph.

```text
Editor      → Author
Serializer  → Persist
Engine      → Execute
Game Layer  → Interpret Semantics
```

---

## Development

Developed using an **AI-assisted workflow** for design review, implementation planning, debugging hypotheses, and code audit.

Final architecture decisions, code integration, refactoring, and **Debug / Release runtime validation** were performed by the developer.

Reference: *Game Engine Architecture, 3rd Edition*.
