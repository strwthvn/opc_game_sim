# Архитектура проекта

## Общий обзор

OPC Game Simulator построен на модульной архитектуре с использованием паттерна ECS (Entity Component System). Проект разделен на независимые модули, каждый из которых отвечает за определенную функциональность.

**Текущий статус:** Фаза 1 (Базовый движок и рендеринг) - Milestone 1.3 (Тайловая система) завершен

## Модули системы

### Core (Ядро) ✅ РЕАЛИЗОВАНО
Базовая функциональность приложения:
- ✅ Управление окном и контекстом (Window class)
- ✅ Игровой цикл с фиксированным timestep ("Fix Your Timestep" паттерн, 60 UPS)
- ✅ Система состояний (State Machine) - Stack-based StateManager
- ✅ Менеджер ресурсов (ResourceManager - текстуры и шрифты)
- ✅ Система логирования (spdlog с ротацией файлов)
- ✅ Система ввода (InputManager)
- ✅ Метрики производительности (PerformanceMetrics)
- ✅ ECS архитектура (EnTT с 12 компонентами, 7 системами)

**Реализованные состояния:**
- MenuState - главное меню
- GameState - основной игровой процесс
- PauseState - пауза с оверлеем

### Rendering (Рендеринг) ✅ РЕАЛИЗОВАНО
Визуализация:
- ✅ Система рендеринга на базе SFML 3
- ✅ Управление камерой (zoom 0.2x-1.0x, pan WASD/стрелки)
- ✅ Рендеринг тайловых карт TMX (tmxlite с frustum culling)
- ✅ Многослойная отрисовка (5 слоев: Background, Ground, Objects, Overlays, UIOverlay)
- ✅ Y-sorting для 3/4 перспективы
- ✅ Кэширование спрайтов для оптимизации
- ✅ Отладочная сетка тайлов (F6)
- ✅ Dual-view система (мировой вид + UI вид)

### Simulation (Симуляция) 🔄 СТРУКТУРА ПОДГОТОВЛЕНА
Физика и игровая логика:
- 🔄 Интеграция Box2D для физики (запланировано в Фазе 2)
- 🔄 Симуляция промышленных процессов
- 🔄 Управление временем симуляции
- 🔄 Обработка коллизий

**Статус:** Структура модуля создана, реализация начнется в Фазе 2

### Industrial (Промышленные протоколы) 🔄 СТРУКТУРА ПОДГОТОВЛЕНА
OPC UA и другие промышленные протоколы:
- 🔄 OPC UA клиент (open62541)
- 🔄 Система привязок переменных
- 🔄 Обработка подписок
- 🔄 Масштабирование и преобразование данных
- 🔄 Modbus TCP/RTU (опционально)

**Статус:** Структура модуля создана, реализация запланирована для будущих фаз

### Scripting (Скриптинг) 🔄 СТРУКТУРА ПОДГОТОВЛЕНА
Lua интеграция:
- 🔄 Движок скриптов
- 🔄 API для работы с ECS
- 🔄 Загрузка и выполнение скриптов
- 🔄 Обработка событий

**Статус:** Структура модуля создана, реализация запланирована для будущих фаз

### Editor (Редактор) 🔄 СТРУКТУРА ПОДГОТОВЛЕНА
Инструменты редактирования:
- 🔄 Редактор уровней
- 🔄 Размещение объектов
- 🔄 Система сохранения/загрузки
- 🔄 Управление проектами (SQLite)

**Статус:** Структура модуля создана, реализация запланирована для будущих фаз

### UI (Пользовательский интерфейс) 🔄 СТРУКТУРА ПОДГОТОВЛЕНА
ImGui интерфейс:
- 🔄 Инспектор объектов
- 🔄 Панель иерархии
- 🔄 Браузер OPC UA
- 🔄 Графики реального времени (ImPlot)
- 🔄 Консоль логов

**Статус:** Структура модуля создана, реализация запланирована для будущих фаз

## ECS архитектура

### Компоненты (Components)

**Расположение:** `include/core/Components.h`

Компоненты - это данные без логики. Базовые компоненты (12 в Core) реализованы, дополнительные физические компоненты (2 в Simulation) подготовлены для Фазы 2:

#### Базовые компоненты

