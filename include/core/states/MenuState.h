#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <vector>

namespace core {

/**
 * @brief Состояние главного меню
 *
 * Отображает главное меню с опциями:
 * - Новая игра
 * - Продолжить
 * - Настройки
 * - Выход
 */
class MenuState : public State {
public:
    /**
     * @brief Конструктор
     * @param stateManager Указатель на менеджер состояний
     */
    explicit MenuState(StateManager* stateManager);

    /**
     * @brief Деструктор
     */
    ~MenuState() override = default;

    void onEnter() override;
    void onExit() override;
    bool handleEvent(const sf::Event& event) override;
    void update(double dt) override;
    void render(sf::RenderWindow& window) override;
    void onWindowResize(const sf::Vector2u& newSize) override;
    std::string getName() const override { return "MenuState"; }

private:
    /**
     * @brief Пункты меню
     */
    enum class MenuItem {
        NewGame = 0,
        Continue,
        Settings,
        Exit,
        Count
    };

    /**
     * @brief Инициализирует текст меню
     */
    void initializeMenu();

    /**
     * @brief Переходит к следующему пункту меню
     */
    void moveUp();

    /**
     * @brief Переходит к предыдущему пункту меню
     */
    void moveDown();

    /**
     * @brief Активирует выбранный пункт меню
     */
    void activateSelected();

    /**
     * @brief Обновляет цвета пунктов меню
     */
    void updateMenuColors();

    const sf::Font* m_font;                    ///< Указатель на шрифт из ResourceManager
    std::unique_ptr<sf::Text> m_title;         ///< Заголовок меню
    std::vector<std::unique_ptr<sf::Text>> m_menuItems;  ///< Пункты меню
    int m_selectedItem;                        ///< Индекс выбранного пункта
    bool m_fontLoaded;                         ///< Флаг загрузки шрифта
    sf::View m_uiView;                         ///< Фиксированный View для UI

    // Цвета
    static constexpr auto SELECTED_COLOR = sf::Color::Yellow;
    static constexpr auto NORMAL_COLOR = sf::Color::White;
    static constexpr auto TITLE_COLOR = sf::Color(100, 200, 255);

    // Размеры и позиции (в координатах фиксированного UI View 1280x720)
    static constexpr unsigned int UI_WIDTH = 1280;
    static constexpr unsigned int UI_HEIGHT = 720;
    static constexpr unsigned int TITLE_FONT_SIZE = 48;
    static constexpr unsigned int MENU_ITEM_FONT_SIZE = 32;
    static constexpr float TITLE_X = 400.0f;
    static constexpr float TITLE_Y = 100.0f;
    static constexpr float MENU_ITEMS_X = 500.0f;
    static constexpr float MENU_ITEMS_START_Y = 300.0f;
    static constexpr float MENU_ITEMS_SPACING = 60.0f;
};

} // namespace core
