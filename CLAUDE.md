# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OPC Game Simulator is a 2D industrial automation simulator for PLC programming education with OPC UA integration. Built with C++20/23 using ECS architecture (EnTT), combining game sandbox mechanics with realistic industrial process simulation.

**Current Status:** Phase 0 (Infrastructure Setup) - preparing for Phase 1 (Basic Engine & Rendering)

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
- `TransformComponent` - position, rotation, scale
- `SpriteComponent` - visual representation
- `SensorComponent` - industrial sensors (presence, level, temperature)
- `ActuatorComponent` - motors, valves, indicators
- `PLCBindingComponent` - OPC UA variable bindings
- `RigidBodyComponent` - Box2D physics body reference

**Systems contain logic** (no data):
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

**Current: Phase 0** - Infrastructure complete
**Next: Phase 1** - Basic engine (SFML window, game loop, ECS, tilemap rendering)

See ROADMAP.md for full 10-phase plan (12-15 month timeline).

## Key Dependencies

- **SFML 3** - Graphics, window, audio
- **EnTT** - ECS framework
- **Box2D** - 2D physics
- **open62541** - OPC UA client/server
- **Lua + sol2** - Scripting
- **Dear ImGui + ImPlot** - UI and plotting
- **tmxlite** - TMX map loader
- **SQLite3** - Project storage
- **spdlog** - Logging
- **Catch2** - Testing framework

All dependencies managed via vcpkg manifest mode.