```cpp
// Позиция, поворот, масштаб в пиксельных координатах
struct TransformComponent {
    float x, y;           // Координаты в пикселях
    float rotation;       // Угол поворота в градусах
    float scale;          // Масштаб
};

// Визуальное представление
struct SpriteComponent {
    std::string textureName;    // Имя текстуры в ResourceManager
    sf::IntRect textureRect;    // Область текстуры для спрайта
    sf::Color color;            // Цвет модуляции
    int layer;                  // Слой для Z-ordering
    bool visible;               // Флаг видимости
};

// Линейная и угловая скорость
struct VelocityComponent {
    float vx, vy;               // Линейная скорость
    float angularVelocity;      // Угловая скорость
};

// Камера (одна активная на сцену)
struct CameraComponent {
    float zoom;                 // Уровень зума
    bool active;                // Флаг активности
};

// Читаемое имя для отладки
struct NameComponent {
    std::string name;
};

// Теги для категоризации
struct TagComponent {
    std::string tag;
};
```

#### Компоненты времени жизни и анимации

```cpp
// Автоматическое уничтожение по таймеру
struct LifetimeComponent {
    float lifetime;             // Оставшееся время жизни
    bool autoDestroy;           // Уничтожить по истечении
    bool fadeOut;               // Постепенное исчезновение
};

// Покадровая анимация спрайтов
struct AnimationComponent {
    std::string currentAnimation;    // Имя текущей анимации
    int currentFrame;                // Текущий кадр
    int frameCount;                  // Количество кадров
    float frameTime;                 // Время одного кадра
    float elapsedTime;               // Прошедшее время
    bool loop;                       // Зациклить анимацию
    int frameWidth;                  // Ширина кадра
    int frameHeight;                 // Высота кадра
};
```

#### Компоненты иерархии и позиционирования

```cpp
// Родительская связь
struct ParentComponent {
    entt::entity parent;        // ID родительской сущности
};

// Дочерние связи
struct ChildrenComponent {
    std::set<entt::entity> children;  // ID дочерних сущностей
};

// Тайловая позиция (система координат 32x32)
struct TilePositionComponent {
    int tileX, tileY;           // Координаты в тайлах
    int widthTiles;             // Ширина в тайлах
    int heightTiles;            // Высота в тайлах
    bool autoSync;              // Автосинхронизация с Transform

    // Методы конвертации
    sf::Vector2f getPixelPosition() const;
    sf::Vector2f getCenterPixel() const;
    void setFromPixelPosition(float px, float py);
    bool containsTile(int x, int y) const;
};

// Оверлей привязанный к родителю
struct OverlayComponent {
    float localOffsetX;         // Локальное смещение X
    float localOffsetY;         // Локальное смещение Y
};
```

#### Компоненты физики

**Расположение:** `include/simulation/PhysicsComponents.h`

Компоненты для интеграции с Box2D физическим движком (подготовлены для Фазы 2):

```cpp
// Форма и свойства коллайдера для Box2D
struct ColliderComponent {
    enum class Shape { Box, Circle, Polygon };

    Shape shape;                        // Тип формы коллайдера
    sf::Vector2f size;                  // Размер для прямоугольника
    float radius;                       // Радиус для круга
    std::vector<sf::Vector2f> vertices; // Вершины для полигона
    sf::Vector2f offset;                // Смещение от Transform

    float density;                      // Плотность материала
    float friction;                     // Коэффициент трения
    float restitution;                  // Упругость (отскок)
    bool isSensor;                      // Триггер без физики

    // Утилиты для тайловой системы
    void setFromTileSize(int w, int h, int tileSize = 32);
    static ColliderComponent createBox(int w, int h, int tileSize = 32);
};

// Свойства твёрдого тела для Box2D
struct RigidbodyComponent {
    enum class BodyType { Static, Kinematic, Dynamic };

    BodyType bodyType;                  // Тип тела
    float mass;                         // Масса (кг)
    float linearDamping;                // Затухание линейной скорости
    float angularDamping;               // Затухание угловой скорости
    float gravityScale;                 // Множитель гравитации

    bool fixedRotation;                 // Запретить вращение
    bool allowSleep;                    // Разрешить "засыпание"
    bool isBullet;                      // Непрерывная детекция коллизий

    sf::Vector2f linearVelocity;        // Начальная линейная скорость
    float angularVelocity;              // Начальная угловая скорость

    void* box2dBody;                    // Указатель на b2Body

    // Утилиты для создания
    static RigidbodyComponent createStatic();
    static RigidbodyComponent createKinematic();
    static RigidbodyComponent createDynamic(float mass = 1.0f);
};
```

