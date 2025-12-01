# ECS Components Documentation

## Overview

This document describes the ECS (Entity Component System) components available in the OPC Game Simulator project. The project uses **EnTT** as the ECS library, following a data-oriented design pattern where components store data and systems implement logic.

## Core Architecture Principles

- **Components** are pure data structures (POD types)
- **Systems** contain logic and operate on entities with specific component combinations
- **Registry** (`entt::registry`) manages all entities and components

## Component Categories

### 1. Basic Transform & Rendering Components

#### TransformComponent

Stores spatial information for entities.

**Definition:**
```cpp
struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;      // Degrees
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};
```

**Usage Example:**
```cpp
auto entity = registry.create();
auto& transform = registry.emplace<TransformComponent>(entity);
transform.x = 100.0f;
transform.y = 200.0f;
transform.rotation = 45.0f;
transform.scaleX = 2.0f;
transform.scaleY = 2.0f;
```

**Systems using this component:**
- `UpdateSystem` - updates position based on velocity
- `RenderSystem` - uses transform for sprite positioning

---

#### SpriteComponent

Defines visual representation of entities.

**Definition:**
```cpp
struct SpriteComponent {
    std::string textureName;
    sf::Color color = sf::Color::White;
    int layer = 0;              // Rendering layer (lower = background)
    bool visible = true;
};
```

**Usage Example:**
```cpp
auto& sprite = registry.emplace<SpriteComponent>(entity);
sprite.textureName = "player_sprite";
sprite.color = sf::Color(255, 200, 200);  // Light red tint
sprite.layer = 5;
sprite.visible = true;
```

**Notes:**
- Textures must be loaded via `ResourceManager::loadTexture()` first
- Color is multiplied with texture (use `sf::Color::White` for no tint)
- Higher layer values render on top

**Systems using this component:**
- `RenderSystem` - renders sprite with texture and color

---

#### VelocityComponent

Stores movement data for dynamic entities.

**Definition:**
```cpp
struct VelocityComponent {
    float vx = 0.0f;            // Pixels per second (X-axis)
    float vy = 0.0f;            // Pixels per second (Y-axis)
    float angularVelocity = 0.0f;  // Degrees per second
};
```

**Usage Example:**
```cpp
auto& velocity = registry.emplace<VelocityComponent>(entity);
velocity.vx = 100.0f;    // Move 100 pixels/sec to the right
velocity.vy = -50.0f;    // Move 50 pixels/sec up
velocity.angularVelocity = 90.0f;  // Rotate 90 degrees/sec
```

**Systems using this component:**
- `UpdateSystem` - applies velocity to transform

---

#### CameraComponent

Marks an entity as a camera for rendering.

**Definition:**
```cpp
struct CameraComponent {
    float zoom = 1.0f;
    bool active = true;
};
```

**Usage Example:**
```cpp
auto camera = registry.create();
registry.emplace<TransformComponent>(camera);  // Camera position
auto& cam = registry.emplace<CameraComponent>(camera);
cam.zoom = 0.5f;  // Zoom out (2x view area)
cam.active = true;
```

**Notes:**
- Only one camera should be active at a time
- Zoom < 1.0 = zoom out, Zoom > 1.0 = zoom in
- Camera's `TransformComponent` defines view center

---

### 2. Utility Components (Milestone 1.2)

#### NameComponent

Provides human-readable names for debugging and identification.

**Definition:**
```cpp
struct NameComponent {
    std::string name;

    NameComponent() : name("Unnamed Entity") {}
    explicit NameComponent(const std::string& n) : name(n) {}
};
```

**Usage Example:**
```cpp
// Method 1: During creation
registry.emplace<NameComponent>(entity, "Player_Character");

// Method 2: Modify later
auto& nameComp = registry.get<NameComponent>(entity);
nameComp.name = "Enemy_01";
```

**Use Cases:**
- Debugging entity hierarchies
- Logging entity lifecycle events
- Serialization/deserialization
- Editor entity selection

**Example in LifetimeSystem:**
```cpp
if (registry.all_of<NameComponent>(entity)) {
    const auto& name = registry.get<NameComponent>(entity);
    LOG_DEBUG("Destroying entity '{}'", name.name);
}
```

---

#### TagComponent

Categorizes entities for filtering and searching.

**Definition:**
```cpp
struct TagComponent {
    std::string tag;

    TagComponent() : tag("") {}
    explicit TagComponent(const std::string& t) : tag(t) {}
};
```

