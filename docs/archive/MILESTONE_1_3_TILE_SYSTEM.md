# Milestone 1.3: Тайловая система

## Обзор

Тайловая система является основой визуального представления игрового мира. Игра использует вид сверху с перспективой 3/4 (изометрическая проекция без углов), где все объекты располагаются на жёсткой сетке тайлов.

---

## Базовые концепции

### Единица измерения - тайл

- **Размер тайла**: 32x32 пикселя (фиксировано)
- **Координаты**: целочисленные (tileX, tileY)
- **Преобразование**: `pixelPos = tilePos * 32`

```cpp
constexpr int TILE_SIZE = 32;

// Преобразование координат
sf::Vector2i tileToPixel(sf::Vector2i tile) {
    return {tile.x * TILE_SIZE, tile.y * TILE_SIZE};
}

sf::Vector2i pixelToTile(sf::Vector2f pixel) {
    return {
        static_cast<int>(std::floor(pixel.x / TILE_SIZE)),
        static_cast<int>(std::floor(pixel.y / TILE_SIZE))
    };
}
```

### Многотайловые объекты

Объекты могут занимать несколько тайлов, но размер всегда кратен 32:

| Объект | Размер (тайлы) | Размер (пиксели) |
|--------|----------------|------------------|
| Насос | 1x1 | 32x32 |
| Конвейер | 1x1 | 32x32 |
| Резервуар | 2x2 | 64x64 |
| Котёл | 2x3 | 64x96 |
| Станция | 3x3 | 96x96 |

**Anchor point**: левый верхний угол объекта определяет его тайловую позицию.

---

## Система слоёв

Рендеринг происходит послойно, от нижнего к верхнему:

```
┌─────────────────────────┐
│  Layer 4: UI Overlay    │  ← HUD, курсор
├─────────────────────────┤
│  Layer 3: Properties    │  ← Индикаторы, кнопки состояния
├─────────────────────────┤
│  Layer 2: Objects       │  ← Промышленные объекты
├─────────────────────────┤
│  Layer 1: Ground        │  ← Базовая карта (пол, земля)
├─────────────────────────┤
│  Layer 0: Background    │  ← Фон (под картой)
└─────────────────────────┘
```

### Layer 0: Background
- Статичный фон или параллакс
- Не участвует в игровой логике

### Layer 1: Ground (Карта)
- Тайлы пола: бетон, металл, земля, вода
- Загружается из TMX файла
- Определяет проходимость и тип поверхности

### Layer 2: Objects (Объекты)
- Промышленные объекты: насосы, клапаны, конвейеры
- Каждый объект - отдельная ECS entity
- Может иметь анимацию

### Layer 3: Properties (Свойства)
- Визуальные индикаторы состояния объектов
- Накладываются поверх спрайта объекта
- Примеры: кнопки вкл/выкл, индикаторы уровня, стрелки направления

### Layer 4: UI Overlay
- Элементы интерфейса в игровом мире
- Выделение объектов, подсказки

---

## Интеграция с текущей архитектурой

### Существующие компоненты (в Components.h)

Текущая реализация уже содержит базовые компоненты, которые нужно **расширить**, а не заменить:

| Существующий | Использование в тайловой системе |
|-------------|----------------------------------|
| `TransformComponent` | Хранит пиксельные координаты (x, y). Будет синхронизироваться с TilePositionComponent |
| `SpriteComponent` | Уже имеет `layer`, `textureRect`, `visible`. Используем как есть |
| `AnimationComponent` | Базовая структура есть, но система не реализована. Расширим для спрайт-листов |
| `ParentComponent` / `ChildrenComponent` | Используем для связи оверлеев с родительским объектом |

### Что нужно добавить в Components.h

```cpp
// ============== НОВЫЕ КОМПОНЕНТЫ ДЛЯ ТАЙЛОВОЙ СИСТЕМЫ ==============

constexpr int TILE_SIZE = 32;

/**
 * @brief Позиция объекта в тайловых координатах
 * Дополняет TransformComponent, который хранит пиксельные координаты
 */
struct TilePositionComponent {
    int tileX = 0;
    int tileY = 0;
    int widthTiles = 1;
    int heightTiles = 1;

    // Синхронизация с TransformComponent выполняется в TileSystem
};

/**
 * @brief Оверлей состояния объекта (кнопка, индикатор)
 * Создается как отдельная entity с ParentComponent
 */
struct OverlayComponent {
    sf::Vector2f localOffset;  ///< Смещение относительно родителя
    bool syncWithParent = true; ///< Автоматическая синхронизация позиции
};
```

