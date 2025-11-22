# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OPC Game Simulator is a 2D industrial automation simulator for PLC programming education with OPC UA integration. Built with C++20/23 using ECS architecture (EnTT), combining game sandbox mechanics with realistic industrial process simulation.

**Current Status:** Phase 1 (Basic Engine & Rendering) - Milestone 1.3 (Tile System) completed

## Build System & Dependencies

### Prerequisites
- **VCPKG_ROOT** environment variable must be set to vcpkg installation path
- C++20/23 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+

### Quick Build Commands

```bash
# Using convenience scripts
./build.sh              # Debug build (Linux/macOS)
./build.sh --release    # Release build
./build.sh --tests      # Build and run tests
./build.sh --clean      # Clean rebuild
build.bat              # Windows equivalent

# Using CMake presets
cmake --preset debug            # or: linux-debug, windows-debug
cmake --build --preset debug

# Manual CMake
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug --parallel
```

### CMake Build Options
- `BUILD_TESTS=ON/OFF` - Enable unit and integration tests (default: ON)
- `BUILD_DOCS=ON/OFF` - Enable Doxygen documentation (default: OFF)
- `ENABLE_TRACY=ON/OFF` - Enable Tracy profiler integration
- `ENABLE_MODBUS=ON/OFF` - Enable Modbus protocol support
- `ENABLE_OPENAL=ON/OFF` - Enable OpenAL 3D audio

### Testing

```bash
# All tests
ctest --test-dir build --output-on-failure

# Specific test suites (when implemented)
./build/bin/UnitTests
./build/bin/IntegrationTests

# With Catch2 filters (when tests exist)
./build/bin/UnitTests "[tag]"
```

### Code Formatting

```bash
# Format all C++ files
find src include -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# Check formatting (CI uses this)
find src include -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format --dry-run --Werror
```

### Critical SFML 3 API Changes

**⚠️ IMPORTANT:** This project uses SFML 3, which has breaking API changes from SFML 2. Always use the following patterns:

#### Event Handling (SFML 3 uses std::variant)
```cpp
// ❌ WRONG (SFML 2 style)
if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) { }
}

// ✅ CORRECT (SFML 3 style)
if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
    if (keyPressed->code == sf::Keyboard::Key::Escape) { }
}

// Mouse wheel scrolling
if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
    float delta = wheelScrolled->delta;
}
```

#### Vector and Geometric Types
```cpp
// ❌ WRONG (SFML 2 style)
sprite.setOrigin(0.0f, 0.0f);

// ✅ CORRECT (SFML 3 style)
sprite.setOrigin(sf::Vector2f(0.0f, 0.0f));
```

#### Primitive Types for Drawing
```cpp
// ❌ WRONG (SFML 2 style)
window.draw(vertices.data(), vertices.size(), sf::Lines);

// ✅ CORRECT (SFML 3 style)
window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::Lines);
```

### Dependency Notes

- **tmxlite requires zstd**: When using TileMapSystem, ensure `find_package(zstd REQUIRED)` is in CMakeLists.txt and `zstd::libzstd_static` is linked to the Rendering module
- **open62541 disabled on MinGW**: OPC UA temporarily disabled for Windows/MinGW builds due to compatibility issues

## Architecture

### Modular Structure

The project uses a **7-module architecture**, each built as a static library:

1. **Core** (`src/core/`, `include/core/`)
   - Application lifecycle, window management, game loop
   - State machine for app states (menu, game, pause)
   - Resource manager (textures, sounds)
   - Logging system (spdlog)

2. **Simulation** (`src/simulation/`, `include/simulation/`)
   - Box2D physics integration
   - Industrial process simulation
   - Collision handling
   - Runs in separate thread

3. **Rendering** (`src/rendering/`, `include/rendering/`)
   - SFML-based 2D rendering
   - Camera system (zoom, pan)
   - TMX tilemap rendering (tmxlite)
   - Multi-layer rendering with culling

4. **Industrial** (`src/industrial/`, `include/industrial/`)
   - OPC UA client (open62541)
   - PLC variable binding system
   - Subscription management
   - Runs in separate thread
   - Optional Modbus TCP/RTU support

