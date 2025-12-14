#include "core/states/GameState.h"
#include "core/states/PauseState.h"
#include "core/states/MenuState.h"
#include "core/StateManager.h"
#include "core/InputManager.h"
#include "core/ResourceManager.h"
#include "core/systems/RenderSystem.h"
#include "core/systems/UpdateSystem.h"
#include "core/systems/LifetimeSystem.h"
#include "core/systems/CollisionSystem.h"
#include "core/systems/FSMSystem.h"
#include "core/systems/TilePositionSystem.h"
#include "core/systems/AnimationSystem.h"
#include "core/systems/OverlaySystem.h"
#include "rendering/TileMapSystem.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace core {

GameState::GameState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_fontLoaded(false)
    , m_elapsedTime(0.0)
    , m_updateCount(0)
    , m_cameraZoom(CAMERA_DEFAULT_ZOOM)
    , m_debugDrawGrid(true) {  // Включаем отладочную сетку по умолчанию
}

GameState::~GameState() = default;

void GameState::onEnter() {
    LOG_INFO("Entering GameState");

    // Инициализируем Views
    auto windowSize = getWindowSize();

    // World View - расширяется с размером окна (показывает больше мира)
    m_worldView.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    // Центрируем камеру на тайловой сетке (12x8 тайлов = 384x256 пикселей)
    m_worldView.setCenter(sf::Vector2f(192.0f, 128.0f));  // Центр тайловой сетки
    LOG_DEBUG("GameState World View initialized: {}x{}, centered at tile grid", windowSize.x, windowSize.y);

    // UI View - 1:1 с пикселями окна для четкого текста
    m_uiView.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    m_uiView.setCenter(sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f));
    LOG_DEBUG("GameState UI View initialized: {}x{} (window size)", windowSize.x, windowSize.y);

    initializeScene();
}

void GameState::onExit() {
    LOG_INFO("Exiting GameState");
}

void GameState::onWindowResize(const sf::Vector2u& newSize) {
    LOG_DEBUG("GameState received window resize event: {}x{}", newSize.x, newSize.y);

    // Обновляем World View - он расширяется, показывая больше мира
    m_worldView.setSize(sf::Vector2f(newSize.x, newSize.y));
    m_worldView.setCenter(sf::Vector2f(newSize.x / 2.0f, newSize.y / 2.0f));
    LOG_DEBUG("World View updated to: {}x{}", newSize.x, newSize.y);

    // Обновляем UI View - 1:1 с пикселями окна для четкого текста
    m_uiView.setSize(sf::Vector2f(newSize.x, newSize.y));
    m_uiView.setCenter(sf::Vector2f(newSize.x / 2.0f, newSize.y / 2.0f));
    LOG_DEBUG("UI View updated to: {}x{}", newSize.x, newSize.y);
}

bool GameState::handleEvent(const sf::Event& event) {
    // Обработка нажатий клавиш (SFML 3 API)
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // F6 - переключение отладочной сетки
        if (keyPressed->code == sf::Keyboard::Key::F6) {
            m_debugDrawGrid = !m_debugDrawGrid;
            LOG_INFO("Debug grid: {}", m_debugDrawGrid ? "ON" : "OFF");
            return true;
        }
    }

    // Обработка зума колесом мыши (SFML 3 API)
    if (const auto* wheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
            // Зум in/out
            float delta = wheelScrolled->delta;
            m_cameraZoom -= delta * CAMERA_ZOOM_SPEED;

            // Ограничиваем зум
            m_cameraZoom = std::clamp(m_cameraZoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);

            LOG_DEBUG("Camera zoom: {:.2f}", m_cameraZoom);
            return true;  // Событие обработано
        }
    }

    // Обработка кликов мыши, если нужно
    // TODO: Обработка кликов мыши для выделения объектов

    return false;  // Не блокируем события
}

