#include "core/states/MenuState.h"
#include "core/states/GameState.h"
#include "core/StateManager.h"
#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace core {

MenuState::MenuState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_selectedItem(0)
    , m_fontLoaded(false) {
}

void MenuState::onEnter() {
    LOG_INFO("Entering MenuState");
    initializeMenu();
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

void MenuState::render(sf::RenderWindow& window) {
    // Не рендерим если шрифт не загрузился
    if (!m_fontLoaded) {
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
    // Получаем шрифт из ResourceManager
    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available in MenuState");
        return;
    }

    try {
        m_font = &resources->getFont("default");
        m_fontLoaded = true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load default font for MenuState: {}", e.what());
        m_fontLoaded = false;
        return; // Не инициализируем текст если шрифт не загрузился
    }

    // Настройка заголовка
    m_title = std::make_unique<sf::Text>(*m_font);
    m_title->setString("OPC GAME SIMULATOR");
    m_title->setCharacterSize(TITLE_FONT_SIZE);
    m_title->setFillColor(TITLE_COLOR);
    m_title->setPosition(sf::Vector2f(TITLE_X, TITLE_Y));

    // Создание пунктов меню
    const std::vector<std::string> itemLabels = {
        "New Game",
        "Continue",
        "Settings",
        "Exit"
    };

    m_menuItems.clear();

    for (size_t i = 0; i < itemLabels.size(); ++i) {
        auto item = std::make_unique<sf::Text>(*m_font);
        item->setString(itemLabels[i]);
        item->setCharacterSize(MENU_ITEM_FONT_SIZE);
        item->setPosition(sf::Vector2f(MENU_ITEMS_X, MENU_ITEMS_START_Y + i * MENU_ITEMS_SPACING));
        m_menuItems.push_back(std::move(item));
    }

    updateMenuColors();
}

void MenuState::moveUp() {
    m_selectedItem--;
    if (m_selectedItem < 0) {
        m_selectedItem = static_cast<int>(MenuItem::Count) - 1;
    }
    updateMenuColors();
    LOG_DEBUG("Menu selection: {}", m_selectedItem);
}

void MenuState::moveDown() {
    m_selectedItem++;
    if (m_selectedItem >= static_cast<int>(MenuItem::Count)) {
        m_selectedItem = 0;
    }
    updateMenuColors();
    LOG_DEBUG("Menu selection: {}", m_selectedItem);
}

void MenuState::activateSelected() {
    LOG_INFO("Menu item activated: {}", m_selectedItem);

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
            LOG_INFO("Settings not implemented yet");
            // TODO: Состояние настроек
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
