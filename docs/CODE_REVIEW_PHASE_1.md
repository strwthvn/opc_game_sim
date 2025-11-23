# Код-ревью: Фаза 1 (Базовый движок) и Milestone 1.3 (Тайловая система)

**Дата:** 23.11.2024
**Версия:** 1.0
**Статус проекта:** Milestone 1.3 завершен

---

## 🔴 Критические проблемы

### 1. Отсутствие валидации в TilePositionSystem

**Файл:** `src/core/systems/TilePositionSystem.cpp:46-57`

```cpp
if (sprite.layer >= RenderLayer::Objects && sprite.layer < RenderLayer::Overlays) {
    sprite.layer = RenderLayer::Objects + tilePos.tileY;
}
```

**Проблема:**
- Y-координата не ограничена, что может привести к переполнению слоев
- При `tileY > 200` объекты попадут в слой Overlays (300+)
- Нет обработки отрицательных координат

**Решение:**
```cpp
int yOffset = std::clamp(tilePos.tileY, 0, 99);  // Ограничение 0-99
sprite.layer = RenderLayer::Objects + yOffset;
```

### 2. Гонка данных в OverlaySystem

**Файл:** `src/core/systems/OverlaySystem.cpp:47-54`

```cpp
static int logCount = 0;
if (logCount < 5) {
    LOG_DEBUG(...);
    logCount++;
}
```

**Проблема:**
- Статическая переменная `logCount` не потокобезопасна
- При будущей многопоточности возможна гонка данных
- Нарушает принцип "системы без состояния"

**Решение:**
- Убрать отладочное логирование из production кода
- Или использовать атомарный счетчик: `static std::atomic<int> logCount{0}`

### 3. Утечка памяти в TileMapSystem

**Файл:** `src/rendering/TileMapSystem.cpp:199-215`

```cpp
void TileMapSystem::renderTile(...) {
    sf::Sprite sprite(*tileset->texture, texRect);  // Создание нового спрайта каждый кадр
    target.draw(sprite);
}
```

**Проблема:**
- Создание `sf::Sprite` каждый кадр для каждого видимого тайла
- При карте 50x50 и видимой области 20x15 = 300 аллокаций за кадр @ 60 FPS = 18000 аллокаций/сек
- Полностью игнорируется `m_batchCache` и `m_vertices`

**Решение:**
Реализовать батчинг через VertexArray:
```cpp
void TileMapSystem::render(sf::RenderTarget& target, const sf::View& camera) {
    m_vertices.clear();
    // Добавить все видимые тайлы в VertexArray
    for (auto& tile : visibleTiles) {
        appendQuad(m_vertices, tile);
    }
    // Одна отрисовка вместо сотен
    target.draw(m_vertices, &tileset->texture);
}
```

---

## ⚠️ Проблемы производительности

### 4. Двойная сортировка в RenderSystem

**Файл:** `src/core/systems/RenderSystem.cpp:33-53`

```cpp
for (auto entity : view) {
    renderQueue.push_back({entity, &transform, &sprite, sprite.layer});
}
std::sort(renderQueue.begin(), renderQueue.end(), ...);
```

**Проблема:**
- Сортировка O(N log N) выполняется каждый кадр
- При 1000 объектов @ 60 FPS = дорогостоящая операция
- TilePositionSystem уже вычисляет layer с Y-sorting

**Решение:**
- Использовать `std::stable_sort` только при изменении объектов
- Добавить dirty flag для пересортировки:
```cpp
if (m_layersDirty) {
    std::stable_sort(...);
    m_layersDirty = false;
}
```

### 5. Избыточный markDirty в AnimationSystem

**Файл:** `src/core/systems/AnimationSystem.cpp:68-74`

```cpp
if (sprite.textureRect.position != frameRect.position ||
    sprite.textureRect.size != frameRect.size) {
    sprite.textureRect = frameRect;
    sprite.markDirty();  // Помечаем каждый раз при смене кадра
}
```

**Проблема:**
- При анимации 10 FPS каждый 6-й кадр (60 FPS / 10 FPS) помечается dirty
- Пересоздание кеша спрайта без реальной необходимости
- textureRect можно менять без пересоздания Sprite

**Решение:**
Удалить `markDirty()` из AnimationSystem - textureRect обновляется без пересоздания:
```cpp
sprite.textureRect = frameRect;
// Не нужно markDirty - RenderSystem применяет textureRect напрямую
```

### 6. Отсутствие Frustum Culling для объектов

**Файл:** `src/core/systems/RenderSystem.cpp:36-44`

**Проблема:**
- Рендерятся ВСЕ видимые объекты, даже за пределами камеры
- TileMapSystem имеет frustum culling, а объекты - нет
- При большой сцене (1000+ объектов) потеря производительности