**Usage Example:**
```cpp
// Tag entities for batch operations
registry.emplace<TagComponent>(player, "player");
registry.emplace<TagComponent>(enemy1, "enemy");
registry.emplace<TagComponent>(enemy2, "enemy");
registry.emplace<TagComponent>(powerup, "collectible");
```

**Finding tagged entities:**
```cpp
// Find all enemies
auto view = registry.view<TagComponent>();
for (auto entity : view) {
    const auto& tag = view.get<TagComponent>(entity);
    if (tag.tag == "enemy") {
        // Process enemy entity
    }
}
```

**Common Tag Examples:**
- `"player"`, `"enemy"`, `"npc"`
- `"projectile"`, `"particle"`
- `"ui_element"`, `"background"`
- `"sensor"`, `"actuator"`, `"conveyor"`

---

#### LifetimeComponent

Controls automatic entity destruction after a time period.

**Definition:**
```cpp
struct LifetimeComponent {
    float lifetime = 1.0f;      // Seconds remaining
    bool autoDestroy = true;    // Destroy when lifetime <= 0

    explicit LifetimeComponent(float time = 1.0f, bool destroy = true)
        : lifetime(time), autoDestroy(destroy) {}
};
```

**Usage Example:**
```cpp
// Create temporary particle that lives for 2 seconds
auto particle = registry.create();
registry.emplace<TransformComponent>(particle);
registry.emplace<SpriteComponent>(particle);
registry.emplace<LifetimeComponent>(particle, 2.0f, true);
```

**Advanced Usage - Timed Events:**
```cpp
// Timer entity (doesn't auto-destroy, just tracks time)
auto timer = registry.create();
registry.emplace<NameComponent>(timer, "GameTimer");
auto& lifetime = registry.emplace<LifetimeComponent>(timer, 10.0f, false);

// In update loop:
if (lifetime.lifetime <= 0.0f && !lifetime.autoDestroy) {
    LOG_INFO("Timer expired!");
    // Trigger event, reset timer, etc.
    lifetime.lifetime = 10.0f;  // Reset
}
```

**Use Cases:**
- Particle effects (explosions, smoke)
- Temporary UI notifications
- Projectiles with limited range
- Timed game events
- Buff/debuff durations

**Systems using this component:**
- `LifetimeSystem` - decrements lifetime and destroys entities

---

#### AnimationComponent

Manages sprite animation state (for future `AnimationSystem`).

**Definition:**
```cpp
struct AnimationComponent {
    std::string currentAnimation;  // Animation name (e.g., "walk", "idle")
    int currentFrame = 0;          // Current frame index
    float frameTime = 0.0f;        // Time accumulated for current frame
    float frameDelay = 0.1f;       // Seconds per frame
    bool loop = true;              // Loop animation when complete
    bool playing = true;           // Animation active state
};
```

**Usage Example:**
```cpp
auto character = registry.create();
registry.emplace<TransformComponent>(character);
registry.emplace<SpriteComponent>(character);

auto& anim = registry.emplace<AnimationComponent>(character);
anim.currentAnimation = "walk";
anim.frameDelay = 0.1f;  // 10 FPS animation
anim.loop = true;
anim.playing = true;
```

**Planned AnimationSystem Logic:**
```cpp
// Pseudocode for future AnimationSystem
void AnimationSystem::update(registry, dt) {
    for (auto entity : registry.view<AnimationComponent, SpriteComponent>()) {
        auto& anim = registry.get<AnimationComponent>(entity);

        if (!anim.playing) continue;

        anim.frameTime += dt;
        if (anim.frameTime >= anim.frameDelay) {
            anim.frameTime -= anim.frameDelay;
            anim.currentFrame++;

            if (anim.currentFrame >= animationData[anim.currentAnimation].frameCount) {
                if (anim.loop) {
                    anim.currentFrame = 0;
                } else {
                    anim.playing = false;
                }
            }

            // Update sprite texture rect based on currentFrame
        }
    }
}
```

**Animation Control:**
```cpp
// Change animation
anim.currentAnimation = "jump";
anim.currentFrame = 0;
anim.frameTime = 0.0f;
anim.playing = true;

// Pause/resume
anim.playing = false;  // Pause
anim.playing = true;   // Resume
```

---

### 3. Hierarchy Components

#### ParentComponent

Marks an entity as having a parent in the scene hierarchy.

**Definition:**
```cpp
struct ParentComponent {
    entt::entity parent = entt::null;

    ParentComponent() = default;
    explicit ParentComponent(entt::entity p) : parent(p) {}
};
```

