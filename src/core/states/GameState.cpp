#include "core/states/GameState.h"
#include "core/states/PauseState.h"
#include "core/states/MenuState.h"
#include "core/StateManager.h"
#include "core/InputManager.h"
#include "core/ResourceManager.h"
#include "core/systems/RenderSystem.h"
#include "core/systems/UpdateSystem.h"
#include "core/systems/LifetimeSystem.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Event.hpp>
#include <sstream>
#include <iomanip>

namespace core {

GameState::GameState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_fontLoaded(false)
    , m_elapsedTime(0.0)
    , m_updateCount(0) {
}

GameState::~GameState() = default;

void GameState::onEnter() {
    LOG_INFO("Entering GameState");

    // Инициализируем Views
    auto windowSize = getWindowSize();

    // World View - расширяется с размером окна (показывает больше мира)
    m_worldView.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    m_worldView.setCenter(sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f));
    LOG_DEBUG("GameState World View initialized: {}x{}", windowSize.x, windowSize.y);

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
    // Оставляем handleEvent для специальных событий (например, закрытие окна)
    // Клавиатурный ввод теперь обрабатывается через InputManager в update()

    // Обработка кликов мыши, если нужно
    // TODO: Обработка кликов мыши, перемещения камеры и т.д.

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

        // TODO: Обработка непрерывного ввода (например, перемещение камеры)
        // if (input->isKeyPressed(sf::Keyboard::Key::W)) { camera.moveUp(dt); }
    }

    // Обновление ECS систем
    if (m_updateSystem) {
        m_updateSystem->update(m_registry, dt);
    }

    // Обновление системы времени жизни
    if (m_lifetimeSystem) {
        m_lifetimeSystem->update(m_registry, dt);
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
        oss << "\nControls:\n";
        oss << "ESC/P - Pause\n";
        oss << "Q - Back to Menu";

        m_infoText->setString(oss.str());
    }
}

void GameState::render(sf::RenderWindow& window) {
    // Устанавливаем World View для рендеринга игровых объектов
    window.setView(m_worldView);

    // Рендеринг ECS entities (игровые объекты)
    if (m_renderSystem) {
        m_renderSystem->render(m_registry, window);
    }

    // TODO: Рендеринг тайловой карты

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

    LOG_INFO("Created {} test entities", m_registry.storage<entt::entity>().size());
    LOG_INFO("Game scene initialized with ECS");
}

} // namespace core
