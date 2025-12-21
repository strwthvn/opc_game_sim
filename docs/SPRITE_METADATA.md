# Sprite Metadata System

## Обзор

Система метаданных спрайтов позволяет хранить информацию о спрайтах в структурированном JSON формате. Каждый спрайт может иметь метаданные об авторе, версии, анимациях и другие параметры.

## Формат файла `.sprite.json`

Файлы метаданных спрайтов используют расширение `.sprite.json` и хранятся рядом с PNG текстурами.

### Базовая структура

```json
{
  "sprite": {
    "name": "sprite_name",
    "author": "Author Name",
    "version": "1.0",
    "created": "2025-12-20",
    "description": "Sprite description",
    "texture": "sprite_texture.png",
    "size": {
      "width": 32,
      "height": 32
    },
    "origin": {
      "x": 0,
      "y": 32
    },
    "tags": ["tag1", "tag2"],
    "animations": [...]
  }
}
```

### Поля метаданных

| Поле | Тип | Обязательное | Описание |
|------|-----|--------------|----------|
| `name` | string | Да | Уникальное имя спрайта |
| `author` | string | Нет | Автор спрайта |
| `version` | string | Нет | Версия спрайта (по умолчанию "1.0") |
| `created` | string | Нет | Дата создания (ISO 8601 формат) |
| `description` | string | Нет | Описание спрайта |
| `texture` | string | Да | Путь к PNG файлу (относительно .sprite.json) |
| `size` | object | Нет | Размер спрайта (по умолчанию 32x32) |
| `origin` | object | Нет | Точка origin (по умолчанию bottom-left: 0, 32) |
| `tags` | array | Нет | Теги для категоризации |
| `animations` | array | Нет | Массив анимаций |

### Структура анимации

```json
{
  "name": "animation_name",
  "loop": true,
  "frames": [
    {
      "x": 0,
      "y": 0,
      "duration": 100
    },
    {
      "x": 32,
      "y": 0,
      "duration": 100
    }
  ]
}
```

#### Поля анимации

| Поле | Тип | Описание |
|------|-----|----------|
| `name` | string | Имя анимации (например, "idle", "running") |
| `loop` | boolean | Зацикливать анимацию |
| `frames` | array | Массив кадров анимации |

#### Поля кадра

| Поле | Тип | Описание |
|------|-----|----------|
| `x` | int | X координата кадра в текстуре (пиксели) |
| `y` | int | Y координата кадра в текстуре (пиксели) |
| `duration` | int | Длительность кадра (миллисекунды) |

## Примеры использования

### Пример 1: Простой статичный спрайт

```json
{
  "sprite": {
    "name": "test_floor",
    "author": "OPC Game Sim Team",
    "version": "1.0",
    "texture": "testFloor1.png",
    "size": {
      "width": 32,
      "height": 32
    },
    "tags": ["tile", "floor"],
    "animations": [
      {
        "name": "idle",
        "loop": false,
        "frames": [
          {"x": 0, "y": 0, "duration": 0}
        ]
      }
    ]
  }
}
```

### Пример 2: Анимированный промышленный объект

```json
{
  "sprite": {
    "name": "conveyor_belt",
    "author": "OPC Game Sim Team",
    "version": "1.0",
    "description": "Industrial conveyor belt with running animation",
    "texture": "conveyor_belt.png",
    "size": {
      "width": 32,
      "height": 32
    },
    "tags": ["industrial", "transport", "animated"],
    "animations": [
      {
        "name": "idle",
        "loop": false,
        "frames": [
          {"x": 0, "y": 0, "duration": 0}
        ]
      },
      {
        "name": "running",
        "loop": true,
        "frames": [
          {"x": 0, "y": 0, "duration": 100},
          {"x": 32, "y": 0, "duration": 100},
          {"x": 64, "y": 0, "duration": 100},
          {"x": 96, "y": 0, "duration": 100}
        ]
      },
      {
        "name": "slow",
        "loop": true,
        "frames": [
          {"x": 0, "y": 0, "duration": 200},
          {"x": 32, "y": 0, "duration": 200},
          {"x": 64, "y": 0, "duration": 200},
          {"x": 96, "y": 0, "duration": 200}
        ]
      }
    ]
  }
}
```

