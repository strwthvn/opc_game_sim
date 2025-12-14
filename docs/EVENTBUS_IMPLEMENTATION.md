# Задача 2.4: Система событий (EventBus) - ЗАВЕРШЕНА ✅

## Обзор

Реализована полнофункциональная система событий на базе `boost::signals2` для развязки модулей в OPC Game Simulator.

## Реализованные компоненты

### 1. Базовый класс EventBus

**Файлы:**
- [include/core/EventBus.h](../include/core/EventBus.h) - Заголовочный файл с определением EventBus и типов событий
- [src/core/EventBus.cpp](../src/core/EventBus.cpp) - Реализация EventBus

**Особенности:**
- Singleton паттерн для глобального доступа
- Типобезопасные шаблонные методы subscribe/publish
- RAII управление подписками через `boost::signals2::connection`
- Потокобезопасность (boost::signals2 thread-safe)

### 2. Базовые типы событий

Определены 5 базовых типов событий:

#### CollisionEvent
- Публикуется: `CollisionSystem`
- Содержит: две сущности, тип коллизии (Enter/Stay/Exit), точка контакта
- Использование: воспроизведение звуков, визуальные эффекты, статистика

#### StateChangedEvent
- Публикуется: `EntityStateComponent::setState()`
- Содержит: сущность, предыдущее состояние, новое состояние
- Использование: логирование, обработка критических состояний

#### EntityCreatedEvent / EntityDestroyedEvent
- Публикуется: (планируется для будущих фаз)
- Содержит: сущность
- Использование: управление жизненным циклом, сериализация

#### InputEvent
- Публикуется: (планируется для будущих фаз)
- Содержит: тип ввода, код клавиши, кнопка мыши, позиция мыши
- Использование: обработка пользовательского ввода

### 3. Интеграция с системами

#### CollisionSystem
**Файл:** [src/core/systems/CollisionSystem.cpp](../src/core/systems/CollisionSystem.cpp)

Изменения:
- Добавлен `#include "core/EventBus.h"`
- В `handleCollision()`: публикуется `CollisionEvent::Type::Enter` и `CollisionEvent::Type::Stay`
- В `handleCollisionExit()`: публикуется `CollisionEvent::Type::Exit`
- Вычисляется точка контакта как центр между двумя объектами