5. **Scripting** (`src/scripting/`, `include/scripting/`)
   - Lua engine (sol2)
   - ECS API bindings
   - Event handling

6. **Editor** (`src/editor/`, `include/editor/`)
   - Level editor
   - Object placement
   - Project management (SQLite)
   - Save/load system

7. **UI** (`src/ui/`, `include/ui/`)
   - Dear ImGui integration
   - Inspector panels
   - OPC UA browser
   - Real-time plots (ImPlot)
   - Console logs

### ECS Pattern (EnTT)

**Components are pure data** (no logic):
- `TransformComponent` - position, rotation, scale (pixel coordinates)
- `SpriteComponent` - visual representation with layer-based rendering and sprite caching
- `TilePositionComponent` - tile-based positioning (converts to pixel coordinates)
- `AnimationComponent` - frame-based sprite animation
- `OverlayComponent` - parent-relative positioning for UI overlays
- `ParentComponent` / `ChildrenComponent` - parent-child hierarchy
- `SensorComponent` - industrial sensors (presence, level, temperature)
- `ActuatorComponent` - motors, valves, indicators
- `PLCBindingComponent` - OPC UA variable bindings
- `RigidBodyComponent` - Box2D physics body reference

**Systems contain logic** (no data):
- `RenderSystem` - renders sprites with layer sorting and Y-sorting
- `TilePositionSystem` - syncs tile coordinates to pixel coordinates and manages Y-sorting
- `AnimationSystem` - updates sprite animation frames
- `OverlaySystem` - syncs overlay positions with parent entities
- Iterate over entities with `registry.view<ComponentA, ComponentB>()`
- Update components based on game state
- Systems run in main loop or dedicated threads

### Multi-Threading Architecture

1. **Main Thread**: Input, UI (ImGui), rendering, coordination
2. **Physics Thread**: Box2D simulation, collision processing
3. **OPC UA Thread**: PLC communication, subscriptions, variable sync
4. **Script Thread** (optional): Lua execution, event processing

Communication via `boost::signals2` for thread-safe event handling.

### Data Flow: OPC UA Integration

```
PLC → OPC UA Server → OPC UA Client (Industrial module) →
BindingSystem → PLCBindingComponent → ECS entities →
Simulation/Rendering systems
```

### Game Loop Pattern

```
Input → Update Systems → Physics Step → OPC UA Sync →
Render (SFML) → UI (ImGui) → Present
```

### Coordinate System and Rendering

**Tile-Based Coordinate System:**
- **Tile size**: 32x32 pixels (constant `TILE_SIZE`)
- **Origin**: Bottom-left corner of sprites (NOT top-left as in traditional 2D)
- **TilePositionComponent**: Stores position in tile coordinates (e.g., (3, 6))
- **TransformComponent**: Stores position in pixel coordinates (e.g., (96, 224))

**Sprite Origin and Positioning:**
- RenderSystem sets sprite origin to **bottom-left corner** for non-rotating sprites
- This means `TransformComponent(x, y)` specifies where the bottom-left corner of the sprite is placed
- For rotating sprites, origin is set to center for correct rotation
- **Rationale**: Bottom-left origin is more intuitive for 2D games where objects "stand" on the ground

**Example:**
```cpp
// Object at tile (3, 6) with size 1x1 (32x32 pixels)
TilePositionComponent tilePos{3, 6, 1, 1};
// getPixelPosition() returns (3*32, (6+1)*32) = (96, 224)
// Sprite with origin at bottom-left will be drawn:
//   - Bottom-left at (96, 224)
//   - Top-left at (96, 192)
//   - Occupies tile (3, 6) visually
```

**Rendering Layers:**
- `Ground = 100` - Ground tiles and background
- `Objects = 200` - Game objects with Y-sorting (layer = 200 + tileY)
- `Overlays = 300` - UI overlays attached to objects (layer = 300 + tileY)
- `UIOverlay = 400` - Top-level UI elements

**Y-Sorting:** Objects with higher Y coordinates (further down) render later, creating depth perception for 3/4 perspective views.