**Usage Example:**
```cpp
// Create parent-child relationship
auto parent = registry.create();
auto child = registry.create();

registry.emplace<ParentComponent>(child, parent);
registry.get_or_emplace<ChildrenComponent>(parent).addChild(child);
```

**Transform Inheritance:**
```cpp
// Planned: Transform hierarchy system
void TransformHierarchySystem::update(registry) {
    auto view = registry.view<TransformComponent, ParentComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& parent = view.get<ParentComponent>(entity);

        if (parent.parent != entt::null && registry.valid(parent.parent)) {
            auto& parentTransform = registry.get<TransformComponent>(parent.parent);

            // Apply parent transform to child (world position)
            transform.x += parentTransform.x;
            transform.y += parentTransform.y;
            transform.rotation += parentTransform.rotation;
            transform.scaleX *= parentTransform.scaleX;
            transform.scaleY *= parentTransform.scaleY;
        }
    }
}
```

---

#### ChildrenComponent

Stores references to child entities.

**Definition:**
```cpp
struct ChildrenComponent {
    std::vector<entt::entity> children;

    void addChild(entt::entity child) {
        children.push_back(child);
    }

    void removeChild(entt::entity child) {
        children.erase(
            std::remove(children.begin(), children.end(), child),
            children.end()
        );
    }
};
```

**Usage Example:**
```cpp
auto parent = registry.create();
auto child1 = registry.create();
auto child2 = registry.create();

auto& children = registry.emplace<ChildrenComponent>(parent);
children.addChild(child1);
children.addChild(child2);

registry.emplace<ParentComponent>(child1, parent);
registry.emplace<ParentComponent>(child2, parent);
```

**Recursive Destruction:**
```cpp
void destroyWithChildren(entt::registry& registry, entt::entity entity) {
    if (registry.all_of<ChildrenComponent>(entity)) {
        auto& children = registry.get<ChildrenComponent>(entity);

        // Destroy children recursively
        for (auto child : children.children) {
            if (registry.valid(child)) {
                destroyWithChildren(registry, child);
            }
        }
    }

    registry.destroy(entity);
}
```

**Iteration Example:**
```cpp
// Update all children of a parent
if (registry.all_of<ChildrenComponent>(parent)) {
    const auto& children = registry.get<ChildrenComponent>(parent);

    for (auto child : children.children) {
        if (registry.valid(child)) {
            // Process child entity
            auto& transform = registry.get<TransformComponent>(child);
            transform.rotation += 10.0f;
        }
    }
}
```

---

## Systems

### LifetimeSystem

**Purpose:** Automatically destroys entities when their lifetime expires.

**Location:** `src/core/systems/LifetimeSystem.cpp`

**Update Logic:**
```cpp
void LifetimeSystem::update(entt::registry& registry, double dt) {
    auto view = registry.view<LifetimeComponent>();
    std::vector<entt::entity> entitiesToDestroy;

    // Decrease lifetime for all entities
    for (auto entity : view) {
        auto& lifetime = view.get<LifetimeComponent>(entity);
        lifetime.lifetime -= static_cast<float>(dt);

        // Mark for destruction if expired
        if (lifetime.lifetime <= 0.0f && lifetime.autoDestroy) {
            entitiesToDestroy.push_back(entity);
        }
    }

    // Destroy marked entities (outside iteration)
    for (auto entity : entitiesToDestroy) {
        if (registry.all_of<NameComponent>(entity)) {
            LOG_DEBUG("Destroying entity '{}'",
                registry.get<NameComponent>(entity).name);
        }
        registry.destroy(entity);
    }
}
```

**Integration in GameState:**
```cpp
// In GameState::initializeScene()
m_lifetimeSystem = std::make_unique<LifetimeSystem>();

// In GameState::update(double dt)
if (m_lifetimeSystem) {
    m_lifetimeSystem->update(m_registry, dt);
}
```

---

## Complete Usage Example

### Creating a Complete Entity

