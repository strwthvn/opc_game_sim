# Система анимаций с поддержкой JSON метаданных

## Обзор

Проект теперь поддерживает **два типа анимаций**:

1. **AnimationComponent** (старая система) - простая горизонтальная полоса кадров с одинаковой длительностью
2. **AnimationComponentV2** (новая система) - полная поддержка JSON метаданных с множественными анимациями

## Новый компонент: AnimationComponentV2

### Основные возможности

- ✅ **Несколько именованных анимаций** на одном спрайте ("idle", "running", "jump")
- ✅ **Разная длительность каждого кадра** анимации
- ✅ **Произвольное расположение кадров** в текстуре (не только горизонтально)
- ✅ **Загрузка из JSON метаданных** (SpriteMetadata)
- ✅ **Shared AnimationData** - несколько сущностей могут использовать одни данные анимаций

## Структура данных

### AnimationFrameData

Один кадр анимации:

```cpp
struct AnimationFrameData {
    sf::IntRect textureRect;    // Прямоугольник в текстуре
    sf::Time duration;          // Длительность кадра
};
```

### AnimationDefinition

Определение одной анимации:

```cpp
struct AnimationDefinition {
    std::string name;                       // Имя ("idle", "running")
    std::vector<AnimationFrameData> frames; // Кадры
    bool loop;                              // Зацикливать?
};
```

### AnimationData

Менеджер всех анимаций для спрайта:

```cpp
class AnimationData {
    // Создание из SpriteMetadata
    static std::shared_ptr<AnimationData> createFromMetadata(const SpriteMetadata& metadata);

    // Получение анимации по имени
    const AnimationDefinition* getAnimation(const std::string& name) const;
};
```

### AnimationComponentV2

Компонент для сущности:

```cpp
struct AnimationComponentV2 {
    std::shared_ptr<AnimationData> animationData; // Данные анимаций
    std::string currentAnimation;                 // Текущая анимация
    int currentFrame;                             // Текущий кадр
    sf::Time elapsedTime;                         // Время с начала кадра
    bool playing;                                 // Воспроизводится?

    // Переключение анимации
    bool setAnimation(const std::string& name, bool restart = true);
};
```

## Примеры использования

### Пример 1: Загрузка из JSON метаданных

```cpp
#include "core/ResourceManager.h"
#include "core/AnimationData.h"
#include "core/Components.h"
#include <entt/entt.hpp>

// Загружаем метаданные спрайта
core::ResourceManager resourceManager;
const auto* metadata = resourceManager.loadSpriteMetadata(
    "assets/sprites/TEST/testObjAnimation.sprite.json"
);

if (metadata) {
    // Создаем AnimationData из метаданных
    auto animData = core::AnimationData::createFromMetadata(*metadata);

    // Создаем сущность
    entt::registry registry;
    auto entity = registry.create();

    // Добавляем компоненты
    auto& sprite = registry.emplace<core::SpriteComponent>(entity);
    sprite.textureName = "assets/sprites/TEST/" + metadata->getTexturePath();

    // Добавляем AnimationComponentV2
    auto& anim = registry.emplace<core::AnimationComponentV2>(
        entity,
        animData,
        "idle"  // Начальная анимация
    );

    LOG_INFO("Created animated entity with {} animations",
             animData->getAnimationCount());
}
```

### Пример 2: Переключение анимаций

```cpp
// Получаем компонент
auto& anim = registry.get<core::AnimationComponentV2>(entity);

// Переключаемся на анимацию "running"
if (anim.setAnimation("running")) {
    LOG_INFO("Switched to running animation");
}

// Переключаемся на "jump" без перезапуска
anim.setAnimation("jump", false);

// Останавливаем анимацию
anim.playing = false;

// Возобновляем
anim.playing = true;
```

### Пример 3: Создание анимаций вручную

```cpp
#include "core/AnimationData.h"

// Создаем AnimationData
auto animData = std::make_shared<core::AnimationData>();

// Добавляем анимацию "idle"
core::AnimationDefinition idleAnim("idle");
idleAnim.loop = false;
idleAnim.frames.push_back(core::AnimationFrameData(
    sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32)),
    sf::Time::Zero  // Статичный кадр
));
animData->addAnimation(idleAnim);

// Добавляем анимацию "running"
core::AnimationDefinition runAnim("running");
runAnim.loop = true;

for (int i = 0; i < 4; i++) {
    runAnim.frames.push_back(core::AnimationFrameData(
        sf::IntRect(sf::Vector2i(i * 32, 0), sf::Vector2i(32, 32)),
        sf::milliseconds(100)
    ));
}
animData->addAnimation(runAnim);

// Используем с компонентом
auto& anim = registry.emplace<core::AnimationComponentV2>(entity, animData, "idle");
```

### Пример 4: Несколько сущностей с одними данными

```cpp
// Загружаем метаданные один раз
auto animData = core::AnimationData::createFromMetadata(*metadata);

// Создаем несколько сущностей
for (int i = 0; i < 10; i++) {
    auto entity = registry.create();

    // Все сущности используют ОДНИ и те же данные анимаций
    registry.emplace<core::AnimationComponentV2>(entity, animData, "idle");
}

// Экономия памяти: данные анимаций хранятся в одном экземпляре
```

