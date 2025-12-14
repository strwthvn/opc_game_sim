#include "core/states/MenuState.h"
#include "core/states/GameState.h"
#include "core/states/SettingsState.h"
#include "core/StateManager.h"
#include "core/ResourceManager.h"
#include "core/AudioManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace core {

MenuState::MenuState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_selectedItem(0)
    , m_fontLoaded(false) {
    // UI View будет инициализирован в onEnter() когда размер окна известен
}

void MenuState::onEnter() {
    LOG_INFO("Entering MenuState");

    // Загружаем звук клика меню
    auto* resourceManager = getResourceManager();
    if (resourceManager && !resourceManager->hasSound("menu_click")) {
        if (resourceManager->loadSound("menu_click", "assets/sounds/menu_click.wav")) {
            LOG_INFO("Loaded menu click sound");
        } else {
            LOG_WARN("Failed to load menu click sound");
        }
    }

    // Инициализируем UI View размером окна (1:1 пиксели)
    auto windowSize = getWindowSize();
    m_uiView.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    m_uiView.setCenter(sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f));
    LOG_DEBUG("MenuState UI View initialized: {}x{} (window size)", windowSize.x, windowSize.y);

    initializeMenu();
    LOG_DEBUG("MenuState::onEnter() completed. Font loaded: {}", m_fontLoaded);
}

void MenuState::onExit() {
    LOG_INFO("Exiting MenuState");
}

bool MenuState::handleEvent(const sf::Event& event) {
    // Обработка нажатий клавиш
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Up:
                moveUp();
                return true;

            case sf::Keyboard::Key::Down:
                moveDown();
                return true;

            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                activateSelected();
                return true;

            case sf::Keyboard::Key::Escape:
                // В меню Escape = выход
                if (m_selectedItem != static_cast<int>(MenuItem::Exit)) {
                    m_selectedItem = static_cast<int>(MenuItem::Exit);
                    updateMenuColors();
                }
                return true;

            default:
                break;
        }
    }

    return false;
}

void MenuState::update(double dt) {
    (void)dt; // Пока нет анимаций
}

void MenuState::onWindowResize(const sf::Vector2u& newSize) {
    LOG_DEBUG("MenuState received window resize event: {}x{}", newSize.x, newSize.y);

    // Обновляем UI View размером окна (1:1 пиксели для четкого текста)
    m_uiView.setSize(sf::Vector2f(newSize.x, newSize.y));
    m_uiView.setCenter(sf::Vector2f(newSize.x / 2.0f, newSize.y / 2.0f));

    // Пересоздаем меню с новыми позициями
    if (m_fontLoaded) {
        initializeMenu();
    }
}

void MenuState::render(sf::RenderWindow& window) {
    static int renderCallCount = 0;
    if (renderCallCount == 0) {
        LOG_INFO("MenuState::render() called for the first time");
    }
    renderCallCount++;

    if (renderCallCount % 300 == 0) {  // Логируем каждые ~5 секунд при 60 FPS
        LOG_DEBUG("MenuState::render() has been called {} times", renderCallCount);
    }

    // Устанавливаем фиксированный UI View для рендеринга меню
    window.setView(m_uiView);

    // Визуальная отладка: рисуем несколько цветных прямоугольников
    // для проверки, что рендеринг работает вообще
    sf::RectangleShape debugRect1(sf::Vector2f(200, 100));
    debugRect1.setPosition(sf::Vector2f(100, 100));
    debugRect1.setFillColor(sf::Color::Red);
    window.draw(debugRect1);

    sf::RectangleShape debugRect2(sf::Vector2f(150, 80));
    debugRect2.setPosition(sf::Vector2f(100, 220));
    debugRect2.setFillColor(sf::Color::Green);
    window.draw(debugRect2);

    sf::RectangleShape debugRect3(sf::Vector2f(180, 90));
    debugRect3.setPosition(sf::Vector2f(100, 320));
    debugRect3.setFillColor(sf::Color::Blue);
    window.draw(debugRect3);

    // Не рендерим текст если шрифт не загрузился
    if (!m_fontLoaded) {
        LOG_WARN("MenuState::render() - font not loaded, showing only debug rectangles");
        return;
    }

    // Рисуем заголовок
    if (m_title) {
        window.draw(*m_title);
    }

    // Рисуем пункты меню
    for (const auto& item : m_menuItems) {
        if (item) {
            window.draw(*item);
        }
    }
}