**Примечание:** В модуле Core также есть `CollisionComponent` для простых тайловых коллизий (AABB проверки). Используйте его для базовых проверок столкновений до интеграции Box2D.

#### Константы системы

```cpp
constexpr int TILE_SIZE = 32;  // Размер тайла в пикселях

// Слои рендеринга
enum RenderLayer {
    Background = 0,
    Ground = 100,
    Objects = 200,     // + tileY для Y-sorting
    Overlays = 300,    // + tileY для Y-sorting
    UIOverlay = 400
};
```

### Системы (Systems)

**Расположение:** `include/core/systems/`, `src/core/systems/`

Системы - это логика без данных. Все системы реализуют интерфейс `ISystem`:

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(entt::registry& registry, float deltaTime) = 0;
    virtual int getPriority() const = 0;
};
```

#### 1. UpdateSystem (Приоритет: 0)

Обновление позиций на основе скорости:

```cpp
class UpdateSystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 0; }

private:
    void updateMovement(entt::registry& registry, float deltaTime);
};
```

#### 2. LifetimeSystem (Приоритет: 50)

Управление временем жизни сущностей:

```cpp
class LifetimeSystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 50; }

private:
    void processEntity(entt::registry& registry, entt::entity entity,
                      LifetimeComponent& lifetime, float deltaTime);
};
```

#### 3. TilePositionSystem (Приоритет: 200)

Синхронизация тайловых координат с пиксельными:

```cpp
class TilePositionSystem : public ISystem {
public:
    explicit TilePositionSystem(entt::registry& registry);
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 200; }

private:
    void syncPositions(entt::registry& registry);
    void updateLayers(entt::registry& registry);

    // EnTT observers для реактивного обновления
    entt::observer m_tileObserver;
    entt::observer m_transformObserver;
};
```

**Особенности:**
- Использует EnTT observers для эффективного отслеживания изменений
- Автоматически обновляет только измененные сущности
- Конвертирует тайловые координаты: `x = tileX * 32`, `y = (tileY + height) * 32`
- Обновляет слой для Y-sorting: `layer = Objects + tileY`

#### 4. AnimationSystem (Приоритет: 300)

Покадровая анимация спрайтов:

```cpp
class AnimationSystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 300; }

private:
    void updateAnimationState(entt::registry& registry, float deltaTime);
    void updateTextureRects(entt::registry& registry);
};
```

#### 5. OverlaySystem (Приоритет: 400)

Синхронизация оверлеев с родительскими объектами:

```cpp
class OverlaySystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 400; }

private:
    void syncOverlayPositions(entt::registry& registry);
};
```

#### 6. RenderSystem (Приоритет: 500)

Рендеринг всех видимых спрайтов:

```cpp
class RenderSystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    void render(entt::registry& registry);
    void setRenderTarget(sf::RenderTarget* target);
    void invalidateCache(entt::entity entity);
    void clearCache();
    void markLayersDirty();
    int getPriority() const override { return 500; }

private:
    sf::RenderTarget* m_renderTarget;
    std::unordered_map<entt::entity, sf::Sprite> m_spriteCache;
    bool m_layersDirty;
};
```

**Особенности:**
- Кэширует `sf::Sprite` объекты для оптимизации
- Поддерживает многослойную отрисовку с Z-ordering
- Y-sorting для 3/4 перспективы (сортировка по Y-координате)
- Устанавливает origin спрайта в нижний левый угол для не вращающихся объектов
- Устанавливает origin в центр для вращающихся объектов

#### 7. TileMapSystem (Модуль Rendering)

Загрузка и рендеринг TMX карт:

```cpp
class TileMapSystem {
public:
    bool loadMap(const std::string& mapPath);
    void unloadMap();
    void render(sf::RenderTarget& target, const sf::View& view);
    int getMapWidth() const;
    int getMapHeight() const;
    bool isLoaded() const;

private:
    struct Tileset { /* ... */ };
    struct TileLayer { /* ... */ };