void GameState::update(double dt) {
    m_elapsedTime += dt;
    m_updateCount++;

    // Обработка ввода через InputManager
    auto* input = getInputManager();
    if (input) {
        // Пауза (только что нажата клавиша)
        if (input->isKeyJustPressed(sf::Keyboard::Key::Escape) ||
            input->isKeyJustPressed(sf::Keyboard::Key::P)) {
            LOG_INFO("Opening pause menu");
            m_stateManager->pushState(std::make_unique<PauseState>(m_stateManager));
        }

        // Быстрый выход в меню (для отладки)
        if (input->isKeyJustPressed(sf::Keyboard::Key::Q)) {
            LOG_INFO("Quick exit to menu");
            m_stateManager->changeState(std::make_unique<MenuState>(m_stateManager));
        }

        // Перемещение камеры (WASD)
        sf::Vector2f cameraMove(0.0f, 0.0f);
        float moveSpeed = CAMERA_MOVE_SPEED * static_cast<float>(dt);

        // Учитываем зум при перемещении (больший зум = медленнее перемещение)
        moveSpeed *= m_cameraZoom;

        if (input->isKeyPressed(sf::Keyboard::Key::W) || input->isKeyPressed(sf::Keyboard::Key::Up)) {
            cameraMove.y -= moveSpeed;
        }
        if (input->isKeyPressed(sf::Keyboard::Key::S) || input->isKeyPressed(sf::Keyboard::Key::Down)) {
            cameraMove.y += moveSpeed;
        }
        if (input->isKeyPressed(sf::Keyboard::Key::A) || input->isKeyPressed(sf::Keyboard::Key::Left)) {
            cameraMove.x -= moveSpeed;
        }
        if (input->isKeyPressed(sf::Keyboard::Key::D) || input->isKeyPressed(sf::Keyboard::Key::Right)) {
            cameraMove.x += moveSpeed;
        }

        // Применяем перемещение камеры
        if (cameraMove.x != 0.0f || cameraMove.y != 0.0f) {
            m_worldView.move(cameraMove);
        }

        // Применяем зум (через zoom() изменяется размер view)
        // Устанавливаем размер view в соответствии с зумом
        auto windowSize = getWindowSize();
        m_worldView.setSize(sf::Vector2f(
            windowSize.x * m_cameraZoom,
            windowSize.y * m_cameraZoom
        ));
    }

    // Вычисляем bounds камеры для RenderSystem (frustum culling)
    if (m_renderSystem) {
        sf::FloatRect viewBounds = sf::FloatRect(
            m_worldView.getCenter() - m_worldView.getSize() / 2.0f,
            m_worldView.getSize()
        );
        m_renderSystem->setViewBounds(viewBounds);
    }

    // Обновление ECS систем
    if (m_updateSystem) {
        m_updateSystem->update(m_registry, dt);
    }

    // Обновление системы времени жизни
    if (m_lifetimeSystem) {
        m_lifetimeSystem->update(m_registry, dt);
    }

    // Обновление системы коллизий (приоритет 100)
    if (m_collisionSystem) {
        m_collisionSystem->update(m_registry, dt);
    }

    // Обновление системы FSM (приоритет 150)
    if (m_fsmSystem) {
        m_fsmSystem->update(m_registry, dt);
    }

    // === ДЕМОНСТРАЦИЯ FSM: Автоматическое переключение состояний лампы ===
    // Переключаем состояние лампы на основе времени в текущем состоянии
    auto fsmView = m_registry.view<EntityStateComponent, NameComponent>();
    for (auto entity : fsmView) {
        auto& fsm = fsmView.get<EntityStateComponent>(entity);
        auto& name = fsmView.get<NameComponent>(entity);

        // Обрабатываем только нашу тестовую лампу
        if (name.name == "Lamp_FSM") {
            // off → on после 3 секунд
            if (fsm.isInState("off") && fsm.timeInState > 3.0f) {
                fsm.setState("on");
            }
            // on → broken после 3 секунд работы
            else if (fsm.isInState("on") && fsm.timeInState > 3.0f) {
                fsm.setState("broken");
            }
            // broken - конечное состояние, лампа "сломана"
        }
    }

    // Обновление Tile Systems (Milestone 1.3)
    if (m_tilePositionSystem) {
        m_tilePositionSystem->update(m_registry);
        // TilePositionSystem обновляет слои объектов для Y-sorting
        // Помечаем слои как измененные для пересортировки в RenderSystem
        if (m_renderSystem) {
            m_renderSystem->markLayersDirty();
        }
    }

    if (m_animationSystem) {
        m_animationSystem->update(m_registry, dt);
    }

    if (m_overlaySystem) {
        m_overlaySystem->update(m_registry);
    }

    // Обновление RenderSystem (подготовка данных для рендеринга)
    if (m_renderSystem) {
        m_renderSystem->update(m_registry, dt);
    }

    // TODO: Обновление физики
    // TODO: Обновление OPC UA привязок

    // Обновляем информационный текст
    if (m_infoText) {
        std::ostringstream oss;
        oss << "GAME STATE - RUNNING (ECS Active)\n";
        oss << "Time: " << std::fixed << std::setprecision(1) << m_elapsedTime << "s\n";
        oss << "Updates: " << m_updateCount << "\n";
        oss << "Entities: " << m_registry.storage<entt::entity>().size() << "\n";
        oss << "Camera Zoom: " << std::fixed << std::setprecision(2) << m_cameraZoom << "x\n";
        oss << "\nControls:\n";
        oss << "WASD/Arrows - Move Camera\n";
        oss << "Mouse Wheel - Zoom\n";
        oss << "ESC/P - Pause\n";
        oss << "Q - Back to Menu";

        m_infoText->setString(oss.str());
    }
}