void MenuState::initializeMenu() {
    LOG_DEBUG("MenuState::initializeMenu() called");

    // Получаем шрифт из ResourceManager
    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available in MenuState");
        return;
    }

    LOG_DEBUG("ResourceManager is available, attempting to load font");

    try {
        m_font = &resources->getFont("default");
        m_fontLoaded = true;
        LOG_INFO("Font loaded successfully in MenuState");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load default font for MenuState: {}", e.what());
        LOG_ERROR("MenuState will not display text. Check font paths in ResourceManager.");
        m_fontLoaded = false;
        return; // Не инициализируем текст если шрифт не загрузился
    }

    // Получаем текущий размер окна для относительного позиционирования
    auto windowSize = getWindowSize();
    float centerX = windowSize.x / 2.0f;
    float centerY = windowSize.y / 2.0f;

    // Настройка заголовка (по центру экрана, сверху)
    m_title = std::make_unique<sf::Text>(*m_font);
    m_title->setString("OPC GAME SIMULATOR");
    m_title->setCharacterSize(TITLE_FONT_SIZE);
    m_title->setFillColor(TITLE_COLOR);

    // Центрируем заголовок
    auto titleBounds = m_title->getLocalBounds();
    float titleX = centerX - titleBounds.size.x / 2.0f;
    float titleY = windowSize.y * 0.15f; // 15% от высоты экрана
    m_title->setPosition(sf::Vector2f(titleX, titleY));
    LOG_DEBUG("Title created at ({}, {}), size {}", titleX, titleY, TITLE_FONT_SIZE);

    // Создание пунктов меню
    const std::vector<std::string> itemLabels = {
        "New Game",
        "Continue",
        "Settings",
        "Exit"
    };

    m_menuItems.clear();
    float menuStartY = centerY - (itemLabels.size() * MENU_ITEMS_SPACING / 2.0f); // Центрируем по вертикали

    for (size_t i = 0; i < itemLabels.size(); ++i) {
        auto item = std::make_unique<sf::Text>(*m_font);
        item->setString(itemLabels[i]);
        item->setCharacterSize(MENU_ITEM_FONT_SIZE);

        // Центрируем каждый пункт меню
        auto itemBounds = item->getLocalBounds();
        float itemX = centerX - itemBounds.size.x / 2.0f;
        float itemY = menuStartY + i * MENU_ITEMS_SPACING;
        item->setPosition(sf::Vector2f(itemX, itemY));

        LOG_DEBUG("Menu item {} '{}' created at ({}, {})", i, itemLabels[i], itemX, itemY);
        m_menuItems.push_back(std::move(item));
    }

    updateMenuColors();
    LOG_INFO("MenuState initialized with {} menu items for window {}x{}",
             m_menuItems.size(), windowSize.x, windowSize.y);
}

void MenuState::moveUp() {
    m_selectedItem--;
    if (m_selectedItem < 0) {
        m_selectedItem = static_cast<int>(MenuItem::Count) - 1;
    }
    updateMenuColors();
    LOG_DEBUG("Menu selection: {}", m_selectedItem);

    // Воспроизводим звук навигации
    auto* audioManager = getAudioManager();
    if (audioManager) {
        audioManager->playSound("menu_click", 80.0f);
    }
}

void MenuState::moveDown() {
    m_selectedItem++;
    if (m_selectedItem >= static_cast<int>(MenuItem::Count)) {
        m_selectedItem = 0;
    }
    updateMenuColors();
    LOG_DEBUG("Menu selection: {}", m_selectedItem);

    // Воспроизводим звук навигации
    auto* audioManager = getAudioManager();
    if (audioManager) {
        audioManager->playSound("menu_click", 80.0f);
    }
}

void MenuState::activateSelected() {
    LOG_INFO("Menu item activated: {}", m_selectedItem);

    // Воспроизводим звук подтверждения (чуть громче чем навигация)
    auto* audioManager = getAudioManager();
    if (audioManager) {
        audioManager->playSound("menu_click", 100.0f);
    }

    switch (static_cast<MenuItem>(m_selectedItem)) {
        case MenuItem::NewGame:
            LOG_INFO("Starting new game");
            m_stateManager->changeState(std::make_unique<GameState>(m_stateManager));
            break;

        case MenuItem::Continue:
            LOG_INFO("Continue not implemented yet");
            // TODO: Загрузка сохраненной игры
            break;

        case MenuItem::Settings:
            LOG_INFO("Opening settings menu");
            m_stateManager->pushState(std::make_unique<SettingsState>(m_stateManager));
            break;

        case MenuItem::Exit:
            LOG_INFO("Exit requested from menu");
            m_stateManager->clearStates();
            break;

        default:
            break;
    }
}

void MenuState::updateMenuColors() {
    for (size_t i = 0; i < m_menuItems.size(); ++i) {
        if (m_menuItems[i]) {
            if (static_cast<int>(i) == m_selectedItem) {
                m_menuItems[i]->setFillColor(SELECTED_COLOR);
            } else {
                m_menuItems[i]->setFillColor(NORMAL_COLOR);
            }
        }
    }
}

} // namespace core
