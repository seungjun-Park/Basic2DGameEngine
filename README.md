# Basic2DGameEngine

A personal 2D game engine built with **C++20, WinAPI, and DirectX 11**.

V1.0 started as a minimal 2D framework.
V1.1 focused on evolving it into a structured runtime with clearer **lifecycle, ownership, rendering, physics, and debugging architecture**.

---

## V1.0 → V1.1

| Area            | V1.0                    | V1.1                                                   |
| --------------- | ----------------------- | ------------------------------------------------------ |
| Main Loop       | Variable Update         | FixedUpdate + Update + LateUpdate                      |
| Physics         | AABB Collision          | Box2D PhysicsSystem                                    |
| Rendering       | Direct Sprite Rendering | RenderQueue + Sorting + Batching                       |
| Entity Lifetime | Direct References       | EntityHandle + Deferred Destruction                    |
| Resources       | Texture Cache           | Texture / Tile / Animation / Audio                     |
| Runtime Systems | Minimal                 | Animation / Audio / TileMap / EventBus / Serialization |
| Debugging       | Basic Debug Render      | ImGui + CPU Profiler + Runtime Stats                   |

---

## Key Engineering Points

### Render Pipeline

```text id="5mqhi4"
Scene / TileMap
      ↓
 RenderQueue
      ↓
     Sort
      ↓
    Batch
      ↓
 SpriteRenderer
      ↓
 DirectX 11
```

Commands are ordered by:

```text id="7s0yjg"
RenderLayer
→ sortZ
→ submission order
```

In the validation scene, batching reduced draw calls to approximately **1/10 of the previous count**.

### Runtime Identity

```text id="b3t3wq"
EntityHandle
    ↓
Scene::ResolveEntity()
    ↓
temporary Entity*
```

Entity destruction is deferred so logical death and physical deletion are separated.

### Physics Lifetime

```text id="34h4w9"
Box2D userData
      ↓
PhysicsBodyUserData
      ↓
EntityHandle
      ↓
Scene::ResolveEntity()
```

Persistent raw `Entity*` pointers are not stored in Box2D callback data.

### Ownership Policy

```text id="ncr0m5"
std::unique_ptr<T>  → ownership
T& / T*             → non-owning dependency / observer
EntityHandle        → persistent Entity identity
AudioPlaybackHandle → persistent playback identity
ComPtr<T>           → Direct3D resource ownership
```

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
* JSON scene serialization
* Dear ImGui runtime tools
* CPU profiler

---

## Technology

`C++20` · `WinAPI` · `DirectX 11` · `Box2D` · `XAudio2 2.9` · `Dear ImGui` · `nlohmann/json` · `vcpkg`

Validated with:

```text id="5dh11b"
Debug x64
Release x64
```

---

## Architecture

```text id="trfl6e"
Application
├─ WinWindow
└─ Engine
   ├─ Scene
   ├─ PhysicsSystem
   ├─ AudioSystem
   ├─ EventBus
   ├─ ResourceManager
   ├─ RenderQueue
   ├─ SpriteRenderer
   ├─ DX11Renderer
   ├─ GuiSystem
   └─ CpuProfiler
```

---

## Development

Developed using an **AI-assisted workflow** for design review, implementation planning, debugging hypotheses, and code audit.

Final architecture decisions, code integration, refactoring, and runtime validation were performed by the developer.

Reference: *Game Engine Architecture, 3rd Edition*.