**Решение:**
Добавить culling по bounds камеры:
```cpp
sf::FloatRect viewBounds = getViewBounds(window.getView());
if (!viewBounds.intersects(sprite.getGlobalBounds())) {
    continue;  // Пропускаем объекты вне камеры
}
```

---

## 🏗️ Архитектурные недостатки

### 7. SpriteComponent нарушает Single Responsibility

**Файл:** `include/core/Components.h:54-98`

```cpp
struct SpriteComponent {
    std::string textureName;
    sf::IntRect textureRect;
    sf::Color color;
    int layer;
    bool visible;
    mutable std::optional<sf::Sprite> cachedSprite;  // ❌ Кеш в компоненте
    mutable bool dirty;                              // ❌ Состояние системы
};
```

**Проблема:**
- Компонент содержит данные (textureName, layer) + кеш системы (cachedSprite, dirty)
- Нарушение ECS принципа "компоненты = чистые данные"
- `mutable` - признак проблемы с архитектурой
- Будущие проблемы с сериализацией (как сохранять cachedSprite?)

**Решение:**
Вынести кеш в RenderSystem:
```cpp
struct SpriteComponent {
    std::string textureName;
    sf::IntRect textureRect;
    sf::Color color;
    int layer;
    bool visible;
};

class RenderSystem {
    std::unordered_map<entt::entity, sf::Sprite> m_spriteCache;
};
```

### 8. Дублирование координат

**Проблема:**
- `TilePositionComponent` хранит (tileX, tileY)
- `TransformComponent` хранит (x, y) в пикселях
- TilePositionSystem синхронизирует их КАЖДЫЙ кадр
- Источник правды неясен: что первично - тайловые или пиксельные координаты?

**Решение:**
Выбрать один источник правды:
```cpp
// Вариант 1: Только пиксельные координаты + методы конвертации
struct TransformComponent {
    float x, y;
    sf::Vector2i getTilePosition() const { return {x/32, y/32}; }
};

// Вариант 2: Добавить флаг управления
struct TilePositionComponent {
    int tileX, tileY;
    bool autoSync = true;  // Автоматическая синхронизация с Transform
};
```

### 9. Жесткая связь систем

**Файл:** `src/core/states/GameState.cpp:162-182`

```cpp
if (m_updateSystem) m_updateSystem->update(...);
if (m_lifetimeSystem) m_lifetimeSystem->update(...);
if (m_tilePositionSystem) m_tilePositionSystem->update(...);
if (m_animationSystem) m_animationSystem->update(...);
if (m_overlaySystem) m_overlaySystem->update(...);
```

**Проблема:**
- Ручное управление порядком систем
- Добавление новой системы требует модификации GameState
- Нет зависимостей между системами (AnimationSystem должна идти ДО RenderSystem)

**Решение:**
Создать SystemScheduler:
```cpp
class SystemScheduler {
    std::vector<std::unique_ptr<ISystem>> systems;
public:
    void registerSystem(std::unique_ptr<ISystem> sys, std::vector<std::type_index> deps);
    void update(entt::registry& reg, float dt);  // Автоматический порядок
};
```

---

## 🔧 Расширяемость и API

### 10. Отсутствие интерфейсов систем ✅ ИСПРАВЛЕНО

**Проблема:**
Все системы имеют разные сигнатуры:
- `RenderSystem::render(registry, window)`
- `AnimationSystem::update(registry, dt)`
- `TilePositionSystem::update(registry)`
- `OverlaySystem::update(registry)`

**Решение:**
Унифицировать через базовый интерфейс:
```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(entt::registry& reg, double dt) = 0;
    virtual int getPriority() const { return 0; }  // Порядок выполнения
    virtual const char* getName() const = 0;
    virtual bool isActive() const { return true; }
    virtual void setActive(bool active) {}
};

class AnimationSystem : public ISystem {
    void update(entt::registry& reg, double dt) override;
    int getPriority() const override { return 300; }
    const char* getName() const override { return "AnimationSystem"; }
};
```

**Статус:** ✅ Исправлено
- Создан базовый интерфейс `ISystem` (include/core/systems/ISystem.h)
- Все системы обновления наследуются от `ISystem`:
  - `UpdateSystem` (приоритет 0)
  - `LifetimeSystem` (приоритет 50)
  - `TilePositionSystem` (приоритет 200)
  - `AnimationSystem` (приоритет 300)
  - `OverlaySystem` (приоритет 400)
  - `RenderSystem` (приоритет 500, с дополнительным методом `render()` и `setRenderTarget()`)
- Интерфейс поддерживает:
  - Унифицированный метод `update(registry, dt)`
  - Приоритеты для управления порядком выполнения
  - Имена систем для отладки
  - Возможность активации/деактивации систем

### 11. Глобальные константы в namespace

**Файл:** `include/core/Components.h:25-31`