    std::vector<Tileset> m_tilesets;
    std::vector<TileLayer> m_layers;
    bool m_loaded;
};
```

**Особенности:**
- Использует библиотеку tmxlite для парсинга
- Реализует frustum culling для оптимизации
- Поддерживает множественные слои тайлов
- Батчинг для эффективной отрисовки

#### SystemScheduler

Управление порядком выполнения систем:

```cpp
class SystemScheduler {
public:
    void addSystem(std::shared_ptr<ISystem> system);
    void update(entt::registry& registry, float deltaTime);
    template<typename T>
    std::shared_ptr<T> getSystem();
    void clear();

private:
    std::vector<std::shared_ptr<ISystem>> m_systems;
    bool m_sorted;
};
```

**Порядок выполнения систем (по приоритету):**
1. UpdateSystem (0) - обновление позиций
2. LifetimeSystem (50) - управление временем жизни
3. TilePositionSystem (200) - синхронизация координат
4. AnimationSystem (300) - обновление анимации
5. OverlaySystem (400) - синхронизация оверлеев
6. RenderSystem (500) - финальная отрисовка

## Потоки выполнения

**Текущий статус:** Однопоточная архитектура (Phase 1). Многопоточность запланирована для будущих фаз.

### Главный поток (реализовано)
- ✅ Обработка ввода (InputManager)
- ✅ Обновление систем ECS (SystemScheduler)
- ✅ Рендеринг (RenderSystem, TileMapSystem)
- ✅ Управление состояниями (StateManager)
- ✅ Игровой цикл с фиксированным timestep

### Планируемые потоки (будущие фазы)

#### Поток физики
- 🔄 Симуляция Box2D
- 🔄 Обновление состояний объектов
- 🔄 Обработка коллизий

#### Поток OPC UA
- 🔄 Коммуникация с ПЛК
- 🔄 Обработка подписок
- 🔄 Чтение/запись переменных

#### Поток скриптов (опционально)
- 🔄 Выполнение Lua скриптов
- 🔄 Обработка событий

**Примечание:** Для межпоточной коммуникации планируется использование `boost::signals2`.

## Система событий

**Текущий статус:** Планируется для будущих фаз

Будет использоваться `boost::signals2` для развязки модулей:

```cpp
// Определение сигнала
boost::signals2::signal<void(float)> onSensorValueChanged;

// Подписка
onSensorValueChanged.connect([](float value) {
    LOG_INFO("Sensor value: {}", value);
});

// Генерация события
onSensorValueChanged(25.5f);
```

## Управление ресурсами

### ResourceManager ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/ResourceManager.h`, `src/core/ResourceManager.cpp`

Централизованное управление текстурами и шрифтами:

```cpp
class ResourceManager {
public:
    // Получение ресурсов (автозагрузка при отсутствии)
    const sf::Font& getFont(const std::string& name);
    const sf::Texture& getTexture(const std::string& name);

    // Явная загрузка
    bool loadFont(const std::string& name, const std::string& path);
    bool loadTexture(const std::string& name, const std::string& path);

    // Пакетная загрузка с прогресс-коллбеком
    void preloadTextures(const std::vector<std::string>& paths,
                        std::function<void(size_t, size_t)> progressCallback);
    void preloadFonts(const std::vector<std::pair<std::string, std::string>>& configs,
                     std::function<void(size_t, size_t)> progressCallback);

    // Проверка наличия
    bool hasFont(const std::string& name) const;
    bool hasTexture(const std::string& name) const;

    // Выгрузка
    void unloadFont(const std::string& name);
    void unloadTexture(const std::string& name);
    void clear();

    // Статистика
    size_t getFontCount() const;
    size_t getTextureCount() const;

private:
    std::unordered_map<std::string, sf::Font> m_fonts;
    std::unordered_map<std::string, sf::Texture> m_textures;
};
```

**Особенности:**
- Ленивая загрузка с автоматическим fallback на системные шрифты
- Кэширование всех загруженных ресурсов
- Поддержка предзагрузки с отслеживанием прогресса

### TileMapSystem ✅ РЕАЛИЗОВАНО

**Расположение:** `include/rendering/TileMapSystem.h`

Управление тайловыми картами:

```cpp
class TileMapSystem {
public:
    bool loadMap(const std::string& mapPath);
    void unloadMap();
    void render(sf::RenderTarget& target, const sf::View& view);
    int getMapWidth() const;
    int getMapHeight() const;
    bool isLoaded() const;
};
```