### Адаптация существующего AnimationComponent

Текущий `AnimationComponent` нужно расширить для работы со спрайт-листами:

```cpp
struct AnimationComponent {
    std::string currentAnimation;      // Имя анимации (существует)
    int currentFrame = 0;              // Текущий кадр (существует)
    float frameTime = 0.0f;            // Накопитель времени (существует)
    float frameDelay = 0.1f;           // Задержка между кадрами (существует)
    bool loop = true;                  // Зацикливание (существует)
    bool playing = true;               // Воспроизведение (существует)

    // === НОВЫЕ ПОЛЯ ===
    int frameCount = 1;                ///< Общее количество кадров
    int frameWidth = TILE_SIZE;        ///< Ширина кадра в пикселях
    int frameHeight = TILE_SIZE;       ///< Высота кадра в пикселях

    /// Получить прямоугольник текущего кадра для textureRect
    sf::IntRect getCurrentFrameRect() const {
        return {
            currentFrame * frameWidth,
            0,
            frameWidth,
            frameHeight
        };
    }
};
```

### Использование существующего SpriteComponent.layer

**Не нужен отдельный TileLayerComponent!** Используем существующий `layer` в SpriteComponent:

```cpp
// Константы слоёв (добавить в Components.h)
namespace RenderLayer {
    constexpr int Background = 0;
    constexpr int Ground = 100;
    constexpr int Objects = 200;      // + tileY для Y-sorting
    constexpr int Overlays = 300;     // + tileY для синхронизации с родителем
    constexpr int UIOverlay = 400;
}
```

### Системы для реализации

1. **TilePositionSystem** - синхронизация TilePositionComponent → TransformComponent
2. **AnimationSystem** - обновление кадров анимации (TODO в текущем коде)
3. **OverlaySystem** - синхронизация позиций оверлеев с родителями
4. **TileMapSystem** - загрузка и рендеринг TMX карт

### Существующий RenderSystem

Текущий `RenderSystem` уже:
- Сортирует по `SpriteComponent.layer`
- Кеширует sf::Sprite
- Применяет трансформации из `TransformComponent`

**Изменения не требуются!** Достаточно правильно выставлять `layer` с учётом Y-sorting.

---

## ECS Компоненты

### TilePositionComponent

```cpp
/**
 * @brief Позиция объекта в тайловых координатах
 */
struct TilePositionComponent {
    int tileX;           ///< X координата в тайлах
    int tileY;           ///< Y координата в тайлах
    int widthTiles;      ///< Ширина объекта в тайлах
    int heightTiles;     ///< Высота объекта в тайлах

    /// Получить пиксельную позицию (левый верхний угол)
    sf::Vector2f getPixelPosition() const {
        return {
            static_cast<float>(tileX * TILE_SIZE),
            static_cast<float>(tileY * TILE_SIZE)
        };
    }

    /// Получить центр объекта в пикселях
    sf::Vector2f getCenterPixel() const {
        return {
            (tileX + widthTiles * 0.5f) * TILE_SIZE,
            (tileY + heightTiles * 0.5f) * TILE_SIZE
        };
    }

    /// Проверить, содержит ли объект указанный тайл
    bool containsTile(int x, int y) const {
        return x >= tileX && x < tileX + widthTiles &&
               y >= tileY && y < tileY + heightTiles;
    }
};
```

### TileLayerComponent

```cpp
/**
 * @brief Слой отрисовки для корректного z-ordering
 */
struct TileLayerComponent {
    enum class Layer : uint8_t {
        Background = 0,
        Ground = 1,
        Objects = 2,
        Properties = 3,
        UIOverlay = 4
    };

    Layer layer;
    int subOrder;  ///< Порядок внутри слоя (для перекрытия объектов)

    /// Вычислить общий приоритет отрисовки
    int getRenderPriority() const {
        return static_cast<int>(layer) * 10000 + subOrder;
    }
};
```

### PropertyOverlayComponent

