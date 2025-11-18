#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <entt/entt.hpp>
#include <memory>

namespace core {

// Forward declarations
class RenderSystem;
class UpdateSystem;
class LifetimeSystem;

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
    ~GameState() override;

    void onEnter() override;
    void onExit() override;
    bool handleEvent(const sf::Event& event) override;
    void update(double dt) override;
    void render(sf::RenderWindow& window) override;
    void onWindowResize(const sf::Vector2u& newSize) override;
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

    // ECS
    entt::registry m_registry;                     ///< EnTT registry для управления сущностями
    std::unique_ptr<RenderSystem> m_renderSystem;  ///< Система рендеринга
    std::unique_ptr<UpdateSystem> m_updateSystem;  ///< Система обновления
    std::unique_ptr<LifetimeSystem> m_lifetimeSystem;  ///< Система времени жизни

    // Views для рендеринга
    sf::View m_worldView;                      ///< View для игрового мира (расширяется с окном)
    sf::View m_uiView;                         ///< View для UI (фиксированный)

    // Размеры и позиции UI (в координатах фиксированного UI View)
    static constexpr unsigned int UI_WIDTH = 1280;
    static constexpr unsigned int UI_HEIGHT = 720;
    static constexpr unsigned int INFO_TEXT_FONT_SIZE = 24;
    static constexpr float INFO_TEXT_X = 20.0f;
    static constexpr float INFO_TEXT_Y = 20.0f;
};

} // namespace core
