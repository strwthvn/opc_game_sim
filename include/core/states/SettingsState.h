#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <vector>
#include <string>

namespace core {

/**
 * @brief Состояние настроек приложения
 *
 * Отображает меню настроек с опциями:
 * - Разрешение окна
 * - VSync вкл/выкл
 * - Громкость (заглушка для будущего)
 * - Кнопки управления (только отображение)
 * - Back (возврат в меню)
 */
class SettingsState : public State {
public:
    /**
     * @brief Конструктор
     * @param stateManager Указатель на менеджер состояний
     */
    explicit SettingsState(StateManager* stateManager);

    /**
     * @brief Деструктор
     */
    ~SettingsState() override = default;

    void onEnter() override;
    void onExit() override;
    bool handleEvent(const sf::Event& event) override;
    void update(double dt) override;
    void render(sf::RenderWindow& window) override;
    void onWindowResize(const sf::Vector2u& newSize) override;
    std::string getName() const override { return "SettingsState"; }

private:
    /**
     * @brief Пункты меню настроек
     */
    enum class SettingsItem {
        Resolution = 0,
        VSync,
        MasterVolume,  // Заглушка
        Controls,      // Только отображение
        Back,
        Count
    };

    /**
     * @brief Доступные разрешения экрана
     */
    struct Resolution {
        unsigned int width;
        unsigned int height;
        std::string label;
    };

    /**
     * @brief Инициализирует UI настроек
     */
    void initializeUI();

    /**
     * @brief Переходит к следующему пункту меню
     */
    void moveUp();

    /**
     * @brief Переходит к предыдущему пункту меню
     */
    void moveDown();

    /**
     * @brief Переходит к следующему значению настройки (вправо)
     */
    void moveRight();

    /**
     * @brief Переходит к предыдущему значению настройки (влево)
     */
    void moveLeft();

    /**
     * @brief Активирует выбранный пункт меню (для Back)
     */
    void activateSelected();

    /**
     * @brief Применяет текущие настройки
     */
    void applySettings();

    /**
     * @brief Обновляет отображение всех пунктов настроек
     */
    void updateSettingsDisplay();

    /**
     * @brief Обновляет цвета пунктов меню
     */
    void updateMenuColors();

    /**
     * @brief Получает строку для отображения текущего значения настройки
     * @param item Пункт настройки
     * @return Строка со значением
     */
    std::string getSettingValueString(SettingsItem item) const;

    const sf::Font* m_font;                          ///< Указатель на шрифт из ResourceManager
    std::unique_ptr<sf::Text> m_title;               ///< Заголовок меню настроек
    std::vector<std::unique_ptr<sf::Text>> m_settingLabels;   ///< Метки пунктов настроек
    std::vector<std::unique_ptr<sf::Text>> m_settingValues;   ///< Значения пунктов настроек
    int m_selectedItem;                              ///< Индекс выбранного пункта
    bool m_fontLoaded;                               ///< Флаг загрузки шрифта
    sf::View m_uiView;                               ///< Фиксированный View для UI

    // Значения настроек
    std::vector<Resolution> m_availableResolutions;  ///< Доступные разрешения
    size_t m_currentResolutionIndex;                 ///< Индекс текущего разрешения
    bool m_vsyncEnabled;                             ///< VSync включен/выключен
    int m_masterVolume;                              ///< Общая громкость (0-100)

    // Флаг изменений
    bool m_settingsChanged;                          ///< Настройки были изменены

    // Цвета
    static constexpr auto SELECTED_COLOR = sf::Color::Yellow;
    static constexpr auto NORMAL_COLOR = sf::Color::White;
    static constexpr auto VALUE_COLOR = sf::Color(150, 200, 255);
    static constexpr auto TITLE_COLOR = sf::Color(100, 200, 255);

    // Размеры и позиции
    static constexpr unsigned int TITLE_FONT_SIZE = 42;
    static constexpr unsigned int ITEM_FONT_SIZE = 28;
    static constexpr float ITEM_SPACING = 60.0f;
};

} // namespace core