```cpp
/**
 * @brief Визуальные индикаторы состояния объекта
 * Накладываются поверх базового спрайта объекта
 */
struct PropertyOverlayComponent {
    struct Overlay {
        std::string textureId;     ///< ID текстуры в ResourceManager
        sf::Vector2f localOffset;  ///< Смещение относительно объекта
        bool visible;              ///< Видимость индикатора
        sf::Color tint;            ///< Цветовой оттенок
    };

    std::vector<Overlay> overlays;

    /// Добавить индикатор
    void addOverlay(const std::string& texture, sf::Vector2f offset = {0, 0}) {
        overlays.push_back({texture, offset, true, sf::Color::White});
    }

    /// Установить видимость индикатора по индексу
    void setVisible(size_t index, bool visible) {
        if (index < overlays.size()) {
            overlays[index].visible = visible;
        }
    }
};
```

### TileAnimationComponent

```cpp
/**
 * @brief Анимация на основе спрайт-листа
 *
 * Спрайт-лист: горизонтальная полоса кадров
 * Размер: (TILE_SIZE * frameCount) x (TILE_SIZE * heightTiles)
 */
struct TileAnimationComponent {
    std::string spriteSheetId;  ///< ID спрайт-листа в ResourceManager

    int frameCount;             ///< Количество кадров
    int currentFrame;           ///< Текущий кадр
    float frameDuration;        ///< Длительность кадра в секундах
    float elapsedTime;          ///< Прошедшее время

    bool loop;                  ///< Зацикливать анимацию
    bool playing;               ///< Воспроизводится ли сейчас

    /// Получить прямоугольник текущего кадра
    sf::IntRect getCurrentFrameRect(int tileWidth = 1, int tileHeight = 1) const {
        return {
            currentFrame * TILE_SIZE * tileWidth,
            0,
            TILE_SIZE * tileWidth,
            TILE_SIZE * tileHeight
        };
    }

    /// Обновить анимацию
    void update(float deltaTime) {
        if (!playing) return;

        elapsedTime += deltaTime;
        if (elapsedTime >= frameDuration) {
            elapsedTime -= frameDuration;
            currentFrame++;

            if (currentFrame >= frameCount) {
                currentFrame = loop ? 0 : frameCount - 1;
                if (!loop) playing = false;
            }
        }
    }

    /// Запустить анимацию
    void play() {
        playing = true;
        currentFrame = 0;
        elapsedTime = 0;
    }

    /// Остановить анимацию
    void stop() {
        playing = false;
    }
};
```

---

## Системы

### TileRenderSystem

```cpp
/**
 * @brief Система отрисовки тайловых объектов
 */
class TileRenderSystem {
public:
    void update(entt::registry& registry, sf::RenderTarget& target) {
        // Собираем все отрисовываемые сущности
        std::vector<RenderItem> items;

        // Объекты с тайловой позицией и спрайтом
        auto view = registry.view<TilePositionComponent, SpriteComponent, TileLayerComponent>();

        for (auto entity : view) {
            auto& pos = view.get<TilePositionComponent>(entity);
            auto& sprite = view.get<SpriteComponent>(entity);
            auto& layer = view.get<TileLayerComponent>(entity);

            // Вычисляем Y-sorting внутри слоя объектов
            int subOrder = layer.subOrder;
            if (layer.layer == TileLayerComponent::Layer::Objects) {
                // Объекты ниже по Y рисуются раньше (перспектива 3/4)
                subOrder = pos.tileY * 100 + pos.tileX;
            }

            items.push_back({
                entity,
                layer.getRenderPriority() + subOrder,
                pos.getPixelPosition()
            });
        }

        // Сортируем по приоритету
        std::sort(items.begin(), items.end(),
            [](const auto& a, const auto& b) {
                return a.priority < b.priority;
            });

        // Отрисовываем
        for (const auto& item : items) {
            renderEntity(registry, target, item.entity, item.position);
        }
    }

private:
    struct RenderItem {
        entt::entity entity;
        int priority;
        sf::Vector2f position;
    };

    void renderEntity(entt::registry& registry, sf::RenderTarget& target,
                      entt::entity entity, sf::Vector2f position) {
        auto& sprite = registry.get<SpriteComponent>(entity);

        // Обновляем позицию спрайта
        sprite.sprite.setPosition(position);

        // Если есть анимация, обновляем текстурный прямоугольник
        if (auto* anim = registry.try_get<TileAnimationComponent>(entity)) {
            auto& tilePos = registry.get<TilePositionComponent>(entity);
            sprite.sprite.setTextureRect(
                anim->getCurrentFrameRect(tilePos.widthTiles, tilePos.heightTiles)
            );
        }

        // Рисуем основной спрайт
        target.draw(sprite.sprite);

        // Рисуем оверлеи свойств
        if (auto* props = registry.try_get<PropertyOverlayComponent>(entity)) {
            for (const auto& overlay : props->overlays) {
                if (overlay.visible) {
                    renderOverlay(target, overlay, position);
                }
            }
        }
    }

    void renderOverlay(sf::RenderTarget& target,
                       const PropertyOverlayComponent::Overlay& overlay,
                       sf::Vector2f basePosition) {
        // Получить спрайт из ResourceManager
        auto& rm = ResourceManager::getInstance();
        sf::Sprite overlaySprite;
        overlaySprite.setTexture(rm.getTexture(overlay.textureId));
        overlaySprite.setPosition(basePosition + overlay.localOffset);
        overlaySprite.setColor(overlay.tint);
        target.draw(overlaySprite);
    }
};
```

