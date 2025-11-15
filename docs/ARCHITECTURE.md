# Архитектура проекта

## Общий обзор

OPC Game Simulator построен на модульной архитектуре с использованием паттерна ECS (Entity Component System). Проект разделен на независимые модули, каждый из которых отвечает за определенную функциональность.

## Модули системы

### Core (Ядро)
Базовая функциональность приложения:
- Управление окном и контекстом
- Игровой цикл с фиксированным timestep
- Система состояний (State Machine)
- Менеджер ресурсов
- Система логирования

### Simulation (Симуляция)
Физика и игровая логика:
- Интеграция Box2D для физики
- Симуляция промышленных процессов
- Управление временем симуляции
- Обработка коллизий

### Rendering (Рендеринг)
Визуализация:
- Система рендеринга на базе SFML
- Управление камерой (zoom, pan)
- Рендеринг тайловых карт (TMX)
- Многослойная отрисовка
- Визуализация физики (debug)

### Industrial (Промышленные протоколы)
OPC UA и другие промышленные протоколы:
- OPC UA клиент (open62541)
- Система привязок переменных
- Обработка подписок
- Масштабирование и преобразование данных
- Modbus TCP/RTU (опционально)

### Scripting (Скриптинг)
Lua интеграция:
- Движок скриптов
- API для работы с ECS
- Загрузка и выполнение скриптов
- Обработка событий

### Editor (Редактор)
Инструменты редактирования:
- Редактор уровней
- Размещение объектов
- Система сохранения/загрузки
- Управление проектами (SQLite)

### UI (Пользовательский интерфейс)
ImGui интерфейс:
- Инспектор объектов
- Панель иерархии
- Браузер OPC UA
- Графики реального времени (ImPlot)
- Консоль логов

## ECS архитектура

### Компоненты (Components)

Компоненты - это данные без логики:

```cpp
struct TransformComponent {
    float x, y;
    float rotation;
    float scale;
};

struct SpriteComponent {
    std::string texturePath;
    sf::IntRect textureRect;
    sf::Color color;
};

struct SensorComponent {
    SensorType type;
    float value;
    float minValue, maxValue;
};

struct PLCBindingComponent {
    std::string nodeId;
    BindingType type;
    float scaleFactor;
    float offset;
};
```

### Системы (Systems)

Системы - это логика без данных:

```cpp
class RenderSystem {
public:
    void Update(entt::registry& registry) {
        auto view = registry.view<TransformComponent, SpriteComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            auto& sprite = view.get<SpriteComponent>(entity);
            // Отрисовка спрайта
        }
    }
};

class PhysicsSystem {
public:
    void Update(entt::registry& registry, float deltaTime) {
        // Шаг физической симуляции
        world->Step(deltaTime, velocityIterations, positionIterations);

        // Синхронизация ECS с Box2D
        auto view = registry.view<TransformComponent, RigidBodyComponent>();
        for (auto entity : view) {
            // Обновление позиций
        }
    }
};
```

## Потоки выполнения

### Главный поток
- Обработка ввода
- Обновление UI (ImGui)
- Рендеринг
- Координация других потоков

### Поток физики
- Симуляция Box2D
- Обновление состояний объектов
- Обработка коллизий

### Поток OPC UA
- Коммуникация с ПЛК
- Обработка подписок
- Чтение/запись переменных

### Поток скриптов (опционально)
- Выполнение Lua скриптов
- Обработка событий

## Система событий

Используется `boost::signals2` для развязки модулей:

```cpp
// Определение сигнала
boost::signals2::signal<void(float)> onSensorValueChanged;

// Подписка
onSensorValueChanged.connect([](float value) {
    std::cout << "Sensor value: " << value << std::endl;
});

// Генерация события
onSensorValueChanged(25.5f);
```

## Управление ресурсами

### Текстуры
```cpp
class ResourceManager {
    std::unordered_map<std::string, sf::Texture> textures;

public:
    const sf::Texture& GetTexture(const std::string& path);
    void LoadTexture(const std::string& path);
};
```

### Карты
```cpp
class MapManager {
    tmx::Map currentMap;

public:
    bool LoadMap(const std::string& path);
    void RenderMap(sf::RenderTarget& target);
};
```

## Система сохранения

### SQLite схема
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

### Архитектура клиента

```cpp
class OPCUAClient {
    UA_Client* client;
    std::thread workerThread;

public:
    void Connect(const std::string& endpoint);
    void Subscribe(const std::string& nodeId, VariableCallback callback);
    void WriteValue(const std::string& nodeId, const UA_Variant& value);
};
```

### Система привязок

```cpp
class BindingSystem {
    OPCUAClient& client;

public:
    void Update(entt::registry& registry) {
        auto view = registry.view<PLCBindingComponent>();
        for (auto entity : view) {
            auto& binding = view.get<PLCBindingComponent>(entity);
            // Синхронизация с OPC UA
        }
    }
};
```

## Производительность

### Оптимизация рендеринга
- Frustum culling для тайлов
- Батчинг спрайтов
- Многопоточная подготовка данных

### Оптимизация ECS
- Cache-friendly итерация
- Минимизация аллокаций
- Использование view вместо итерации по всем сущностям

### Профилирование
Tracy profiler для анализа:
- Время кадра
- Горячие точки CPU
- Аллокации памяти

## Расширяемость

### Добавление нового компонента

1. Определите структуру компонента
2. Создайте систему для обработки
3. Зарегистрируйте в ImGui для инспектора
4. Добавьте сериализацию

### Добавление нового промышленного объекта

1. Создайте компоненты для объекта
2. Реализуйте систему симуляции
3. Добавьте спрайты и анимации
4. Настройте привязки OPC UA

## Диаграммы

### Поток данных OPC UA
```
ПЛК → OPC UA Server → OPC UA Client → BindingSystem → ECS Components → Simulation/Rendering
```

### Игровой цикл
```
Input → Update Systems → Physics Step → OPC UA Sync → Render → Present
```

## Дополнительные материалы

- [EnTT Documentation](https://github.com/skypjack/entt)
- [Box2D Manual](https://box2d.org/documentation/)
- [OPC UA Specification](https://opcfoundation.org/)
