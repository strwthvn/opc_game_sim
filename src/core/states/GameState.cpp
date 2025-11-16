#include "core/states/GameState.h"
#include "core/states/PauseState.h"
#include "core/states/MenuState.h"
#include "core/StateManager.h"
#include "core/InputManager.h"
#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
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

void GameState::onEnter() {
    LOG_INFO("Entering GameState");
    initializeScene();
}

void GameState::onExit() {
    LOG_INFO("Exiting GameState");
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

    // TODO: Обновление ECS систем
    // TODO: Обновление физики
    // TODO: Обновление OPC UA привязок

    // Обновляем информационный текст
    if (m_infoText) {
        std::ostringstream oss;
        oss << "GAME STATE - RUNNING\n";
        oss << "Time: " << std::fixed << std::setprecision(1) << m_elapsedTime << "s\n";
        oss << "Updates: " << m_updateCount << "\n";
        oss << "\nControls:\n";
        oss << "ESC/P - Pause\n";
        oss << "Q - Back to Menu";

        m_infoText->setString(oss.str());
    }
}

void GameState::render(sf::RenderWindow& window) {
    // TODO: Рендеринг игровых объектов
    // TODO: Рендеринг тайловой карты
    // TODO: Рендеринг ECS entities

    // Не рендерим текст если шрифт не загрузился
    if (!m_fontLoaded) {
        return;
    }

    // Рисуем информационный текст
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

    // TODO: Инициализация ECS
    // TODO: Загрузка уровня
    // TODO: Создание игровых объектов

    LOG_INFO("Game scene initialized");
}

} // namespace core