## Использование в коде

### Загрузка метаданных через ResourceManager

```cpp
#include "core/ResourceManager.h"

// Создаем ResourceManager
core::ResourceManager resourceManager;

// Загружаем метаданные спрайта
const auto* metadata = resourceManager.loadSpriteMetadata(
    "assets/sprites/industrial/conveyor_belt.sprite.json"
);

if (metadata) {
    // Получаем информацию о спрайте
    std::string name = metadata->getName();
    std::string texturePath = metadata->getTexturePath();

    // Получаем анимацию
    const auto* runningAnim = metadata->getAnimation("running");
    if (runningAnim) {
        bool loops = runningAnim->loop;
        size_t frameCount = runningAnim->frames.size();
    }
}
```

### Прямая загрузка из файла

```cpp
#include "core/SpriteMetadata.h"

// Загружаем метаданные напрямую
auto metadataOpt = core::SpriteMetadata::loadFromFile(
    "assets/sprites/TEST/testObj.sprite.json"
);

if (metadataOpt.has_value()) {
    const auto& metadata = metadataOpt.value();

    // Проверяем теги
    if (metadata.hasTag("industrial")) {
        // Это промышленный объект
    }

    // Получаем все анимации
    const auto& animations = metadata.getAnimations();
    for (const auto& anim : animations) {
        std::cout << "Animation: " << anim.name << std::endl;
    }
}
```

### Создание метаданных программно

```cpp
#include "core/SpriteMetadata.h"

// Создаем новый спрайт
core::SpriteMetadata metadata;
metadata.setName("my_sprite");
metadata.setAuthor("Developer");
metadata.setTexturePath("my_sprite.png");

// Устанавливаем размер
core::SpriteSize size{64, 64};
metadata.setSize(size);

// Добавляем теги
metadata.addTag("custom");
metadata.addTag("test");

// Создаем анимацию
core::AnimationInfo anim;
anim.name = "blink";
anim.loop = true;
anim.frames.push_back({0, 0, 200});
anim.frames.push_back({64, 0, 200});

metadata.addAnimation(anim);

// Сохраняем в файл
metadata.saveToFile("assets/sprites/custom/my_sprite.sprite.json");
```

### Асинхронная загрузка

```cpp
// Запускаем асинхронную загрузку
auto future = resourceManager.loadSpriteMetadataAsync(
    "assets/sprites/industrial/conveyor_belt.sprite.json"
);

// Продолжаем работу...

// Проверяем готовность
if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    bool success = future.get();
    if (success) {
        const auto* metadata = resourceManager.getSpriteMetadata("conveyor_belt");
        // Используем метаданные
    }
}
```

### Получение списка всех загруженных спрайтов

```cpp
// Получаем список имен всех загруженных спрайтов
std::vector<std::string> spriteNames = resourceManager.getSpriteNames();

for (const auto& name : spriteNames) {
    const auto* metadata = resourceManager.getSpriteMetadata(name);
    if (metadata) {
        std::cout << "Sprite: " << name
                  << " (texture: " << metadata->getTexturePath() << ")"
                  << std::endl;
    }
}
```

## Интеграция с AnimationComponent

Метаданные спрайтов можно использовать для настройки AnimationComponent:

