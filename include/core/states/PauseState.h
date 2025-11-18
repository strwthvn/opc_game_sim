#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/View.hpp>
#include <vector>

namespace core {

/**
 * @brief Состояние паузы
 *
 * Накладывается поверх игрового состояния
 * Отображает меню паузы с опциями:
 * - Продолжить
 * - Настройки
 * - В главное меню
 * - Выход
 */
class PauseState : public State {
public:
    /**
     * @brief Конструктор
     * @param stateManager Указатель на менеджер состояний
     */
    explicit PauseState(StateManager* stateManager);

    /**
     * @brief Деструктор
     */
    ~PauseState() override = default;

    void onEnter() override;
    void onExit() override;
    bool handleEvent(const sf::Event& event) override;
    void update(double dt) override;
    void render(sf::RenderWindow& window) override;
    void onWindowResize(const sf::Vector2u& newSize) override;
    std::string getName() const override { return "PauseState"; }

    // Не обновляем игру под паузой
    bool updateBelow() const override { return false; }
    // Но рендерим игру под паузой
    bool renderBelow() const override { return true; }

private:
    /**
     * @brief Пункты меню паузы
     */
    enum class PauseMenuItem {
        Resume = 0,
        Settings,
        MainMenu,
        Exit,
        Count
    };

    /**
     * @brief Инициализирует меню паузы
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
    std::unique_ptr<sf::Text> m_title;         ///< Заголовок "PAUSED"
    std::vector<std::unique_ptr<sf::Text>> m_menuItems;  ///< Пункты меню
    sf::RectangleShape m_overlay;              ///< Полупрозрачный оверлей
    int m_selectedItem;                        ///< Индекс выбранного пункта
    bool m_fontLoaded;                         ///< Флаг загрузки шрифта
    sf::View m_uiView;                         ///< Фиксированный View для UI

    // Цвета
    static constexpr auto SELECTED_COLOR = sf::Color::Yellow;
    static constexpr auto NORMAL_COLOR = sf::Color::White;
    static constexpr auto TITLE_COLOR = sf::Color(255, 100, 100);

    // Размеры и позиции (в координатах фиксированного UI View 1280x720)
    static constexpr unsigned int UI_WIDTH = 1280;
    static constexpr unsigned int UI_HEIGHT = 720;
    static constexpr unsigned int TITLE_FONT_SIZE = 64;
    static constexpr unsigned int MENU_ITEM_FONT_SIZE = 32;
    static constexpr float TITLE_X = 500.0f;
    static constexpr float TITLE_Y = 150.0f;
    static constexpr float MENU_ITEMS_X = 520.0f;
    static constexpr float MENU_ITEMS_START_Y = 320.0f;
    static constexpr float MENU_ITEMS_SPACING = 60.0f;
};

} // namespace core