**Особенности:**
- Использует tmxlite для парсинга TMX файлов
- Frustum culling для оптимизации (рендерит только видимые тайлы)
- Поддержка множественных тайлсетов и слоев

## Система сохранения

**Текущий статус:** Планируется для будущих фаз (Editor модуль)

### SQLite схема (планируется)
```sql
CREATE TABLE projects (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    created_at DATETIME,
    modified_at DATETIME
);

CREATE TABLE scenes (
    id INTEGER PRIMARY KEY,
    project_id INTEGER,
    name TEXT,
    data BLOB,  -- Сериализованная сцена
    FOREIGN KEY (project_id) REFERENCES projects(id)
);

CREATE TABLE bindings (
    id INTEGER PRIMARY KEY,
    scene_id INTEGER,
    entity_id INTEGER,
    node_id TEXT,
    binding_type TEXT,
    FOREIGN KEY (scene_id) REFERENCES scenes(id)
);
```

## Интеграция OPC UA

**Текущий статус:** Планируется для будущих фаз (Industrial модуль)

### Архитектура клиента (планируется)

```cpp
class OPCUAClient {
    UA_Client* client;
    std::thread workerThread;

public:
    void connect(const std::string& endpoint);
    void subscribe(const std::string& nodeId, VariableCallback callback);
    void writeValue(const std::string& nodeId, const UA_Variant& value);
};
```

### Система привязок (планируется)

```cpp
class BindingSystem : public ISystem {
    OPCUAClient& client;

public:
    void update(entt::registry& registry, float deltaTime) override {
        auto view = registry.view<PLCBindingComponent>();
        for (auto entity : view) {
            auto& binding = view.get<PLCBindingComponent>(entity);
            // Синхронизация с OPC UA
        }
    }
};
```

### Компоненты для промышленной автоматизации (планируются)

```cpp
struct SensorComponent {
    SensorType type;
    float value;
    float minValue, maxValue;
};

struct ActuatorComponent {
    ActuatorType type;  // Motor, Valve, Indicator
    float targetValue;
    float currentValue;
};

struct PLCBindingComponent {
    std::string nodeId;
    BindingType type;  // Read, Write, ReadWrite
    float scaleFactor;
    float offset;
};
```

## Производительность

### Оптимизация рендеринга ✅ РЕАЛИЗОВАНО
- ✅ Frustum culling для тайлов (TileMapSystem)
- ✅ Кэширование sf::Sprite объектов (RenderSystem)
- ✅ Батчинг спрайтов для тайловых карт
- ✅ Y-sorting оптимизирован (сортировка только при изменении слоев)

### Оптимизация ECS ✅ РЕАЛИЗОВАНО
- ✅ Cache-friendly итерация через EnTT views
- ✅ Минимизация аллокаций
- ✅ EnTT observers для реактивного обновления (TilePositionSystem)
- ✅ Использование view вместо итерации по всем сущностям

### PerformanceMetrics ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/PerformanceMetrics.h`, `src/core/PerformanceMetrics.cpp`

```cpp
class PerformanceMetrics {
public:
    void recordFrame(float deltaTime);
    void recordUpdate(float deltaTime);

    float getFPS() const;
    float getUPS() const;
    float getAverageFrameTime() const;
    float getMinFPS() const;
    float getMaxFPS() const;

    void reset();
};
```

**Метрики:**
- FPS (Frames Per Second)
- UPS (Updates Per Second)
- Среднее время кадра
- Мин/макс FPS
- История последних 60 семплов

### Профилирование
Tracy profiler доступен через CMake опцию `ENABLE_TRACY=ON`:
- Время кадра
- Горячие точки CPU
- Аллокации памяти

**Примечание:** В текущей сборке Tracy отключен по умолчанию

## Расширяемость

### Добавление нового компонента

1. Добавьте структуру в `include/core/Components.h`:
```cpp
struct MyComponent {
    float value;
    std::string data;
};
```

2. Создайте систему в `include/core/systems/MySystem.h`:
```cpp
class MySystem : public ISystem {
public:
    void update(entt::registry& registry, float deltaTime) override;
    int getPriority() const override { return 150; }  // Выберите приоритет
};
```

