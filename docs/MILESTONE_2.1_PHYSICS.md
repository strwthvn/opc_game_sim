# Milestone 2.1: Физическая симуляция

**Фаза:** 2 - Промышленные компоненты и физика
**Статус:** ⏳ Планирование
**Зависимости:** Завершение Фазы 1 (ECS, рендеринг, тайловая система)

---

## Обзор

Интеграция Box2D для реалистичной физической симуляции промышленных объектов. Включает систему физики, компоненты для rigid body, коллизии, триггеры и отладочную визуализацию.

---

## Задача 1: Интеграция Box2D в проект

### Описание
Настроить Box2D как зависимость проекта, создать базовую инфраструктуру для физического мира.

### Подзадачи

#### 1.1. Добавление зависимости Box2D
- [x] Добавить `box2d` в `vcpkg.json`
- [x] Обновить `CMakeLists.txt` в модуле Simulation:
  ```cmake
  find_package(box2d CONFIG REQUIRED)
  target_link_libraries(Simulation PRIVATE box2d::box2d)
  ```
- [x] Проверить сборку проекта с новой зависимостью

#### 1.2. Создание PhysicsWorld
- [x] Создать класс `PhysicsWorld` в `include/simulation/PhysicsWorld.h`
- [x] Реализовать инициализацию `b2World` с гравитацией
- [x] Добавить методы:
  - `void step(float deltaTime)` - шаг симуляции
  - `void setGravity(const b2Vec2& gravity)`
  - `b2World* getWorld()` - доступ к Box2D миру
- [x] Документировать API с помощью Doxygen

#### 1.3. Тесты для PhysicsWorld
- [x] Создать `tests/simulation/PhysicsWorldTests.cpp`
- [x] Тест: Создание мира с заданной гравитацией
- [x] Тест: Шаг симуляции не вызывает крашей
- [x] Тест: Изменение гравитации применяется корректно
- [x] Запустить тесты: `./build/bin/UnitTests "[PhysicsWorld]"`

---

## Задача 2: Компонент RigidBodyComponent

### Описание
Создать ECS компонент для хранения ссылки на физическое тело Box2D и его параметров.

### Подзадачи

#### 2.1. Определение компонента
- [x] Создать `include/simulation/components/RigidBodyComponent.h`
- [x] Определить структуру:
  ```cpp
  struct RigidBodyComponent {
      b2Body* body = nullptr;
      b2BodyType bodyType = b2_dynamicBody;
      float density = 1.0f;
      float friction = 0.3f;
      float restitution = 0.0f;
      bool fixedRotation = false;
  };
  ```
- [x] Добавить Doxygen комментарии для полей

#### 2.2. Фабрика физических тел
- [x] Создать `PhysicsBodyFactory` в `include/simulation/PhysicsBodyFactory.h`
- [x] Реализовать методы:
  - `b2Body* createBox(b2World*, const TransformComponent&, const RigidBodyComponent&)`
  - `b2Body* createCircle(b2World*, const TransformComponent&, float radius, const RigidBodyComponent&)`
  - `void destroyBody(b2World*, b2Body*)`
- [x] Реализовать конвертацию координат: пиксели ↔ Box2D метры (32 пикселя = 1 метр)

#### 2.3. Тесты для RigidBodyComponent
- [x] Создать `tests/simulation/RigidBodyComponentTests.cpp`
- [x] Тест: Создание Box2D тела через фабрику (box)
- [x] Тест: Создание Box2D тела через фабрику (circle)
- [x] Тест: Параметры тела (density, friction, restitution) применяются
- [x] Тест: Удаление тела не вызывает утечек памяти
- [x] Тест: Конвертация координат пиксели ↔ метры корректна

---

## Задача 3: Система PhysicsSystem

### Описание
Реализовать систему для обновления физики и синхронизации состояния между Box2D и ECS.

### Подзадачи

#### 3.1. Базовая реализация PhysicsSystem
- [x] Создать `include/simulation/systems/PhysicsSystem.h`
- [x] Реализовать `src/simulation/systems/PhysicsSystem.cpp`
- [x] Добавить методы:
  - `void init(entt::registry&, PhysicsWorld&)` - создание тел для всех сущностей с RigidBodyComponent
  - `void update(entt::registry&, PhysicsWorld&, float deltaTime)` - шаг симуляции
  - `void syncTransforms(entt::registry&)` - синхронизация позиций/углов из Box2D в TransformComponent
- [x] Реализовать fixed timestep для физики (60 Hz)

#### 3.2. Синхронизация трансформаций
- [x] При создании тела: позиция из `TransformComponent` → `b2Body`
- [x] После шага симуляции: позиция/угол из `b2Body` → `TransformComponent`
- [x] Обработка статических тел (не обновлять TransformComponent)
- [x] Учет пиксельной системы координат и origin спрайтов (bottom-left)