### Debug Tools

**Tile Grid Visualization:**
- Press **F6** to toggle tile grid overlay (black semi-transparent lines)
- Grid automatically adjusts to camera zoom and position
- Enabled by default to help with object positioning

**Camera Controls:**
- **WASD / Arrow keys** - Move camera (600 pixels/sec)
- **Mouse wheel** - Zoom (0.2x to 1.0x, default 0.5x)
- **F6** - Toggle debug grid

## Adding New Code

### Adding a New Component

1. Define struct in appropriate `include/<module>/` header
2. Add to ECS registry usage in relevant system
3. Register in UI inspector (ImGui) if needed
4. Implement serialization for save/load

### Adding a New Module Source File

1. Create `.cpp` in `src/<module>/`
2. Create `.h/.hpp` in `include/<module>/`
3. Update `src/<module>/CMakeLists.txt`:
   ```cmake
   target_sources(ModuleName
       PRIVATE
           YourNewFile.cpp
   )
   ```

### Adding a New Industrial Object

1. Define component(s) for object state
2. Create simulation system
3. Add sprites to `assets/sprites/`
4. Configure OPC UA bindings
5. Document in Doxygen format

## Project Configuration Files

- **vcpkg.json** - Dependency manifest (17+ libraries)
- **CMakePresets.json** - Build presets for platforms
- **.clang-format** - LLVM-based code style (4 spaces, 100 col limit)
- **ROADMAP.md** - 10-phase development plan
- **TASK.md** - Complete tech stack documentation

## CI/CD Workflows

- `build.yml` - Multi-platform builds (Ubuntu, Windows, macOS)
- `test.yml` - Unit/integration tests + code coverage
- `release.yml` - Automated releases on version tags
- `code-quality.yml` - clang-format, clang-tidy, cppcheck

## Important Development Notes

### Code Style
- C++20/23 standard required
- Follow SOLID principles
- Document public APIs with Doxygen:
  ```cpp
  /**
   * @brief Brief description
   * @param paramName Description
   * @return Description
   * @throws ExceptionType When this happens
   */
  ```

### Asset Conventions
- Sprites: PNG format, 16x16 or 32x32, use atlases
- Maps: TMX format (Tiled), with metadata for physics
- Scripts: Lua files in `assets/scripts/`
- Configs: YAML in `assets/configs/`
- Projects: SQLite database schema (see docs/ARCHITECTURE.md)

### Database Schema (SQLite)
Projects stored with tables: `projects`, `scenes`, `bindings`
- Scenes contain serialized ECS entity data (BLOB)
- Bindings link entities to OPC UA NodeIds

### Performance Considerations
- Use EnTT views for cache-friendly iteration
- Implement frustum culling for tilemap rendering
- Minimize allocations in hot paths
- Use Tracy profiler (`ENABLE_TRACY=ON`) for analysis

## Development Phases

**Current: Phase 1** - Basic Engine & Rendering
- ✅ Milestone 1.1: Application Framework (SFML window, game loop, state management)
- ✅ Milestone 1.2: ECS Integration (EnTT, basic components and systems)
- ✅ Milestone 1.3: Tile System (tile-based positioning, layers, Y-sorting, overlays, animation)
- 🔄 Milestone 1.4: TMX Map Loading (in progress)

**Next: Phase 2** - Physics Integration (Box2D)

See ROADMAP.md for full 10-phase plan (12-15 month timeline).

## Key Dependencies

- **SFML 3** - Graphics, window, audio (⚠️ API breaking changes from SFML 2)
- **EnTT** - ECS framework
- **Box2D** - 2D physics
- **open62541** - OPC UA client/server (disabled on MinGW)
- **Lua + sol2** - Scripting
- **Dear ImGui + ImPlot** - UI and plotting
- **tmxlite** - TMX map loader (requires zstd)
- **zstd** - Compression library for tmxlite
- **SQLite3** - Project storage
- **spdlog** - Logging
- **fmt** - Formatting library
- **Catch2** - Testing framework

All dependencies managed via vcpkg manifest mode.