3. Реализуйте логику в `src/core/systems/MySystem.cpp`

4. Зарегистрируйте систему в GameState:
```cpp
m_systemScheduler.addSystem(std::make_shared<MySystem>());
```

5. (Опционально) Добавьте инспектор в ImGui (будущие фазы)
6. (Опционально) Добавьте сериализацию (будущие фазы)

### Добавление нового промышленного объекта (будущие фазы)

1. Создайте компоненты для объекта (например, TankComponent, PumpComponent)
2. Реализуйте систему симуляции (наследуйте ISystem)
3. Добавьте спрайты в `assets/sprites/`
4. Настройте привязки OPC UA через PLCBindingComponent

## Диаграммы

### Поток данных (текущая реализация)
```
User Input → InputManager → StateManager → GameState →
SystemScheduler → [UpdateSystem → LifetimeSystem → TilePositionSystem →
AnimationSystem → OverlaySystem → RenderSystem] → Window → Display
```

### Поток данных OPC UA (будущая реализация)
```
ПЛК → OPC UA Server → OPC UA Client → BindingSystem →
PLCBindingComponent → ECS Components → Simulation/Rendering
```

### Игровой цикл (текущая реализация)
```
┌─────────────────────────────────────────────────────────────┐
│ Application::run() - Main Loop                              │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ 1. Poll Events (SFML)                                      │
│    └─> Process window events, input                        │
│                                                             │
│ 2. Accumulate Frame Time                                   │
│    └─> Cap at 0.25s to prevent spiral of death            │
│                                                             │
│ 3. Fixed Update Loop (60 UPS)                              │
│    while (accumulator >= FIXED_TIMESTEP) {                 │
│        ├─> InputManager::update()                          │
│        ├─> StateManager::update(FIXED_TIMESTEP)            │
│        │   └─> GameState::update()                         │
│        │       └─> SystemScheduler::update()               │
│        │           ├─> UpdateSystem (0)                    │
│        │           ├─> LifetimeSystem (50)                 │
│        │           ├─> TilePositionSystem (200)            │
│        │           ├─> AnimationSystem (300)               │
│        │           ├─> OverlaySystem (400)                 │
│        │           └─> RenderSystem (500)                  │
│        └─> PerformanceMetrics::recordUpdate()              │
│    }                                                        │
│                                                             │
│ 4. Render Phase                                            │
│    ├─> Window::clear()                                     │
│    ├─> StateManager::render()                              │
│    │   └─> GameState::render()                             │
│    │       ├─> TileMapSystem::render()                     │
│    │       ├─> RenderSystem::render()                      │
│    │       └─> Debug grid (if enabled)                     │
│    └─> Window::display()                                   │
│                                                             │
│ 5. Performance Logging (every 5 seconds)                   │
│    └─> Log FPS, UPS, frame time                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Система координат и рендеринга

```
┌─────────────────────────────────────────────────────────────┐
│ Тайловая сетка (32x32 пикселя)                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ TilePositionComponent (tileX=3, tileY=6, 1x1)              │
│           ↓                                                 │
│ TilePositionSystem::syncPositions()                         │
│           ↓                                                 │
│ TransformComponent (x=96, y=224)                            │
│           ↓                                                 │
│ SpriteComponent (layer=206)  ← Objects(200) + tileY(6)     │
│           ↓                                                 │
│ RenderSystem::render()                                      │
│   ├─> Сортировка по layer + Y-coordinate                   │
│   ├─> Установка origin в нижний левый угол                 │
│   └─> Отрисовка sf::Sprite                                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘

Origin спрайта:
  ┌────────┐
  │        │  ← Top
  │ Sprite │
  │        │
  └────────┘
  ↑ Bottom-left (0, height) - origin для статичных объектов

  Центр (width/2, height/2) - origin для вращающихся объектов
```

## Управление состояниями приложения

### StateManager ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/StateManager.h`, `src/core/StateManager.cpp`

Stack-based система управления состояниями:

```cpp
class StateManager {
public:
    void pushState(std::unique_ptr<State> state);
    void popState();
    void changeState(std::unique_ptr<State> state);
    void clearStates();

    void update(float deltaTime);
    void render();
    void handleEvent(const sf::Event& event);
    void onWindowResize(const sf::Vector2u& size);

    State* getCurrentState();
    bool hasStates() const;

private:
    std::vector<std::unique_ptr<State>> m_states;

    enum class Action { Push, Pop, Change, Clear };
    std::vector<PendingChange> m_pendingChanges;

    void applyPendingChanges();
};
```