void GameState::render(sf::RenderWindow& window) {
    // Устанавливаем World View для рендеринга игровых объектов
    window.setView(m_worldView);

    // Рендеринг тайловой карты (если загружена)
    if (m_tileMapSystem && m_tileMapSystem->isLoaded()) {
        m_tileMapSystem->render(window, m_worldView);
    }

    // Рендеринг ECS entities (игровые объекты)
    // Данные уже подготовлены в update(), просто рисуем
    if (m_renderSystem) {
        m_renderSystem->render(window);
    }

    // Отладочная визуализация тайловой сетки
    if (m_debugDrawGrid) {
        drawDebugGrid(window);
    }

    // Переключаемся на UI View для рендеринга интерфейса
    window.setView(m_uiView);

    // Не рендерим текст если шрифт не загрузился
    if (!m_fontLoaded) {
        return;
    }

    // Рисуем информационный текст (в координатах UI View)
    if (m_infoText) {
        window.draw(*m_infoText);
    }
}

void GameState::initializeScene() {
    // Получаем шрифт из ResourceManager
    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available in GameState");
        return;
    }

    try {
        m_font = &resources->getFont("default");
        m_fontLoaded = true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load default font for GameState: {}", e.what());
        m_fontLoaded = false;
        return; // Не инициализируем текст если шрифт не загрузился
    }

    // Настройка информационного текста
    m_infoText = std::make_unique<sf::Text>(*m_font);
    m_infoText->setCharacterSize(INFO_TEXT_FONT_SIZE);
    m_infoText->setFillColor(sf::Color::White);
    m_infoText->setPosition(sf::Vector2f(INFO_TEXT_X, INFO_TEXT_Y));

    // Инициализация ECS систем
    LOG_INFO("Initializing ECS systems");
    m_renderSystem = std::make_unique<RenderSystem>(resources);
    m_updateSystem = std::make_unique<UpdateSystem>();
    m_lifetimeSystem = std::make_unique<LifetimeSystem>();
    m_collisionSystem = std::make_unique<CollisionSystem>();
    m_fsmSystem = std::make_unique<FSMSystem>();

    // Инициализация Tile Systems (Milestone 1.3)
    LOG_INFO("Initializing Tile Systems (Milestone 1.3)");
    m_tilePositionSystem = std::make_unique<TilePositionSystem>();
    m_animationSystem = std::make_unique<AnimationSystem>();
    m_overlaySystem = std::make_unique<OverlaySystem>();
    m_tileMapSystem = std::make_unique<rendering::TileMapSystem>();

    // Создание программной тестовой текстуры (простой красный квадрат)
    LOG_INFO("Creating test texture");
    sf::Image testImage(sf::Vector2u(64, 64), sf::Color::Red);

    // Добавляем белую границу
    for (unsigned int x = 0; x < 64; ++x) {
        testImage.setPixel(sf::Vector2u(x, 0), sf::Color::White);
        testImage.setPixel(sf::Vector2u(x, 63), sf::Color::White);
    }
    for (unsigned int y = 0; y < 64; ++y) {
        testImage.setPixel(sf::Vector2u(0, y), sf::Color::White);
        testImage.setPixel(sf::Vector2u(63, y), sf::Color::White);
    }

    // Загружаем текстуру в ResourceManager
    if (resources->loadTextureFromImage("test_square", testImage)) {
        LOG_INFO("Test texture created and loaded into ResourceManager");
    } else {
        LOG_ERROR("Failed to create test texture");
    }

    // Старые тестовые объекты закомментированы - используем тайловую систему
    /*
    // Создание тестовых сущностей для демонстрации ECS
    LOG_INFO("Creating test entities");

    // Создаем несколько тестовых сущностей
    for (int i = 0; i < 3; ++i) {
        auto entity = m_registry.create();

        // Добавляем имя для отладки
        m_registry.emplace<NameComponent>(entity, "RotatingSquare_" + std::to_string(i));

        // Добавляем тег
        m_registry.emplace<TagComponent>(entity, "test_object");

        // Добавляем компонент трансформации
        auto& transform = m_registry.emplace<TransformComponent>(entity);
        transform.x = 200.0f + i * 150.0f;
        transform.y = 300.0f;
        transform.scaleX = 1.0f + i * 0.5f;
        transform.scaleY = 1.0f + i * 0.5f;

        // Добавляем компонент спрайта с тестовой текстурой
        auto& sprite = m_registry.emplace<SpriteComponent>(entity);
        sprite.textureName = "test_square";  // Используем тестовую текстуру
        sprite.color = sf::Color(255, 100 + i * 50, 100 + i * 50);
        sprite.layer = i;
        sprite.visible = true;

        // Добавляем скорость для демонстрации движения
        auto& velocity = m_registry.emplace<VelocityComponent>(entity);
        velocity.vx = 50.0f * (i % 2 == 0 ? 1.0f : -1.0f);
        velocity.vy = 30.0f;
        velocity.angularVelocity = 45.0f * (i + 1);

        LOG_DEBUG("Created test entity {} at ({}, {})", static_cast<int>(entity), transform.x, transform.y);
    }

    // Создаем временную сущность (уничтожится через 5 секунд)
    {
        auto entity = m_registry.create();
        m_registry.emplace<NameComponent>(entity, "TemporarySquare");
        m_registry.emplace<TagComponent>(entity, "temporary");

        auto& transform = m_registry.emplace<TransformComponent>(entity);
        transform.x = 640.0f;
        transform.y = 200.0f;
        transform.scaleX = 1.5f;
        transform.scaleY = 1.5f;

        auto& sprite = m_registry.emplace<SpriteComponent>(entity);
        sprite.textureName = "test_square";
        sprite.color = sf::Color::Cyan;
        sprite.layer = 10;

        auto& velocity = m_registry.emplace<VelocityComponent>(entity);
        velocity.angularVelocity = 90.0f;

        // Добавляем компонент времени жизни (5 секунд)
        m_registry.emplace<LifetimeComponent>(entity, 5.0f, true);

        LOG_INFO("Created temporary entity (lifetime: 5.0s)");
    }
    */

    LOG_INFO("Old test entities disabled - using tile system");
    LOG_INFO("Game scene initialized with ECS");

    // Создаем тестовую сцену с тайловыми объектами
    createTileTestScene();
}