#### 3.3. Тесты для PhysicsSystem
- [x] Создать `tests/simulation/PhysicsSystemTests.cpp`
- [x] Тест: Инициализация создает b2Body для всех сущностей с RigidBodyComponent
- [x] Тест: Динамическое тело падает под действием гравитации
- [x] Тест: Статическое тело не двигается
- [x] Тест: TransformComponent синхронизируется с b2Body после step()
- [x] Тест: Кинематическое тело двигается при изменении TransformComponent
- [x] Интеграционный тест: Столкновение двух объектов обрабатывается

---

## Задача 4: Коллизии и триггеры

### Описание
Реализовать обработку столкновений и триггерных зон с использованием Box2D 3.x event system.

**Примечание:** Box2D 3.x использует event-based подход вместо callback-based (b2ContactListener).
События буферизируются во время симуляции и доступны после каждого шага.

### Подзадачи

#### 4.1. Компонент ColliderComponent (фильтрация коллизий)
- [x] ColliderComponent уже существует в `include/simulation/PhysicsComponents.h`
- [x] Добавить поля для фильтрации коллизий:
  - `uint16_t categoryBits` - категория этого коллайдера
  - `uint16_t maskBits` - маска категорий для фильтрации
  - `int16_t groupIndex` - группа коллизий
- [x] Документировать поля (Doxygen)

#### 4.2. PhysicsEventProcessor для событий коллизий (Box2D 3.x)
- [x] Создать `PhysicsEventProcessor` в `include/simulation/PhysicsEventProcessor.h`
- [x] Реализовать обработку Box2D 3.x событий:
  - `b2ContactBeginTouchEvent` - начало столкновения
  - `b2ContactEndTouchEvent` - конец столкновения
  - `b2ContactHitEvent` - высокоскоростное столкновение
  - `b2SensorBeginTouchEvent` - вход в триггер
  - `b2SensorEndTouchEvent` - выход из триггера
- [x] Использовать `b2Body_GetUserData()` для получения `entt::entity`
- [x] Включить генерацию событий в `b2ShapeDef` (enableContactEvents, enableSensorEvents, enableHitEvents)

#### 4.3. Система событий коллизий
- [x] Определить события в `include/simulation/events/CollisionEvents.h`:
  ```cpp
  struct CollisionBeginEvent { entt::entity entityA, entityB; ... };
  struct CollisionEndEvent { entt::entity entityA, entityB; ... };
  struct CollisionHitEvent { entt::entity entityA, entityB; float approachSpeed; ... };
  struct TriggerEnterEvent { entt::entity triggerEntity, otherEntity; ... };
  struct TriggerExitEvent { entt::entity triggerEntity, otherEntity; ... };
  ```
- [x] Интегрировать `boost::signals2` для отправки событий (CollisionSignals)
- [x] PhysicsEventProcessor отправляет события через сигналы

#### 4.4. Тесты для коллизий
- [x] Создать `tests/unit/test_collision_events.cpp`
- [x] Тест: CollisionBeginEvent генерируется при столкновении двух тел
- [x] Тест: CollisionEndEvent генерируется при разделении тел
- [x] Тест: TriggerEnterEvent отправляется для триггерных коллайдеров
- [x] Тест: Фильтрация коллизий по categoryBits/maskBits работает
- [x] Интеграционный тест: Объект падает на платформу и генерирует событие

---

## Задача 5: Отладочная визуализация физики

### Описание
Реализовать визуализацию физических тел, коллайдеров и контактных точек для отладки.

### Подзадачи

#### 5.1. DebugDraw для Box2D
- [x] Создать класс `PhysicsDebugDraw` в `include/rendering/PhysicsDebugDraw.h`
- [x] Реализовать callback-based API для Box2D 3.x (b2DebugDraw структура с function pointers)
- [x] Реализовать методы:
  - `DrawPolygon` - контур полигона
  - `DrawSolidPolygon` - заполненный полигон с контуром
  - `DrawCircle` / `DrawSolidCircle` - круги
  - `DrawCapsule` - капсулы
  - `DrawSegment` - отрезки
  - `DrawTransform` - оси координат
  - `DrawPoint` - точки
- [x] Использовать `sf::VertexArray` для рендеринга примитивов (batching)

#### 5.2. Интеграция с GameState
- [x] Добавить флаг `bool m_debugDrawPhysics` в `GameState`
- [x] Реализовать рендеринг через `b2World_Draw()` в `GameState::render()`
- [x] Добавить горячую клавишу **F7** для переключения debug draw
- [x] Рендерить после основного слоя спрайтов, перед UI

#### 5.3. Визуализация дополнительной информации
- [x] Отрисовка центра масс (через setDrawMass)
- [x] Отрисовка AABB коллайдеров (через setDrawBounds)
- [x] Отрисовка контактных точек (через setDrawContacts)
- [ ] Отрисовка векторов скорости (синие стрелки) - TODO для следующей итерации

#### 5.4. Тесты для DebugDraw
- [x] Создать `tests/unit/test_physics_debug_draw.cpp`
- [x] Тест: PhysicsDebugDraw создается без ошибок
- [x] Тест: Все function pointers установлены
- [x] Тест: Флаги конфигурации работают корректно
- [x] Тест: begin/end не вызывают крашей
- [x] Интеграционный тест с Box2D миром