### TileAnimationSystem

```cpp
/**
 * @brief Система обновления анимаций
 */
class TileAnimationSystem {
public:
    void update(entt::registry& registry, float deltaTime) {
        auto view = registry.view<TileAnimationComponent>();

        for (auto entity : view) {
            auto& anim = view.get<TileAnimationComponent>(entity);
            anim.update(deltaTime);
        }
    }
};
```

### TileMapSystem

```cpp
/**
 * @brief Система управления тайловой картой (TMX)
 */
class TileMapSystem {
public:
    bool loadMap(const std::string& tmxPath) {
        tmx::Map map;
        if (!map.load(tmxPath)) {
            spdlog::error("Failed to load TMX map: {}", tmxPath);
            return false;
        }

        m_mapWidth = map.getTileCount().x;
        m_mapHeight = map.getTileCount().y;

        // Загружаем тайлсеты
        for (const auto& tileset : map.getTilesets()) {
            loadTileset(tileset);
        }

        // Обрабатываем слои
        for (const auto& layer : map.getLayers()) {
            if (layer->getType() == tmx::Layer::Type::Tile) {
                processTileLayer(static_cast<const tmx::TileLayer&>(*layer));
            } else if (layer->getType() == tmx::Layer::Type::Object) {
                processObjectLayer(static_cast<const tmx::ObjectGroup&>(*layer));
            }
        }

        return true;
    }

    void render(sf::RenderTarget& target, const sf::View& camera) {
        // Culling: рисуем только видимые тайлы
        sf::FloatRect viewBounds = getViewBounds(camera);

        int startX = std::max(0, static_cast<int>(viewBounds.left / TILE_SIZE));
        int startY = std::max(0, static_cast<int>(viewBounds.top / TILE_SIZE));
        int endX = std::min(m_mapWidth, static_cast<int>((viewBounds.left + viewBounds.width) / TILE_SIZE) + 1);
        int endY = std::min(m_mapHeight, static_cast<int>((viewBounds.top + viewBounds.height) / TILE_SIZE) + 1);

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                renderTile(target, x, y);
            }
        }
    }

private:
    int m_mapWidth;
    int m_mapHeight;
    std::vector<std::vector<int>> m_groundLayer;  // ID тайлов
    // ...
};
```

---

## Пример использования

### Создание насоса с индикаторами состояния

```cpp
entt::entity createPump(entt::registry& registry, int tileX, int tileY) {
    auto entity = registry.create();

    // Тайловая позиция (1x1 тайл)
    registry.emplace<TilePositionComponent>(entity, tileX, tileY, 1, 1);

    // Слой объектов
    registry.emplace<TileLayerComponent>(entity,
        TileLayerComponent::Layer::Objects, 0);

    // Базовый спрайт
    auto& sprite = registry.emplace<SpriteComponent>(entity);
    sprite.textureId = "pump_base";
    sprite.sprite.setTexture(ResourceManager::get().getTexture("pump_base"));

    // Оверлеи состояния (кнопки вкл/выкл)
    auto& props = registry.emplace<PropertyOverlayComponent>(entity);

    // Красная кнопка (выключено) - позиция на спрайте насоса
    props.addOverlay("button_red", {4, 8});

    // Зеленая кнопка (включено) - изначально невидима
    props.addOverlay("button_green", {20, 8});
    props.setVisible(1, false);

    // Компонент состояния насоса
    auto& pump = registry.emplace<PumpComponent>(entity);
    pump.isRunning = false;

    return entity;
}

// Переключение состояния насоса
void togglePump(entt::registry& registry, entt::entity pumpEntity) {
    auto& pump = registry.get<PumpComponent>(pumpEntity);
    auto& props = registry.get<PropertyOverlayComponent>(pumpEntity);

    pump.isRunning = !pump.isRunning;

    // Обновляем видимость индикаторов
    props.setVisible(0, !pump.isRunning);  // Красная кнопка
    props.setVisible(1, pump.isRunning);   // Зеленая кнопка
}
```