### Пример 5: Проверка текущего кадра

```cpp
auto& anim = registry.get<core::AnimationComponentV2>(entity);

// Получаем текущее определение анимации
const auto* animDef = anim.getCurrentAnimationDef();
if (animDef) {
    LOG_INFO("Current animation: {}, frame {}/{}",
             animDef->name,
             anim.currentFrame,
             animDef->frames.size());
}

// Получаем текущий кадр
const auto* frame = anim.getCurrentFrame();
if (frame) {
    sf::IntRect rect = frame->textureRect;
    sf::Time duration = frame->duration;

    LOG_DEBUG("Frame rect: ({}, {}), size: ({}, {}), duration: {}ms",
              rect.position.x, rect.position.y,
              rect.size.x, rect.size.y,
              duration.asMilliseconds());
}
```

## Интеграция с AnimationSystem

**ВАЖНО:** Текущий `AnimationSystem` работает **только** со старым `AnimationComponent`. Для работы с `AnimationComponentV2` нужно создать новую систему `AnimationSystemV2`.

### Пример AnimationSystemV2 (псевдокод)

```cpp
class AnimationSystemV2 : public ISystem {
public:
    void update(entt::registry& registry, double dt) override {
        auto view = registry.view<AnimationComponentV2, SpriteComponent>();

        for (auto entity : view) {
            auto& anim = view.get<AnimationComponentV2>(entity);
            auto& sprite = view.get<SpriteComponent>(entity);

            if (!anim.playing || !anim.hasAnimationData()) {
                continue;
            }

            const auto* animDef = anim.getCurrentAnimationDef();
            if (!animDef || animDef->frames.empty()) {
                continue;
            }

            // Обновляем время
            anim.elapsedTime += sf::seconds(static_cast<float>(dt));

            // Получаем текущий кадр
            const auto& currentFrame = animDef->frames[anim.currentFrame];

            // Проверяем, нужно ли переключить кадр
            if (anim.elapsedTime >= currentFrame.duration) {
                anim.elapsedTime -= currentFrame.duration;
                anim.currentFrame++;

                // Зацикливание или остановка
                if (anim.currentFrame >= static_cast<int>(animDef->frames.size())) {
                    if (animDef->loop) {
                        anim.currentFrame = 0;
                    } else {
                        anim.currentFrame = animDef->frames.size() - 1;
                        anim.playing = false;
                    }
                }
            }

            // Обновляем textureRect в SpriteComponent
            const auto& frame = animDef->frames[anim.currentFrame];
            sprite.textureRect = frame.textureRect;
        }
    }
};
```

## Сравнение старой и новой системы

| Функция | AnimationComponent | AnimationComponentV2 |
|---------|-------------------|---------------------|
| Несколько анимаций | ❌ Нет | ✅ Да |
| Разная длительность кадров | ❌ Нет | ✅ Да |
| Произвольное расположение | ❌ Только горизонтально | ✅ Да |
| Загрузка из JSON | ❌ Нет | ✅ Да |
| Shared данные | ❌ Нет | ✅ Да |
| Простота использования | ✅ Проще | Сложнее |

## Миграция со старой на новую систему

### Было (AnimationComponent):

```cpp
auto& anim = registry.emplace<AnimationComponent>(entity);
anim.frameCount = 4;
anim.frameWidth = 32;
anim.frameHeight = 32;
anim.frameDelay = 0.1f;
anim.loop = true;
anim.playing = true;
```

### Стало (AnimationComponentV2):

```cpp
// Создаем данные анимации
auto animData = std::make_shared<AnimationData>();
auto& runAnim = animData->addAnimation("running", true);

for (int i = 0; i < 4; i++) {
    runAnim.frames.push_back(AnimationFrameData(
        sf::IntRect(sf::Vector2i(i * 32, 0), sf::Vector2i(32, 32)),
        sf::milliseconds(100)
    ));
}

// Создаем компонент
auto& anim = registry.emplace<AnimationComponentV2>(entity, animData, "running");
```

## Best Practices

1. **Используйте SpriteMetadata** - создавайте `.sprite.json` файлы вместо ручного создания анимаций
2. **Переиспользуйте AnimationData** - создавайте один экземпляр для всех сущностей одного типа
3. **Именуйте анимации логично** - "idle", "running", "jump", а не "anim1", "anim2"
4. **Проверяйте наличие анимации** - используйте `hasAnimationData()` и `getCurrentAnimationDef()` перед использованием
5. **Обрабатывайте переходы** - переключайте анимации плавно, учитывая текущее состояние

## API Reference

См. также:
- [AnimationData.h](../include/core/AnimationData.h) - API для работы с данными анимаций
- [Components.h](../include/core/Components.h) - Определение AnimationComponentV2
- [SPRITE_METADATA.md](SPRITE_METADATA.md) - Формат JSON метаданных

## Примеры из кодовой базы

После реализации AnimationSystemV2, примеры будут доступны в:
- `src/core/systems/AnimationSystemV2.cpp` - Реализация системы
- `tests/unit/test_animation_system_v2.cpp` - Unit тесты
- `src/core/states/GameState.cpp` - Использование в игре