void GameState::createTileTestScene() {
    LOG_INFO("Creating tile-based test scene (Milestone 1.3)");

    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available");
        return;
    }

    // Загружаем тестовые PNG тайлы из assets/sprites/TEST
    LOG_INFO("Loading tile textures from PNG files");

    // Зеленый тайл (земля) - из PNG файла
    if (!resources->loadTexture("tile_green", "assets/sprites/TEST/testFloor1.png")) {
        LOG_ERROR("Failed to load testFloor1.png");
    } else {
        LOG_INFO("Loaded tile_green from testFloor1.png");
    }

    // Синий тайл (вода) - создаем в памяти
    sf::Image blueTile(sf::Vector2u(TILE_SIZE, TILE_SIZE), sf::Color(30, 144, 255));
    resources->loadTextureFromImage("tile_blue", blueTile);

    // Коричневый тайл (объект) - из PNG файла
    if (!resources->loadTexture("tile_brown", "assets/sprites/TEST/testObj.png")) {
        LOG_ERROR("Failed to load testObj.png");
    } else {
        LOG_INFO("Loaded tile_brown from testObj.png");
    }

    // Желтый тайл 2x2 (большой объект)
    sf::Image yellowBigTile(sf::Vector2u(TILE_SIZE * 2, TILE_SIZE * 2), sf::Color(255, 215, 0));
    for (unsigned int i = 0; i < TILE_SIZE * 2; ++i) {
        yellowBigTile.setPixel(sf::Vector2u(i, 0), sf::Color::Black);
        yellowBigTile.setPixel(sf::Vector2u(i, TILE_SIZE * 2 - 1), sf::Color::Black);
        yellowBigTile.setPixel(sf::Vector2u(0, i), sf::Color::Black);
        yellowBigTile.setPixel(sf::Vector2u(TILE_SIZE * 2 - 1, i), sf::Color::Black);
    }
    resources->loadTextureFromImage("tile_yellow_2x2", yellowBigTile);

    // Красный индикатор (для оверлея) - увеличен до 16x16 для видимости
    sf::Image redIndicator(sf::Vector2u(16, 16), sf::Color::Red);
    // Добавим желтую рамку для отладки
    for (unsigned int i = 0; i < 16; ++i) {
        redIndicator.setPixel(sf::Vector2u(i, 0), sf::Color::Yellow);
        redIndicator.setPixel(sf::Vector2u(i, 15), sf::Color::Yellow);
        redIndicator.setPixel(sf::Vector2u(0, i), sf::Color::Yellow);
        redIndicator.setPixel(sf::Vector2u(15, i), sf::Color::Yellow);
    }
    resources->loadTextureFromImage("indicator_red", redIndicator);

    // Спрайт-лист для анимации - загружаем из PNG файла (5 кадров по 32x32 = 160x32)
    if (!resources->loadTexture("anim_pulse", "assets/sprites/TEST/testObjAnimation.png")) {
        LOG_ERROR("Failed to load testObjAnimation.png");
    } else {
        LOG_INFO("Loaded anim_pulse from testObjAnimation.png (5 frames)");
    }

    // === 1. Создаем "пол" из зеленых тайлов (слой Ground) ===
    LOG_INFO("Creating ground layer");
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 12; ++x) {
            auto entity = m_registry.create();

            m_registry.emplace<NameComponent>(entity, "Ground_" + std::to_string(y) + "_" + std::to_string(x));
            m_registry.emplace<TilePositionComponent>(entity, x, y, 1, 1);

            auto& transform = m_registry.emplace<TransformComponent>(entity);
            // Позиция будет автоматически установлена TilePositionSystem

            auto& sprite = m_registry.emplace<SpriteComponent>(entity);
            sprite.textureName = "tile_green";
            sprite.layer = toInt(RenderLayer::Ground);
            sprite.visible = true;
        }
    }

    // Добавляем немного "воды" на пол
    for (int x = 2; x < 6; ++x) {
        for (int y = 3; y < 5; ++y) {
            auto view = m_registry.view<TilePositionComponent, SpriteComponent>();
            for (auto entity : view) {
                auto& tilePos = view.get<TilePositionComponent>(entity);
                if (tilePos.tileX == x && tilePos.tileY == y) {
                    auto& sprite = view.get<SpriteComponent>(entity);
                    sprite.textureName = "tile_blue";
                }
            }
        }
    }

    // === 2. Создаем промышленные объекты (слой Objects) ===
    LOG_INFO("Creating industrial objects");

    // Ряд простых объектов на разных Y для демонстрации Y-sorting
    for (int i = 0; i < 5; ++i) {
        auto entity = m_registry.create();

        m_registry.emplace<NameComponent>(entity, "Object_" + std::to_string(i));
        m_registry.emplace<TilePositionComponent>(entity, 1 + i * 2, 1 + i, 1, 1);
        m_registry.emplace<TransformComponent>(entity);  // ИСПРАВЛЕНО: добавлен TransformComponent

        auto& sprite = m_registry.emplace<SpriteComponent>(entity);
        sprite.textureName = "tile_brown";
        sprite.layer = toInt(RenderLayer::Objects);  // Y-sorting будет применен TilePositionSystem
        sprite.visible = true;

        LOG_DEBUG("Created object {} at tile ({}, {})", i, 1 + i * 2, 1 + i);
    }

    // === 3. Многотайловый объект 2x2 ===
    {
        auto entity = m_registry.create();

        m_registry.emplace<NameComponent>(entity, "BigObject_2x2");
        m_registry.emplace<TilePositionComponent>(entity, 8, 2, 2, 2);
        m_registry.emplace<TransformComponent>(entity);  // ИСПРАВЛЕНО: добавлен TransformComponent

        auto& sprite = m_registry.emplace<SpriteComponent>(entity);
        sprite.textureName = "tile_yellow_2x2";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;

        LOG_INFO("Created 2x2 tile object at (8, 2)");
    }

    // === 4. Анимированный объект ===
    {
        auto entity = m_registry.create();

        m_registry.emplace<NameComponent>(entity, "AnimatedObject");
        m_registry.emplace<TilePositionComponent>(entity, 5, 6, 1, 1);
        m_registry.emplace<TransformComponent>(entity);  // ИСПРАВЛЕНО: добавлен TransformComponent

        auto& sprite = m_registry.emplace<SpriteComponent>(entity);
        sprite.textureName = "anim_pulse";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;

        // Добавляем анимацию (5 кадров из testObjAnimation.png)
        auto& anim = m_registry.emplace<AnimationComponent>(entity);
        anim.currentAnimation = "pulse";
        anim.frameCount = 5;  // 5 кадров из PNG файла
        anim.frameWidth = TILE_SIZE;
        anim.frameHeight = TILE_SIZE;
        anim.frameDelay = 0.3f;  // ~3 FPS
        anim.loop = true;
        anim.playing = true;

        LOG_INFO("Created animated object at (5, 6) with 5-frame animation");
    }

    // === 5. Объект с оверлеем (индикатором) ===
    {
        // Родительский объект
        auto parent = m_registry.create();

        m_registry.emplace<NameComponent>(parent, "ObjectWithOverlay");
        m_registry.emplace<TilePositionComponent>(parent, 3, 6, 1, 1);
        m_registry.emplace<TransformComponent>(parent);  // ИСПРАВЛЕНО: добавлен TransformComponent

        auto& sprite = m_registry.emplace<SpriteComponent>(parent);
        sprite.textureName = "tile_brown";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;

        // Создаем дочернюю сущность (оверлей)
        auto overlay = m_registry.create();

        m_registry.emplace<NameComponent>(overlay, "RedIndicator");
        m_registry.emplace<ParentComponent>(overlay, parent);

        // Оверлей также должен иметь Transform для рендеринга
        m_registry.emplace<TransformComponent>(overlay);

        auto& overlaySprite = m_registry.emplace<SpriteComponent>(overlay);
        overlaySprite.textureName = "indicator_red";
        overlaySprite.layer = toInt(RenderLayer::Overlays);
        overlaySprite.visible = true;

        // Компонент оверлея (смещение 20 пикселей вправо, 4 вниз от родителя)
        m_registry.emplace<OverlayComponent>(overlay, 20.0f, 4.0f, true);

        // Регистрируем дочернюю сущность у родителя
        auto& children = m_registry.get_or_emplace<ChildrenComponent>(parent);
        children.addChild(overlay);

        LOG_INFO("Created object with overlay at (3, 6)");
    }

    // === 6. Тестирование системы коллизий (Milestone 1.3.1 - Task 1.2) ===
    LOG_INFO("Creating collision test objects");

    // Создаем статичные стены с solid коллизиями
    {
        // Левая стена
        auto wall1 = m_registry.create();
        m_registry.emplace<NameComponent>(wall1, "Wall_Left");
        m_registry.emplace<TilePositionComponent>(wall1, 0, 3, 1, 2);
        m_registry.emplace<TransformComponent>(wall1);

        auto& sprite = m_registry.emplace<SpriteComponent>(wall1);
        sprite.textureName = "tile_brown";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;

        auto& collision = m_registry.emplace<CollisionComponent>(wall1);
        collision.isSolid = true;
        collision.isTrigger = false;
        collision.layer = "wall";
        collision.setFromTileSize(1, 2);
        collision.onCollisionEnter = [](entt::entity other) {
            LOG_INFO("Wall collision ENTER with entity {}", static_cast<uint32_t>(other));
        };

        LOG_INFO("Created wall at (0, 3) with solid collision");
    }

    // Правая стена
    {
        auto wall2 = m_registry.create();
        m_registry.emplace<NameComponent>(wall2, "Wall_Right");
        m_registry.emplace<TilePositionComponent>(wall2, 11, 3, 1, 2);
        m_registry.emplace<TransformComponent>(wall2);

        auto& sprite = m_registry.emplace<SpriteComponent>(wall2);
        sprite.textureName = "tile_brown";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;

        auto& collision = m_registry.emplace<CollisionComponent>(wall2);
        collision.isSolid = true;
        collision.layer = "wall";
        collision.setFromTileSize(1, 2);

        LOG_INFO("Created wall at (11, 3) with solid collision");
    }

    // Создаем движущийся объект с коллизией
    {
        auto movingObj = m_registry.create();
        m_registry.emplace<NameComponent>(movingObj, "MovingObject");
        m_registry.emplace<TilePositionComponent>(movingObj, 6, 0, 1, 1);

        auto& transform = m_registry.emplace<TransformComponent>(movingObj);

        auto& sprite = m_registry.emplace<SpriteComponent>(movingObj);
        sprite.textureName = "test_square";  // Используем красный квадрат
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;
        sprite.color = sf::Color::Cyan;  // Выделяем цветом

        // Добавляем скорость (движется вниз)
        auto& velocity = m_registry.emplace<VelocityComponent>(movingObj);
        velocity.vx = 0.0f;
        velocity.vy = 50.0f;  // Медленное движение вниз

        // Добавляем коллизию
        auto& collision = m_registry.emplace<CollisionComponent>(movingObj);
        collision.isSolid = false;  // Не блокирует, но детектирует
        collision.layer = "player";
        collision.setFromTileSize(1, 1);

        collision.onCollisionEnter = [](entt::entity other) {
            LOG_WARN("MovingObject collision ENTER with entity {}", static_cast<uint32_t>(other));
        };

        collision.onCollisionExit = [](entt::entity other) {
            LOG_INFO("MovingObject collision EXIT with entity {}", static_cast<uint32_t>(other));
        };

        LOG_INFO("Created moving object at (6, 0) with collision detection");
    }

    // Создаем trigger зону
    {
        auto trigger = m_registry.create();
        m_registry.emplace<NameComponent>(trigger, "TriggerZone");
        m_registry.emplace<TilePositionComponent>(trigger, 5, 3, 3, 2);
        m_registry.emplace<TransformComponent>(trigger);

        auto& sprite = m_registry.emplace<SpriteComponent>(trigger);
        sprite.textureName = "tile_green";
        sprite.layer = toInt(RenderLayer::Objects) - 1;  // Под объектами
        sprite.visible = true;
        sprite.color = sf::Color(255, 255, 0, 100);  // Полупрозрачный желтый

        auto& collision = m_registry.emplace<CollisionComponent>(trigger);
        collision.isSolid = false;
        collision.isTrigger = true;
        collision.layer = "trigger";
        collision.setFromTileSize(3, 2);

        collision.onCollisionEnter = [](entt::entity other) {
            LOG_WARN("Entity {} ENTERED trigger zone!", static_cast<uint32_t>(other));
        };

        collision.onCollisionExit = [](entt::entity other) {
            LOG_INFO("Entity {} LEFT trigger zone", static_cast<uint32_t>(other));
        };

        LOG_INFO("Created trigger zone at (5, 3) size 3x2");
    }

    // === 7. Тестирование FSM (Задача 2.2) - Лампа с состояниями: off → on → broken ===
    LOG_INFO("Creating FSM test entity (Lamp)");
    {
        auto lamp = m_registry.create();
        m_registry.emplace<NameComponent>(lamp, "Lamp_FSM");
        m_registry.emplace<TilePositionComponent>(lamp, 10, 6, 1, 1);
        m_registry.emplace<TransformComponent>(lamp);

        auto& sprite = m_registry.emplace<SpriteComponent>(lamp);
        sprite.textureName = "tile_brown";
        sprite.layer = toInt(RenderLayer::Objects);
        sprite.visible = true;
        sprite.color = sf::Color(128, 128, 128);  // Серая (выключенная)

        // Добавляем FSM компонент с начальным состоянием "off"
        auto& fsm = m_registry.emplace<EntityStateComponent>(lamp, "off");

        // Регистрируем коллбеки для состояний
        // ВАЖНО: Захватываем entity, чтобы получить доступ к компонентам через registry
        fsm.registerOnEnter("off", [this, lamp]() {
            if (auto* spr = m_registry.try_get<SpriteComponent>(lamp)) {
                spr->color = sf::Color(128, 128, 128);  // Серая
                LOG_INFO("Lamp FSM: state → OFF (gray)");
            }
        });

        fsm.registerOnEnter("on", [this, lamp]() {
            if (auto* spr = m_registry.try_get<SpriteComponent>(lamp)) {
                spr->color = sf::Color(255, 255, 0);  // Желтая (включенная)
                LOG_INFO("Lamp FSM: state → ON (yellow)");
            }
        });

        fsm.registerOnEnter("broken", [this, lamp]() {
            if (auto* spr = m_registry.try_get<SpriteComponent>(lamp)) {
                spr->color = sf::Color(255, 0, 0);  // Красная (сломанная)
                LOG_WARN("Lamp FSM: state → BROKEN (red) - permanent failure!");
            }
        });

        fsm.registerOnExit("on", []() {
            LOG_INFO("Lamp FSM: exiting ON state");
        });

        LOG_INFO("Created Lamp FSM entity at (10, 6)");
        LOG_INFO("Lamp starts in 'off' state (gray)");
        LOG_INFO("To test FSM: manually trigger state transitions in code or via events");
        LOG_INFO("Expected behavior: off (gray) → on (yellow) → broken (red)");
    }

    LOG_INFO("Tile test scene created with {} entities (including collision test objects and FSM lamp)", m_registry.storage<entt::entity>().size());
}

