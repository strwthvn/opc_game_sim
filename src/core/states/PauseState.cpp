#include "core/states/PauseState.h"
#include "core/states/MenuState.h"
#include "core/StateManager.h"
#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace core {

PauseState::PauseState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_selectedItem(0)
    , m_fontLoaded(false) {
}

void PauseState::onEnter() {
    LOG_INFO("Entering PauseState");
    initializeMenu();
}

void PauseState::onExit() {
    LOG_INFO("Exiting PauseState");
}

bool PauseState::handleEvent(const sf::Event& event) {
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
            case sf::Keyboard::Key::P:
                // Escape в паузе = продолжить игру
                LOG_INFO("Resume from pause via Escape");
                m_stateManager->popState();
                return true;

            default:
                break;
        }
    }

    // Блокируем все события от передачи в игру
    return true;
}

void PauseState::update(double dt) {
    (void)dt; // Пока нет анимаций
}

void PauseState::render(sf::RenderWindow& window) {
    // Рисуем полупрозрачный оверлей (всегда, даже если шрифт не загрузился)
    window.draw(m_overlay);

    // Не рендерим текст если шрифт не загрузился
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

void PauseState::initializeMenu() {
    // Получаем шрифт из ResourceManager
    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available in PauseState");
        return;
    }

    try {
        m_font = &resources->getFont("default");
        m_fontLoaded = true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load default font for PauseState: {}", e.what());
        m_fontLoaded = false;
        return; // Не инициализируем текст если шрифт не загрузился
    }

    // Настройка полупрозрачного оверлея - получаем размер окна динамически
    sf::Vector2u windowSize = getWindowSize();
    m_overlay.setSize(sf::Vector2f(
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y)
    ));
    m_overlay.setFillColor(sf::Color(0, 0, 0, 128)); // Полупрозрачный черный

    // Настройка заголовка
    m_title = std::make_unique<sf::Text>(*m_font);
    m_title->setString("PAUSED");
    m_title->setCharacterSize(TITLE_FONT_SIZE);
    m_title->setFillColor(TITLE_COLOR);
    m_title->setPosition(sf::Vector2f(TITLE_X, TITLE_Y));

    // Создание пунктов меню
    const std::vector<std::string> itemLabels = {
        "Resume",
        "Settings",
        "Main Menu",
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

void PauseState::moveUp() {
    m_selectedItem--;
    if (m_selectedItem < 0) {
        m_selectedItem = static_cast<int>(PauseMenuItem::Count) - 1;
    }
    updateMenuColors();
    LOG_DEBUG("Pause menu selection: {}", m_selectedItem);
}

void PauseState::moveDown() {
    m_selectedItem++;
    if (m_selectedItem >= static_cast<int>(PauseMenuItem::Count)) {
        m_selectedItem = 0;
    }
    updateMenuColors();
    LOG_DEBUG("Pause menu selection: {}", m_selectedItem);
}

void PauseState::activateSelected() {
    LOG_INFO("Pause menu item activated: {}", m_selectedItem);

    switch (static_cast<PauseMenuItem>(m_selectedItem)) {
        case PauseMenuItem::Resume:
            LOG_INFO("Resuming game");
            m_stateManager->popState(); // Убираем паузу, возвращаемся в игру
            break;

        case PauseMenuItem::Settings:
            LOG_INFO("Settings not implemented yet");
            // TODO: Состояние настроек
            break;

        case PauseMenuItem::MainMenu:
            LOG_INFO("Returning to main menu");
            // Убираем паузу и игру, переходим в меню
            m_stateManager->popState(); // Убираем паузу
            m_stateManager->changeState(std::make_unique<MenuState>(m_stateManager)); // Меняем игру на меню
            break;

        case PauseMenuItem::Exit:
            LOG_INFO("Exit requested from pause");
            m_stateManager->clearStates();
            break;

        default:
            break;
    }
}

void PauseState::updateMenuColors() {
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