### State (Базовый класс) ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/State.h`, `src/core/State.cpp`

```cpp
class State {
public:
    virtual ~State() = default;

    // Обязательные методы
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual std::string getName() const = 0;

    // Опциональные методы
    virtual bool updateBelow() const { return false; }
    virtual bool renderBelow() const { return false; }
    virtual void onWindowResize(const sf::Vector2u& size) {}

protected:
    StateManager* m_stateManager;
    InputManager* m_inputManager;
    ResourceManager* m_resourceManager;
    Window* m_window;
};
```

### Реализованные состояния

#### 1. MenuState ✅

**Расположение:** `include/core/states/MenuState.h`, `src/core/states/MenuState.cpp`

- Главное меню с пунктами: New Game, Continue, Settings, Exit
- Навигация клавишами вверх/вниз, Enter для выбора
- Фиксированный UI view (1280x720)

#### 2. GameState ✅

**Расположение:** `include/core/states/GameState.h`, `src/core/states/GameState.cpp`

- Основное игровое состояние
- EnTT registry для управления сущностями
- SystemScheduler с 6 системами
- TileMapSystem для TMX карт
- Dual-view система (мировой вид + UI вид)
- Управление камерой (WASD, zoom колесом мыши)
- Отладочная сетка тайлов (F6)
- Переход в паузу (Escape)

**Особенности:**
- Не обновляется под PauseState (`updateBelow() = false`)
- Рендерится под PauseState (`renderBelow() = true`)

#### 3. PauseState ✅

**Расположение:** `include/core/states/PauseState.h`, `src/core/states/PauseState.cpp`

- Оверлей с полупрозрачным фоном
- Меню: Resume, Settings, Main Menu, Exit
- Показывает игру под собой
- Останавливает обновление игры

## Вспомогательные системы

### InputManager ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/InputManager.h`, `src/core/InputManager.cpp`

Отслеживание состояний ввода:

```cpp
class InputManager {
public:
    void update();
    void handleEvent(const sf::Event& event);
    void reset();

    // Клавиатура
    bool isKeyPressed(sf::Keyboard::Key key) const;
    bool isKeyJustPressed(sf::Keyboard::Key key) const;
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

    // Мышь
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    sf::Vector2i getMousePosition() const;
    float getMouseWheelDelta() const;

private:
    std::unordered_set<sf::Keyboard::Key> m_keysPressed;
    std::unordered_set<sf::Keyboard::Key> m_keysJustPressed;
    std::unordered_set<sf::Keyboard::Key> m_keysJustReleased;
    std::unordered_set<sf::Mouse::Button> m_mouseButtonsPressed;
    sf::Vector2i m_mousePosition;
    float m_mouseWheelDelta;
};
```

**Особенности:**
- Различает "pressed" (зажата) и "just pressed" (только что нажата)
- Автоматическая очистка "just" событий каждый кадр

### Logger ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/Logger.h`, `src/core/Logger.cpp`

Система логирования на базе spdlog:

```cpp
class Logger {
public:
    static void initialize();
    static void shutdown();
    static std::shared_ptr<spdlog::logger> getLogger();
    static void setLevel(spdlog::level::level_enum level);
};

// Макросы для удобства
#define LOG_TRACE(...)    Logger::getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getLogger()->critical(__VA_ARGS__)
```

**Конфигурация:**
- Вывод в консоль и файл `logs/opc_game_sim.log`
- Ротация файлов: 5 МБ, максимум 3 файла
- Формат: `[timestamp] [level] message`

### Window ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/Window.h`, `src/core/Window.cpp`

Обертка над sf::RenderWindow:

```cpp
class Window {
public:
    explicit Window(const WindowConfig& config);

    bool isOpen() const;
    bool pollEvent(sf::Event& event);
    void clear(const sf::Color& color = sf::Color::Black);
    void display();
    void close();

    sf::RenderWindow& getRenderWindow();
    sf::Vector2u getSize() const;

private:
    sf::RenderWindow m_window;
};

struct WindowConfig {
    std::string title;
    unsigned int width;
    unsigned int height;
    unsigned int frameRateLimit;
    bool vsync;
};
```