```cpp
#include "core/Components.h"
#include "core/ResourceManager.h"
#include <entt/entt.hpp>

entt::registry registry;
core::ResourceManager resourceManager;

// Загружаем метаданные
const auto* metadata = resourceManager.loadSpriteMetadata(
    "assets/sprites/TEST/testObjAnimation.sprite.json"
);

if (metadata) {
    // Создаем сущность
    auto entity = registry.create();

    // Добавляем SpriteComponent
    auto& sprite = registry.emplace<core::SpriteComponent>(entity);
    sprite.textureName = metadata->getTexturePath();

    // Настраиваем AnimationComponent из метаданных
    auto& animation = registry.emplace<core::AnimationComponent>(entity);

    const auto* runningAnim = metadata->getAnimation("running");
    if (runningAnim) {
        animation.currentAnimation = "running";
        animation.loop = runningAnim->loop;

        // Конвертируем кадры анимации
        for (const auto& frame : runningAnim->frames) {
            core::AnimationFrame animFrame;
            animFrame.textureRect = sf::IntRect(
                frame.x, frame.y,
                metadata->getSize().width,
                metadata->getSize().height
            );
            animFrame.duration = sf::milliseconds(frame.duration);
            animation.frames.push_back(animFrame);
        }
    }
}
```

## Структура директорий

Рекомендуемая организация файлов спрайтов:

```
assets/sprites/
├── industrial/
│   ├── conveyor_belt.png
│   ├── conveyor_belt.sprite.json
│   ├── valve.png
│   └── valve.sprite.json
├── ui/
│   ├── button.png
│   └── button.sprite.json
└── TEST/
    ├── testObj.png
    ├── testObj.sprite.json
    ├── testObjAnimation.png
    └── testObjAnimation.sprite.json
```

## Best Practices

1. **Именование файлов**: Используйте расширение `.sprite.json` для метаданных
2. **Относительные пути**: Путь к текстуре в поле `texture` должен быть относительным (относительно .sprite.json)
3. **Теги**: Используйте теги для категоризации спрайтов ("industrial", "ui", "animated")
4. **Анимация idle**: Всегда создавайте анимацию "idle" для статичных спрайтов
5. **Origin**: Для объектов, стоящих на земле, используйте bottom-left origin (0, height)
6. **Версионирование**: Обновляйте версию при изменении метаданных

## API Reference

### SpriteMetadata

#### Основные методы
- `static std::optional<SpriteMetadata> loadFromFile(const std::string& path)`
- `static std::optional<SpriteMetadata> loadFromJson(const nlohmann::json& json)`
- `bool saveToFile(const std::string& path) const`
- `nlohmann::json toJson() const`

#### Геттеры
- `const std::string& getName() const`
- `const std::string& getTexturePath() const`
- `const SpriteSize& getSize() const`
- `const SpriteOrigin& getOrigin() const`
- `const AnimationInfo* getAnimation(const std::string& name) const`
- `bool hasAnimation(const std::string& name) const`
- `bool hasTag(const std::string& tag) const`

### ResourceManager

#### Методы работы со спрайтами
- `const SpriteMetadata* loadSpriteMetadata(const std::string& path)`
- `const SpriteMetadata* getSpriteMetadata(const std::string& name) const`
- `bool hasSpriteMetadata(const std::string& name) const`
- `bool unloadSpriteMetadata(const std::string& name)`
- `std::future<bool> loadSpriteMetadataAsync(const std::string& path)`
- `std::vector<std::string> getSpriteNames() const`
- `size_t getSpriteMetadataCount() const`

## Примеры из кодовой базы

- [testObj.sprite.json](../assets/sprites/TEST/testObj.sprite.json) - Простой статичный спрайт
- [testObjAnimation.sprite.json](../assets/sprites/TEST/testObjAnimation.sprite.json) - Анимированный спрайт с несколькими анимациями
- [conveyor_belt.sprite.json](../assets/sprites/industrial/conveyor_belt.sprite.json) - Промышленный объект с разными скоростями анимации

## См. также

- [CLAUDE.md](../CLAUDE.md) - Общая документация проекта
- [AnimationSystem](../include/core/systems/AnimationSystem.h) - Система анимаций
- [ResourceManager](../include/core/ResourceManager.h) - Менеджер ресурсов