### Создание анимированного конвейера

```cpp
entt::entity createConveyor(entt::registry& registry, int tileX, int tileY) {
    auto entity = registry.create();

    registry.emplace<TilePositionComponent>(entity, tileX, tileY, 1, 1);
    registry.emplace<TileLayerComponent>(entity,
        TileLayerComponent::Layer::Objects, 0);

    // Спрайт с анимацией
    auto& sprite = registry.emplace<SpriteComponent>(entity);
    sprite.textureId = "conveyor_sheet";
    sprite.sprite.setTexture(ResourceManager::get().getTexture("conveyor_sheet"));

    // Анимация: 8 кадров, каждый по 32x32
    // Спрайт-лист: 256x32 (32*8 x 32)
    auto& anim = registry.emplace<TileAnimationComponent>(entity);
    anim.spriteSheetId = "conveyor_sheet";
    anim.frameCount = 8;
    anim.currentFrame = 0;
    anim.frameDuration = 0.1f;  // 10 FPS анимация
    anim.loop = true;
    anim.playing = false;  // Запустится при включении

    return entity;
}
```

### Создание многотайлового объекта (резервуар 2x2)

```cpp
entt::entity createTank(entt::registry& registry, int tileX, int tileY) {
    auto entity = registry.create();

    // Занимает 2x2 тайла
    registry.emplace<TilePositionComponent>(entity, tileX, tileY, 2, 2);
    registry.emplace<TileLayerComponent>(entity,
        TileLayerComponent::Layer::Objects, 0);

    auto& sprite = registry.emplace<SpriteComponent>(entity);
    sprite.textureId = "tank_64x64";
    sprite.sprite.setTexture(ResourceManager::get().getTexture("tank_64x64"));

    // Индикатор уровня жидкости (анимированный)
    auto& props = registry.emplace<PropertyOverlayComponent>(entity);
    props.addOverlay("tank_level_0", {16, 16});  // Пустой
    props.addOverlay("tank_level_25", {16, 16});
    props.addOverlay("tank_level_50", {16, 16});
    props.addOverlay("tank_level_75", {16, 16});
    props.addOverlay("tank_level_100", {16, 16}); // Полный

    // Показываем только один уровень
    for (size_t i = 0; i < 5; ++i) {
        props.setVisible(i, false);
    }
    props.setVisible(0, true);  // Начинаем с пустого

    // Компонент резервуара
    auto& tank = registry.emplace<TankComponent>(entity);
    tank.capacity = 1000.0f;
    tank.currentLevel = 0.0f;

    return entity;
}
```

---

## Форматы ассетов

### Спрайты объектов

```
assets/sprites/industrial/
├── pump_base.png        (32x32)   - Базовый спрайт насоса
├── conveyor_sheet.png   (256x32)  - Анимация конвейера (8 кадров)
├── tank_64x64.png       (64x64)   - Резервуар 2x2
└── valve_sheet.png      (128x32)  - Анимация клапана (4 кадра)
```

### Спрайты свойств (оверлеи)

```
assets/sprites/overlays/
├── button_red.png       (8x8 или 16x16)  - Красная кнопка
├── button_green.png     (8x8 или 16x16)  - Зеленая кнопка
├── indicator_on.png     (8x8)            - Индикатор включен
├── indicator_off.png    (8x8)            - Индикатор выключен
├── arrow_up.png         (16x16)          - Стрелка направления
├── tank_level_0.png     (32x48)          - Уровень резервуара 0%
├── tank_level_25.png    (32x48)          - Уровень 25%
└── ...
```

### TMX карты

```
assets/maps/
├── factory_floor.tmx    - Основная карта фабрики
├── tilesets/
│   ├── floor_tiles.tsx  - Тайлсет полов
│   └── walls.tsx        - Тайлсет стен
└── test_map.tmx         - Тестовая карта
```

---

## Оптимизация

### Frustum Culling

Рисуем только тайлы, попадающие в область видимости камеры:

```cpp
bool isTileVisible(int tileX, int tileY, const sf::FloatRect& viewBounds) {
    sf::FloatRect tileBounds(
        tileX * TILE_SIZE,
        tileY * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    );
    return viewBounds.intersects(tileBounds);
}
```

### Texture Atlases