### Application ✅ РЕАЛИЗОВАНО

**Расположение:** `include/core/Application.h`, `src/core/Application.cpp`

Главный класс приложения:

```cpp
class Application {
public:
    explicit Application(const WindowConfig& config);
    ~Application();

    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void logPerformanceMetrics();

    Window m_window;
    StateManager m_stateManager;
    InputManager m_inputManager;
    ResourceManager m_resourceManager;
    PerformanceMetrics m_metrics;

    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 UPS
    static constexpr float MAX_FRAME_TIME = 0.25f;         // Защита от spiral of death
};
```

**Игровой цикл:**
- "Fix Your Timestep" паттерн
- Фиксированный timestep 1/60 (60 обновлений в секунду)
- Переменный framerate для рендеринга
- Защита от "spiral of death" (макс. 0.25с)

## Система координат и тайлов

### Координатная система

**Размер тайла:** 32x32 пикселя (константа `TILE_SIZE`)

**Конвертация координат:**
```cpp
// Тайловые → Пиксельные
pixelX = tileX * TILE_SIZE;
pixelY = (tileY + heightInTiles) * TILE_SIZE;  // Bottom-left origin

// Пиксельные → Тайловые
tileX = pixelX / TILE_SIZE;
tileY = (pixelY / TILE_SIZE) - heightInTiles;
```

**Origin спрайта:**
- Статичные объекты: bottom-left corner (0, textureRect.height)
- Вращающиеся объекты: center (textureRect.width/2, textureRect.height/2)

**Обоснование:** Bottom-left origin интуитивнее для 2D игр, где объекты "стоят" на земле

### Y-Sorting для 3/4 перспективы

Объекты с бОльшей Y-координатой (ниже на экране) рендерятся позже:

```cpp
// В TilePositionSystem
spriteComponent.layer = RenderLayer::Objects + tileY;

// В RenderSystem
std::sort(entities.begin(), entities.end(), [](auto a, auto b) {
    return a.layer < b.layer ||
          (a.layer == b.layer && a.y < b.y);
});
```

### Слои рендеринга

| Слой | Значение | Использование |
|------|----------|---------------|
| Background | 0 | Фоновые элементы |
| Ground | 100 | Земля, пол |
| Objects | 200 + tileY | Игровые объекты с Y-sorting |
| Overlays | 300 + tileY | UI оверлеи привязанные к объектам |
| UIOverlay | 400 | Верхнеуровневые UI элементы |

## Камера и управление

### Управление камерой в GameState

**Расположение:** `include/core/states/GameState.h:198-244`

```cpp
// Константы
static constexpr float CAMERA_MOVE_SPEED = 600.0f;       // пикселей/сек
static constexpr float CAMERA_ZOOM_SPEED = 0.1f;         // за шаг
static constexpr float CAMERA_MIN_ZOOM = 0.2f;
static constexpr float CAMERA_MAX_ZOOM = 1.0f;
static constexpr float CAMERA_DEFAULT_ZOOM = 0.5f;

// Управление
WASD / Стрелки - Перемещение камеры
Колесо мыши - Зум
F6 - Переключить отладочную сетку
Escape - Пауза
```

### Dual-View система

```cpp
// Мировой вид (камера с зумом)
sf::View m_worldView;
window.setView(m_worldView);
// Рендеринг игрового мира

// UI вид (фиксированный 1280x720)
sf::View m_uiView;
window.setView(m_uiView);
// Рендеринг UI элементов
```

**Обоснование:** UI всегда остается четким и не зависит от зума камеры

## Дополнительные материалы

### Документация библиотек
- [EnTT Documentation](https://github.com/skypjack/entt)
- [SFML 3 Documentation](https://www.sfml-dev.org/documentation/3.0.0/)
- [Box2D Manual](https://box2d.org/documentation/) (для будущей интеграции)
- [OPC UA Specification](https://opcfoundation.org/) (для будущей интеграции)
- [tmxlite](https://github.com/fallahn/tmxlite)
- [spdlog](https://github.com/gabime/spdlog)

### Проектная документация
- `ROADMAP.md` - План разработки на 10 фаз
- `TASK.md` - Технологический стек
- `CLAUDE.md` - Руководство для разработки
- `README.md` - Общее описание проекта