```cpp
// Create a rotating, temporary enemy entity
auto enemy = registry.create();

// Identification
registry.emplace<NameComponent>(enemy, "Enemy_Spawner_01");
registry.emplace<TagComponent>(enemy, "enemy");

// Spatial data
auto& transform = registry.emplace<TransformComponent>(enemy);
transform.x = 400.0f;
transform.y = 300.0f;
transform.scaleX = 1.5f;
transform.scaleY = 1.5f;

// Visual representation
auto& sprite = registry.emplace<SpriteComponent>(enemy);
sprite.textureName = "enemy_ship";
sprite.color = sf::Color::Red;
sprite.layer = 10;
sprite.visible = true;

// Movement
auto& velocity = registry.emplace<VelocityComponent>(enemy);
velocity.vx = -50.0f;  // Move left
velocity.angularVelocity = 45.0f;  // Spin

// Animation
auto& anim = registry.emplace<AnimationComponent>(enemy);
anim.currentAnimation = "idle";
anim.frameDelay = 0.15f;
anim.loop = true;

// Auto-destroy after 10 seconds
registry.emplace<LifetimeComponent>(enemy, 10.0f, true);

LOG_INFO("Created enemy entity with lifetime of 10s");
```

### Querying Entities

```cpp
// Find all visible enemies
auto view = registry.view<TagComponent, SpriteComponent>();
for (auto entity : view) {
    const auto& tag = view.get<TagComponent>(entity);
    const auto& sprite = view.get<SpriteComponent>(entity);

    if (tag.tag == "enemy" && sprite.visible) {
        // Process visible enemy
        auto& transform = registry.get<TransformComponent>(entity);
        LOG_DEBUG("Enemy at ({}, {})", transform.x, transform.y);
    }
}
```

### Component Composition Patterns

**Pattern 1: Static Sprite**
```cpp
Components: Transform + Sprite
Use case: Background objects, static decorations
```

**Pattern 2: Moving Sprite**
```cpp
Components: Transform + Sprite + Velocity
Use case: Moving platforms, scrolling backgrounds
```

**Pattern 3: Animated Character**
```cpp
Components: Transform + Sprite + Velocity + Animation + Name
Use case: Player, NPCs, enemies
```

**Pattern 4: Particle Effect**
```cpp
Components: Transform + Sprite + Velocity + Lifetime
Use case: Explosions, smoke, sparks
```

**Pattern 5: UI Element with Timer**
```cpp
Components: Transform + Sprite + Name + Lifetime(autoDestroy=false)
Use case: Timed notifications, cooldown indicators
```

**Pattern 6: Hierarchical Object**
```cpp
Parent: Transform + Sprite + Children
Child: Transform + Sprite + Parent
Use case: Robot arm (base + segments), car (body + wheels)
```

---

## Best Practices

### Component Design
1. **Keep components small** - Single responsibility principle
2. **Use POD types** - Trivially copyable when possible
3. **Avoid logic in components** - Pure data structures only
4. **Document units** - Specify pixels, meters, seconds, etc.

### System Design
1. **Batch processing** - Use `registry.view<>()` for efficiency
2. **Avoid entity creation in iteration** - Use separate vectors
3. **Check entity validity** - Use `registry.valid(entity)` before access
4. **Log important events** - Use NameComponent for meaningful logs

### Performance Tips
1. **Cache-friendly iteration** - EnTT stores components contiguously
2. **Minimize component size** - Smaller = better cache utilization
3. **Use `registry.group<>()` for frequent queries** - Pre-sorted sets
4. **Avoid `registry.get<>()` in hot paths** - Use view access

### Debugging
1. **Always add NameComponent** - Makes debugging much easier
2. **Use TagComponent for filtering** - Isolate entity categories
3. **Log entity lifecycle** - Creation, destruction, state changes
4. **Validate hierarchy** - Check parent/child validity

---

## Future Components (Planned)

### Phase 2: Industrial Components
- `SensorComponent` - Industrial sensors (presence, level, temperature)
- `ActuatorComponent` - Motors, valves, indicators
- `ConveyorComponent` - Conveyor belt simulation
- `TankComponent` - Fluid containers
- `PipeComponent` - Fluid transport

### Phase 3: OPC UA Integration
- `PLCBindingComponent` - OPC UA variable bindings
- `OPCUANodeComponent` - Node metadata

### Phase 4: Physics
- `RigidBodyComponent` - Box2D physics body reference
- `ColliderComponent` - Collision shapes
- `TriggerComponent` - Trigger zones

### Phase 5: Scripting
- `ScriptComponent` - Lua script attachment
- `EventListenerComponent` - Event subscription

---

## References

- **EnTT Documentation:** https://github.com/skypjack/entt
- **ECS Pattern:** https://en.wikipedia.org/wiki/Entity_component_system
- **Project Roadmap:** `ROADMAP.md` - Milestone 1.2
- **Component Definitions:** `include/core/Components.h`
- **System Implementations:** `src/core/systems/`

---

*Last Updated: November 2024*
*Milestone: 1.2 - ECS Base Components*
