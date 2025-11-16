#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace core {

/**
 * @brief Состояние игрового процесса
 *
 * Основное состояние, в котором происходит симуляция и игровой процесс
 * Здесь будут обновляться:
 * - ECS системы
 * - Физика (Box2D)
 * - OPC UA интеграция
 * - Рендеринг игровых объектов
 */
class GameState : public State {
public:
    /**
     * @brief Конструктор
     * @param stateManager Указатель на менеджер состояний
     */
    explicit GameState(StateManager* stateManager);

    /**
     * @brief Деструктор
     */
    ~GameState() override = default;

    void onEnter() override;
    void onExit() override;
    bool handleEvent(const sf::Event& event) override;
    void update(double dt) override;
    void render(sf::RenderWindow& window) override;
    std::string getName() const override { return "GameState"; }

    // GameState должно рендериться под паузой
    bool renderBelow() const override { return true; }

private:
    /**
     * @brief Инициализирует игровую сцену
     */
    void initializeScene();

    const sf::Font* m_font;                    ///< Указатель на шрифт из ResourceManager
    std::unique_ptr<sf::Text> m_infoText;      ///< Информационный текст
    bool m_fontLoaded;                         ///< Флаг загрузки шрифта

    // Временные переменные для демонстрации
    double m_elapsedTime;                      ///< Время с начала игры
    int m_updateCount;                         ///< Счетчик обновлений

    // Размеры и позиции UI
    static constexpr unsigned int INFO_TEXT_FONT_SIZE = 24;
    static constexpr float INFO_TEXT_X = 20.0f;
    static constexpr float INFO_TEXT_Y = 20.0f;
};

} // namespace core