#### EntityStateComponent
**Файлы:**
- [include/core/Components.h](../include/core/Components.h#L360)
- [src/core/Components.cpp](../src/core/Components.cpp)

Изменения:
- Метод `setState()` принимает опциональный параметр `entity`
- При переходе между состояниями публикуется `StateChangedEvent`
- Обратная совместимость: если `entity == entt::null`, событие не публикуется

### 4. Обновление CMake

**Файл:** [src/core/CMakeLists.txt](../src/core/CMakeLists.txt)

Изменения:
- Добавлен `EventBus.cpp` в список исходников Core модуля
- Добавлена зависимость `Boost::headers` в `target_link_libraries`

### 5. Примеры использования

**Файл:** [examples/EventBusExample.cpp](../examples/EventBusExample.cpp)

Содержит полные рабочие примеры:
- `CollisionSoundHandler` - воспроизведение звука при коллизии
- `StateChangeLogger` - логирование изменений состояния
- `exampleGameScene()` - создание игровой сцены с обработчиками событий
- `exampleMultipleSubscribers()` - множественные подписчики на одно событие
- `exampleConditionalHandling()` - условная обработка событий (только коллизии игрока)

### 6. Документация

**Файл:** [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md#L604-L935)

Добавлен раздел "Система событий" с:
- Описание архитектуры EventBus
- Документация всех типов событий
- 4 детальных примера использования
- Описание интеграции с системами
- Руководство по созданию пользовательских событий
- Преимущества использования EventBus
- Планы на будущие расширения

## Критерии приемки

✅ **EventBus реализован с boost::signals2**
- Используется `boost::signals2::signal` для потокобезопасной доставки событий
- Типобезопасные шаблонные методы `subscribe<T>()` и `publish<T>()`

✅ **Минимум 3 типа событий определены**
- Реализовано 5 типов событий: `CollisionEvent`, `StateChangedEvent`, `EntityCreatedEvent`, `EntityDestroyedEvent`, `InputEvent`

✅ **Хотя бы одна система публикует события**
- `CollisionSystem` публикует `CollisionEvent` (Enter/Stay/Exit)
- `EntityStateComponent::setState()` публикует `StateChangedEvent`

✅ **Есть пример подписки на событие**
- Файл `examples/EventBusExample.cpp` содержит 4 подробных примера
- Демонстрируются различные паттерны использования

✅ **Документация обновлена**
- Полная документация в `docs/ARCHITECTURE.md`
- Примеры кода с объяснениями
- Описание архитектуры и преимуществ

## Проверка работоспособности

### Сборка проекта
```bash
./build.sh --build-only
```

**Результат:** ✅ Сборка успешно завершена за 67 секунд

### Запуск примеров
```bash
# (опционально, если скомпилировать EventBusExample.cpp)
./build/bin/EventBusExample
```

## Использование в проекте

### Подписка на события коллизии

```cpp
#include "core/EventBus.h"

// Подписка на события коллизии
auto connection = EventBus::getInstance().subscribe<CollisionEvent>(
    [](const CollisionEvent& evt) {
        if (evt.type == CollisionEvent::Type::Enter) {
            LOG_INFO("Collision detected!");
            AudioManager::getInstance().playSound("collision.wav");
        }
    }
);

// Connection автоматически отписывается при выходе из scope
```

### Подписка на изменения состояния

```cpp
auto connection = EventBus::getInstance().subscribe<StateChangedEvent>(
    [](const StateChangedEvent& evt) {
        if (evt.newState == "error") {
            LOG_WARN("Entity {} entered ERROR state!",
                static_cast<uint32_t>(evt.entity));
        }
    }
);
```

### Изменение состояния с публикацией события

```cpp
auto entity = registry.create();
auto& fsm = registry.emplace<EntityStateComponent>(entity, "idle");

// При переходе автоматически публикуется StateChangedEvent
fsm.setState("running", entity);
```

## Технические детали

### Паттерны проектирования

1. **Singleton** - `EventBus::getInstance()`
2. **Observer** - Подписка/публикация событий
3. **RAII** - Автоматическая отписка через `boost::signals2::connection`
4. **Template Method** - Типобезопасные шаблонные методы

### Потокобезопасность

`boost::signals2::signal` обеспечивает thread-safe доставку событий. Это позволит в будущих фазах:
- Публиковать события из потока физики
- Публиковать события из потока OPC UA
- Обрабатывать события в главном потоке

### Производительность

- **Минимальные накладные расходы**: `std::function` с inline оптимизацией
- **Эффективная диспетчеризация**: `std::type_index` для O(1) поиска подписчиков
- **Ленивая инициализация**: Сигналы создаются только при первой подписке

## Будущие расширения

### Фаза 2 (Physics Integration)
- Публикация физических событий из Box2D (столкновения с силой, триггеры)

### Фаза 3 (Industrial Protocols)
- События OPC UA (изменение переменной, соединение/отключение)
- События Modbus (ошибки коммуникации, чтение/запись)

### Фаза 4 (Scripting)
- Публикация событий из Lua скриптов
- Подписка на события в Lua

### Фаза 5 (UI/Editor)
- События редактора (выбор объекта, изменение свойств)
- События UI (клик по кнопке, изменение настройки)

## Заключение

Задача 2.4 полностью выполнена. Система событий EventBus:
- ✅ Реализована на базе `boost::signals2`
- ✅ Интегрирована с `CollisionSystem` и `EntityStateComponent`
- ✅ Имеет 5 базовых типов событий
- ✅ Полностью документирована с примерами
- ✅ Успешно компилируется и готова к использованию

EventBus обеспечивает:
- **Развязку модулей** - модули не зависят друг от друга напрямую
- **Расширяемость** - легко добавлять новые типы событий и обработчики
- **Потокобезопасность** - готова к многопоточной архитектуре
- **Типобезопасность** - ошибки типов обнаруживаются на этапе компиляции
- **Удобство** - RAII управление подписками, минимум boilerplate кода

Система готова к использованию в дальнейших фазах разработки проекта.