```cpp
namespace RenderLayer {
    constexpr int Background = 0;
    constexpr int Ground = 100;
    constexpr int Objects = 200;
}
```

**Проблема:**
- Нельзя переопределить или настроить слои
- Хардкод в коде вместо конфигурации
- При добавлении слоев придется перекомпилировать

**Решение:**
Использовать enum class + конфигурацию:
```cpp
enum class RenderLayer : int {
    Background = 0,
    Ground = 100,
    Objects = 200,
    Overlays = 300,
    UIOverlay = 400
};

// В будущем: загрузка из YAML
class LayerConfig {
    std::unordered_map<std::string, int> layers;
    void loadFromFile(const std::string& path);
};
```

### 12. Отсутствие событийной системы

**Проблема:**
- Нет механизма реакции на изменения компонентов
- Пример: при изменении TilePositionComponent нужно вручную обновлять TransformComponent
- Невозможно подписаться на события: "объект уничтожен", "анимация завершена"

**Решение:**
Использовать EnTT observers:
```cpp
class TilePositionSystem {
    entt::observer m_observer;
public:
    TilePositionSystem(entt::registry& reg)
        : m_observer(reg, entt::collector.update<TilePositionComponent>()) {}

    void update(entt::registry& reg) {
        // Обновляем только измененные объекты
        for (auto entity : m_observer) {
            syncPosition(reg, entity);
        }
        m_observer.clear();
    }
};
```

---

## 📊 Качество кода и практики

### 13. Магические числа

**Примеры:**
- `GameState.cpp:46` - `192.0f, 128.0f` (центр камеры)
- `GameState.cpp:578` - `sf::Color(0, 0, 0, 128)` (цвет сетки)
- `GameState.cpp:129` - `600.0f` (CAMERA_MOVE_SPEED захардкожен?)

**Решение:**
Вынести в константы или конфигурацию:
```cpp
namespace GameStateConfig {
    constexpr float INITIAL_CAMERA_X = 192.0f;
    constexpr float INITIAL_CAMERA_Y = 128.0f;
    constexpr sf::Color DEBUG_GRID_COLOR(0, 0, 0, 128);
    constexpr float CAMERA_MOVE_SPEED = 600.0f;
}
```

### 14. Отсутствие документации API

**Проблема:**
- Не описано поведение систем при отсутствии компонентов
- Нет гарантий порядка выполнения
- Неясно, нужно ли вручную вызывать TilePositionSystem перед RenderSystem

**Решение:**
Добавить документацию в headers:
```cpp
/**
 * @brief Синхронизирует тайловые координаты с пиксельными
 *
 * @warning Должна вызываться ДО RenderSystem в игровом цикле
 * @note Требует наличия TilePositionComponent И TransformComponent
 * @performance O(N) где N - количество тайловых объектов
 */
void update(entt::registry& registry);
```

### 15. Отсутствие юнит-тестов

**Проблема:**
- Milestone 1.3 завершен БЕЗ покрытия тестами
- Невозможно проверить корректность TilePositionComponent::getPixelPosition()
- Ручное тестирование систем через визуализацию

**Решение:**
Добавить тесты с Catch2:
```cpp
TEST_CASE("TilePositionComponent conversions", "[tile]") {
    TilePositionComponent tile{3, 6, 1, 1};

    SECTION("getPixelPosition returns bottom-left corner") {
        auto pos = tile.getPixelPosition();
        REQUIRE(pos.x == 96.0f);   // 3 * 32
        REQUIRE(pos.y == 224.0f);  // (6 + 1) * 32
    }
}
```

---

## 💡 Предложения по улучшению

### 16. Spatial Hashing для коллизий

**Обоснование:**
- Сейчас все объекты в одном списке
- При добавлении физики (Phase 2) потребуется пространственное разбиение

**Реализация:**
```cpp
class SpatialGrid {
    std::unordered_map<int, std::vector<entt::entity>> cells;
    int cellSize = 64;  // 2x2 тайла
public:
    void insert(entt::entity e, const TransformComponent& t);
    std::vector<entt::entity> query(const sf::FloatRect& bounds);
};
```

### 17. Resource Handles вместо строк

**Проблема:**
- `SpriteComponent::textureName` - поиск по строке в map каждый кадр
- Строковые сравнения дороги

**Решение:**
```cpp
using TextureHandle = uint32_t;

class ResourceManager {
    std::vector<sf::Texture> m_textures;
public:
    TextureHandle loadTexture(const std::string& name);
    const sf::Texture& get(TextureHandle handle);
};

struct SpriteComponent {
    TextureHandle textureHandle;  // Вместо string
};
```

### 18. Component Pools для оптимизации

**Обоснование:**
- EnTT уже использует пулы, но можно оптимизировать дальше
- Часто создаваемые/уничтожаемые компоненты (частицы, пули)

