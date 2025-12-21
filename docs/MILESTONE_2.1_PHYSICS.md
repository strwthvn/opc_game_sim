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
- [ ] Создать `include/simulation/components/RigidBodyComponent.h`
- [ ] Определить структуру:
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
- [ ] Добавить Doxygen комментарии для полей

#### 2.2. Фабрика физических тел
- [ ] Создать `PhysicsBodyFactory` в `include/simulation/PhysicsBodyFactory.h`
- [ ] Реализовать методы:
  - `b2Body* createBox(b2World*, const TransformComponent&, const RigidBodyComponent&)`
  - `b2Body* createCircle(b2World*, const TransformComponent&, float radius, const RigidBodyComponent&)`
  - `void destroyBody(b2World*, b2Body*)`
- [ ] Реализовать конвертацию координат: пиксели ↔ Box2D метры (32 пикселя = 1 метр)

#### 2.3. Тесты для RigidBodyComponent
- [ ] Создать `tests/simulation/RigidBodyComponentTests.cpp`
- [ ] Тест: Создание Box2D тела через фабрику (box)
- [ ] Тест: Создание Box2D тела через фабрику (circle)
- [ ] Тест: Параметры тела (density, friction, restitution) применяются
- [ ] Тест: Удаление тела не вызывает утечек памяти
- [ ] Тест: Конвертация координат пиксели ↔ метры корректна

---

## Задача 3: Система PhysicsSystem

### Описание
Реализовать систему для обновления физики и синхронизации состояния между Box2D и ECS.

### Подзадачи

#### 3.1. Базовая реализация PhysicsSystem
- [ ] Создать `include/simulation/systems/PhysicsSystem.h`
- [ ] Реализовать `src/simulation/systems/PhysicsSystem.cpp`
- [ ] Добавить методы:
  - `void init(entt::registry&, PhysicsWorld&)` - создание тел для всех сущностей с RigidBodyComponent
  - `void update(entt::registry&, PhysicsWorld&, float deltaTime)` - шаг симуляции
  - `void syncTransforms(entt::registry&)` - синхронизация позиций/углов из Box2D в TransformComponent
- [ ] Реализовать fixed timestep для физики (60 Hz)

#### 3.2. Синхронизация трансформаций
- [ ] При создании тела: позиция из `TransformComponent` → `b2Body`
- [ ] После шага симуляции: позиция/угол из `b2Body` → `TransformComponent`
- [ ] Обработка статических тел (не обновлять TransformComponent)
- [ ] Учет пиксельной системы координат и origin спрайтов (bottom-left)

#### 3.3. Тесты для PhysicsSystem
- [ ] Создать `tests/simulation/PhysicsSystemTests.cpp`
- [ ] Тест: Инициализация создает b2Body для всех сущностей с RigidBodyComponent
- [ ] Тест: Динамическое тело падает под действием гравитации
- [ ] Тест: Статическое тело не двигается
- [ ] Тест: TransformComponent синхронизируется с b2Body после step()
- [ ] Тест: Кинематическое тело двигается при изменении TransformComponent
- [ ] Интеграционный тест: Столкновение двух объектов обрабатывается

---

## Задача 4: Коллизии и триггеры

### Описание
Реализовать обработку столкновений и триггерных зон с использованием Box2D contact listener.

### Подзадачи

#### 4.1. Компонент ColliderComponent
- [ ] Создать `include/simulation/components/ColliderComponent.h`
- [ ] Определить структуру:
  ```cpp
  struct ColliderComponent {
      enum class Type { Box, Circle, Polygon };
      Type type = Type::Box;
      sf::Vector2f size{32.0f, 32.0f}; // для Box
      float radius = 16.0f;            // для Circle
      bool isTrigger = false;
      uint16_t categoryBits = 0x0001;
      uint16_t maskBits = 0xFFFF;
  };
  ```
- [ ] Документировать поля (Doxygen)

#### 4.2. ContactListener для событий коллизий
- [ ] Создать `PhysicsContactListener` в `include/simulation/PhysicsContactListener.h`
- [ ] Наследовать от `b2ContactListener`
- [ ] Переопределить методы:
  - `BeginContact(b2Contact*)` - начало столкновения
  - `EndContact(b2Contact*)` - конец столкновения
  - `PreSolve(b2Contact*, const b2Manifold*)` - перед разрешением
  - `PostSolve(b2Contact*, const b2ContactImpulse*)` - после разрешения
- [ ] Использовать `b2Body::GetUserData()` для получения `entt::entity`

#### 4.3. Система событий коллизий
- [ ] Определить события в `include/simulation/events/CollisionEvents.h`:
  ```cpp
  struct CollisionBeginEvent { entt::entity a; entt::entity b; };
  struct CollisionEndEvent { entt::entity a; entt::entity b; };
  struct TriggerEnterEvent { entt::entity trigger; entt::entity other; };
  struct TriggerExitEvent { entt::entity trigger; entt::entity other; };
  ```
- [ ] Интегрировать `boost::signals2` для отправки событий
- [ ] Обновить `PhysicsContactListener` для отправки событий

#### 4.4. Тесты для коллизий
- [ ] Создать `tests/simulation/CollisionTests.cpp`
- [ ] Тест: BeginContact вызывается при столкновении двух динамических тел
- [ ] Тест: EndContact вызывается при разделении тел
- [ ] Тест: TriggerEnterEvent отправляется для триггерных коллайдеров
- [ ] Тест: Фильтрация коллизий по categoryBits/maskBits работает
- [ ] Интеграционный тест: Объект падает на платформу и останавливается

---

## Задача 5: Отладочная визуализация физики

### Описание
Реализовать визуализацию физических тел, коллайдеров и контактных точек для отладки.

### Подзадачи

#### 5.1. DebugDraw для Box2D
- [ ] Создать класс `PhysicsDebugDraw` в `include/rendering/PhysicsDebugDraw.h`
- [ ] Наследовать от `b2Draw`
- [ ] Переопределить методы:
  - `DrawPolygon(const b2Vec2*, int32, const b2Color&)`
  - `DrawSolidPolygon(const b2Vec2*, int32, const b2Color&)`
  - `DrawCircle(const b2Vec2&, float, const b2Color&)`
  - `DrawSolidCircle(const b2Vec2&, float, const b2Vec2&, const b2Color&)`
  - `DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&)`
  - `DrawPoint(const b2Vec2&, float, const b2Color&)`
- [ ] Использовать `sf::VertexArray` для рендеринга примитивов

#### 5.2. Интеграция с RenderSystem
- [ ] Добавить флаг `bool drawPhysicsDebug` в `RenderSystem`
- [ ] Реализовать `void renderPhysicsDebug(sf::RenderWindow&, PhysicsWorld&)`
- [ ] Добавить горячую клавишу **F7** для переключения debug draw
- [ ] Рендерить после основного слоя спрайтов

#### 5.3. Визуализация дополнительной информации
- [ ] Отрисовка центра масс (зеленая точка)
- [ ] Отрисовка AABB коллайдеров (желтый прямоугольник)
- [ ] Отрисовка контактных точек (красные точки)
- [ ] Отрисовка векторов скорости (синие стрелки)

#### 5.4. Тесты для DebugDraw
- [ ] Создать `tests/rendering/PhysicsDebugDrawTests.cpp`
- [ ] Тест: PhysicsDebugDraw создается без ошибок
- [ ] Тест: Методы Draw* вызываются Box2D (mock тест)
- [ ] Визуальный тест (ручной): Создать сцену с несколькими телами и проверить визуализацию
- [ ] Тест: Переключение F7 работает корректно

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