Объединяем мелкие спрайты в атласы для уменьшения draw calls:

```cpp
class TextureAtlas {
public:
    sf::IntRect getRegion(const std::string& name) const;
    const sf::Texture& getTexture() const;

private:
    sf::Texture m_texture;
    std::unordered_map<std::string, sf::IntRect> m_regions;
};
```

### Batching

Группируем отрисовку тайлов с одинаковой текстурой:

```cpp
// Вместо множества draw() вызовов используем VertexArray
sf::VertexArray vertices(sf::Quads);

for (const auto& tile : visibleTiles) {
    appendTileVertices(vertices, tile);
}

target.draw(vertices, &tileset.texture);
```

---

## Интеграция с OPC UA

Состояние объектов синхронизируется с ПЛК через систему привязок:

```cpp
// При изменении OPC UA переменной
void onOpcValueChanged(entt::entity entity, const OpcValue& value) {
    auto& registry = getRegistry();

    if (auto* pump = registry.try_get<PumpComponent>(entity)) {
        if (value.name == "Running") {
            pump->isRunning = value.toBool();

            // Обновляем визуальные индикаторы
            auto& props = registry.get<PropertyOverlayComponent>(entity);
            props.setVisible(0, !pump->isRunning);
            props.setVisible(1, pump->isRunning);

            // Запускаем/останавливаем анимацию
            if (auto* anim = registry.try_get<TileAnimationComponent>(entity)) {
                pump->isRunning ? anim->play() : anim->stop();
            }
        }
    }
}
```

---

## План реализации

### Этап 1: Расширение Components.h

**Файл:** `include/core/Components.h`

Добавить:
- [ ] Константу `TILE_SIZE = 32`
- [ ] Namespace `RenderLayer` с константами слоёв
- [ ] `TilePositionComponent` - тайловые координаты
- [ ] `OverlayComponent` - для индикаторов состояния
- [ ] Расширить `AnimationComponent` полями `frameCount`, `frameWidth`, `frameHeight`

### Этап 2: Новые системы

**Файлы:** `src/core/systems/`, `include/core/systems/`

1. **TilePositionSystem** (новый)
   - Синхронизация `TilePositionComponent` → `TransformComponent`
   - Вычисление `layer` с Y-sorting для перспективы 3/4
   - Файлы: `TilePositionSystem.h`, `TilePositionSystem.cpp`

2. **AnimationSystem** (новый, TODO в текущем коде)
   - Обновление `currentFrame` по времени
   - Установка `textureRect` в `SpriteComponent`
   - Файлы: `AnimationSystem.h`, `AnimationSystem.cpp`

3. **OverlaySystem** (новый)
   - Синхронизация позиции оверлеев с родителем через `ParentComponent`
   - Файлы: `OverlaySystem.h`, `OverlaySystem.cpp`

4. **TileMapSystem** (новый, в модуле Rendering)
   - Интеграция tmxlite
   - Загрузка TMX карт
   - Culling и рендеринг через VertexArray
   - Файлы: `src/rendering/TileMapSystem.cpp`

### Этап 3: Интеграция в GameState

**Файл:** `src/core/states/GameState.cpp`

- [ ] Добавить вызов новых систем в `update()`
- [ ] Создать тестовую сцену с тайловыми объектами
- [ ] Реализовать камеру с перемещением (WASD) и зумом (колесо мыши)

### Этап 4: Тестовые ассеты

**Папки:** `assets/sprites/`, `assets/maps/`

- [ ] Создать тестовые спрайты 32x32 (насос, конвейер)
- [ ] Создать спрайт-лист анимации (256x32)
- [ ] Создать тестовую TMX карту в Tiled

### Порядок реализации файлов

```
1. include/core/Components.h           [расширение]
2. include/core/systems/TilePositionSystem.h
3. src/core/systems/TilePositionSystem.cpp
4. include/core/systems/AnimationSystem.h
5. src/core/systems/AnimationSystem.cpp
6. include/core/systems/OverlaySystem.h
7. src/core/systems/OverlaySystem.cpp
8. src/core/CMakeLists.txt             [добавить новые файлы]
9. src/core/states/GameState.cpp       [интеграция систем]
10. assets/sprites/test_tile.png       [тестовые ассеты]
```

### Зависимости от vcpkg

Убедиться что установлены:
- `tmxlite` - для загрузки TMX карт (Milestone 1.3)

---

*Документ создан: Ноябрь 2024*
*Версия: 1.1 - Обновлён с учётом текущей архитектуры*