void GameState::drawDebugGrid(sf::RenderWindow& window) {
    // Получаем границы видимой области в world coordinates
    sf::Vector2f viewCenter = m_worldView.getCenter();
    sf::Vector2f viewSize = m_worldView.getSize();

    float left = viewCenter.x - viewSize.x / 2.0f;
    float right = viewCenter.x + viewSize.x / 2.0f;
    float top = viewCenter.y - viewSize.y / 2.0f;
    float bottom = viewCenter.y + viewSize.y / 2.0f;

    // Выравниваем границы по сетке тайлов
    int startX = static_cast<int>(left / TILE_SIZE) - 1;
    int endX = static_cast<int>(right / TILE_SIZE) + 1;
    int startY = static_cast<int>(top / TILE_SIZE) - 1;
    int endY = static_cast<int>(bottom / TILE_SIZE) + 1;

    // Создаем массив вертексов для линий (2 точки на линию)
    std::vector<sf::Vertex> lines;
    sf::Color gridColor(0, 0, 0, 128);  // Черный полупрозрачный

    // Вертикальные линии
    for (int x = startX; x <= endX; ++x) {
        float xPos = x * TILE_SIZE;
        lines.emplace_back(sf::Vector2f(xPos, top), gridColor);
        lines.emplace_back(sf::Vector2f(xPos, bottom), gridColor);
    }

    // Горизонтальные линии
    for (int y = startY; y <= endY; ++y) {
        float yPos = y * TILE_SIZE;
        lines.emplace_back(sf::Vector2f(left, yPos), gridColor);
        lines.emplace_back(sf::Vector2f(right, yPos), gridColor);
    }

    // Рисуем все линии одним вызовом
    window.draw(lines.data(), lines.size(), sf::PrimitiveType::Lines);
}

} // namespace core