**Решение:**
```cpp
template<typename T>
class ComponentPool {
    std::vector<T> pool;
    std::stack<size_t> freeIndices;
public:
    T& acquire() { /* ... */ }
    void release(T& component) { /* ... */ }
};
```

### 19. Prefab система

**Обоснование:**
- `GameState::createTileTestScene()` дублирует код создания объектов
- Нет переиспользования конфигураций

**Решение:**
```cpp
class PrefabManager {
    std::unordered_map<std::string, PrefabData> prefabs;
public:
    void loadPrefab(const std::string& name, const std::string& path);
    entt::entity instantiate(entt::registry& reg, const std::string& name);
};

// assets/prefabs/pump.yaml
// components:
//   - TilePosition: {x: 0, y: 0, w: 1, h: 1}
//   - Sprite: {texture: "pump.png", layer: Objects}
```

### 20. Command Pattern для undo/redo

**Обоснование:**
- Редактор уровней (Phase 6) потребует отмену действий
- Сейчас прямые изменения ECS без истории

**Решение:**
```cpp
class ICommand {
public:
    virtual void execute(entt::registry& reg) = 0;
    virtual void undo(entt::registry& reg) = 0;
};

class MoveEntityCommand : public ICommand {
    entt::entity entity;
    sf::Vector2f oldPos, newPos;
    // ...
};
```

---

## 🎯 Приоритизация исправлений

### Немедленно (критично):
1. ✅ **Проблема 3**: Батчинг в TileMapSystem (утечка памяти)
2. ✅ **Проблема 1**: Валидация в TilePositionSystem (переполнение слоев)
3. ✅ **Проблема 6**: Frustum culling для объектов

### До Phase 2 (важно):
4. ✅ **Проблема 7**: Вынести кеш из SpriteComponent (ИСПРАВЛЕНО)
5. ✅ **Проблема 8**: Дублирование координат (ИСПРАВЛЕНО: добавлен autoSync)
6. ✅ **Проблема 9**: Создать SystemScheduler (ИСПРАВЛЕНО)
7. **Проблема 12**: Добавить событийную систему (EnTT observers)

### До релиза (желательно):
7. ✅ **Проблема 15**: Добавить юнит-тесты
8. ✅ **Проблема 17**: Resource handles вместо строк
9. ✅ **Проблема 19**: Prefab система

### Опционально (оптимизация):
10. **Проблема 4**: Оптимизация сортировки
11. **Проблема 16**: Spatial hashing
12. **Проблема 18**: Component pools

---

## 📈 Метрики качества кода

| Метрика | Текущее | Целевое | Статус |
|---------|---------|---------|--------|
| Покрытие тестами | 0% | 70% | 🔴 |
| Цикломатическая сложность | ~8-12 | <10 | 🟡 |
| Дублирование кода | ~5% | <3% | 🟡 |
| Документация API | 40% | 80% | 🟡 |
| Memory leaks (Valgrind) | Не проверено | 0 | 🔴 |
| FPS @ 1000 objects | Не замерено | >60 | ❓ |

---

## ✅ Что сделано хорошо

1. **Четкое разделение ECS** - компоненты и системы правильно разделены
2. **Bottom-left origin** - интуитивная система координат для 2D
3. **Y-sorting** - корректная перспектива 3/4
4. **Frustum culling в TileMapSystem** - есть базовая оптимизация
5. **ResourceManager** - централизованное управление ресурсами
6. **Logging** - хорошее использование spdlog для отладки
7. **SFML 3 API** - правильное использование новых паттернов

---

## 🔮 Рекомендации для Phase 2

### Перед началом Box2D интеграции:

1. **Исправить проблемы 1-3** (критичные)
2. **Добавить SystemScheduler** - для правильного порядка Physics → Transform sync
3. **Внедрить профилирование** - Tracy profiler для анализа узких мест
4. **Создать тестовую сцену** - с 1000+ объектами для стресс-теста

### Архитектурные решения:

```cpp
// Синхронизация Box2D ↔ ECS
class PhysicsSystem {
    void update(entt::registry& reg, float dt) {
        // 1. Box2D step
        world->Step(dt, 8, 3);

        // 2. Sync Box2D → TransformComponent
        auto view = reg.view<RigidBodyComponent, TransformComponent>();
        for (auto e : view) {
            auto& rb = view.get<RigidBodyComponent>(e);
            auto& tr = view.get<TransformComponent>(e);
            auto pos = rb.body->GetPosition();
            tr.x = pos.x * PIXELS_PER_METER;
            tr.y = pos.y * PIXELS_PER_METER;
        }
    }
};
```

---

*Документ создан: 23.11.2024*
*Автор: Claude Code Review*
*Версия проекта: Phase 1, Milestone 1.3*