---

## Задача 6: Интеграция с многопоточностью

### Описание
Вынести физическую симуляцию в отдельный поток для повышения производительности.

### Подзадачи

#### 6.1. Поток физики
- [ ] Создать класс `PhysicsThread` в `include/simulation/PhysicsThread.h`
- [ ] Использовать `std::thread` для запуска физического цикла
- [ ] Реализовать fixed timestep 60 Hz (16.67 ms)
- [ ] Добавить флаг `std::atomic<bool> running` для остановки потока

#### 6.2. Синхронизация между потоками
- [ ] Использовать `std::mutex` для защиты доступа к `entt::registry`
- [ ] Реализовать double buffering для TransformComponent:
  - Поток физики пишет в буфер A
  - Главный поток читает из буфера B
  - Swap буферов каждый кадр
- [ ] Минимизировать время удержания мьютекса

#### 6.3. Управление жизненным циклом
- [ ] Запуск потока в `Application::init()`
- [ ] Остановка потока в `Application::shutdown()`
- [ ] Обработка исключений в потоке физики
- [ ] Логирование ошибок через spdlog (thread-safe)

#### 6.4. Тесты для многопоточности
- [ ] Создать `tests/simulation/PhysicsThreadTests.cpp`
- [ ] Тест: Поток запускается и останавливается корректно
- [ ] Тест: Физика работает параллельно с главным потоком
- [ ] Тест: Нет race conditions при доступе к registry (thread sanitizer)
- [ ] Стресс-тест: 1000 физических объектов в течение 10 секунд
- [ ] Тест: Обработка исключений в потоке не крашит приложение

---

## Задача 7: Документация и примеры

### Описание
Создать документацию и примеры использования физической системы.

### Подзадачи

#### 7.1. Doxygen документация
- [ ] Документировать все публичные API:
  - `PhysicsWorld`
  - `PhysicsSystem`
  - `PhysicsBodyFactory`
  - `PhysicsContactListener`
  - `PhysicsDebugDraw`
  - `PhysicsThread`
- [ ] Добавить примеры использования в комментариях `@code ... @endcode`
- [ ] Обновить `docs/ARCHITECTURE.md` с разделом о физике

#### 7.2. Примеры кода
- [ ] Создать `examples/physics_demo.cpp`:
  - Создание сцены с несколькими объектами
  - Демонстрация коллизий
  - Использование триггеров
  - Визуализация debug draw
- [ ] Добавить в CMakeLists опциональную сборку примеров (`BUILD_EXAMPLES=ON`)

#### 7.3. Руководство для разработчиков
- [ ] Создать `docs/PHYSICS_GUIDE.md`:
  - Как добавить физику к объекту
  - Настройка параметров (density, friction, restitution)
  - Обработка событий коллизий
  - Производительность и best practices
  - Конвертация координат (пиксели ↔ метры)

---

## Критерии приемки

### Функциональность
- [ ] Все тесты пройдены (unit + integration)
- [ ] Box2D корректно симулирует физику (падение, коллизии, трение)
- [ ] TransformComponent синхронизируется с Box2D без рассинхронизации
- [ ] События коллизий отправляются корректно
- [ ] Debug draw отображает все физические тела

### Производительность
- [ ] Физика работает в отдельном потоке без блокировок главного потока
- [ ] 1000+ физических объектов симулируются при 60 FPS
- [ ] Нет race conditions (проверено с thread sanitizer)

### Качество кода
- [ ] Весь код отформатирован согласно `.clang-format`
- [ ] Публичные API документированы (Doxygen)
- [ ] Нет предупреждений компилятора
- [ ] Code coverage тестами > 80%

### Документация
- [ ] API документация сгенерирована (Doxygen)
- [ ] Руководство для разработчиков написано
- [ ] Примеры кода компилируются и работают

---

## Зависимости и риски

### Зависимости
- **Milestone 1.2**: ECS архитектура (EnTT, TransformComponent)
- **Milestone 1.3**: Тайловая система и координаты

### Риски
1. **Рассинхронизация координат**: Box2D использует метры, проект - пиксели
   - **Митигация**: Константа конвертации `PIXELS_PER_METER = 32.0f`, тщательное тестирование
2. **Производительность многопоточности**: Мьютексы могут замедлить симуляцию
   - **Митигация**: Double buffering, минимизация критических секций
3. **Сложность отладки коллизий**: Физические артефакты трудно воспроизвести
   - **Митигация**: Debug draw, детальное логирование событий

---

## Следующие шаги (Milestone 2.2)

После завершения Milestone 2.1 переходим к **Базовым промышленным объектам**:
- SensorComponent (датчики)
- ActuatorComponent (исполнительные механизмы)
- ConveyorComponent (конвейеры)
- Интеграция с физикой

---

**Версия документа:** 1.0
**Дата создания:** 2025-12-21
**Автор:** Claude Code
