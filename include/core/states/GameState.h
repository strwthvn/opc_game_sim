#pragma once

#include "core/State.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <future>

namespace core {

// Forward declarations
class RenderSystem;
class UpdateSystem;
class LifetimeSystem;
class CollisionSystem;
class FSMSystem;
class TilePositionSystem;
class AnimationSystem;
class AnimationSystemV2;
class OverlaySystem;
}

namespace simulation {
class PhysicsWorld;
class PhysicsSystem;
class PhysicsThread;
}

namespace rendering {
class TileMapSystem;
class PhysicsDebugDraw;
}

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

    /**
     * @brief Создает тестовую сцену с тайловыми объектами
     * Демонстрирует работу тайловой системы (Milestone 1.3)
     */
    void createTileTestScene();

    /**
     * @brief Запускает асинхронную загрузку всех необходимых ресурсов
     */
    void startAsyncResourceLoading();

    /**
     * @brief Проверяет, завершена ли загрузка ресурсов
     * @return true если все ресурсы загружены
     */
    bool areResourcesLoaded() const;

    const sf::Font* m_font;                    ///< Указатель на шрифт из ResourceManager
    std::unique_ptr<sf::Text> m_infoText;      ///< Информационный текст
    bool m_fontLoaded;                         ///< Флаг загрузки шрифта

    // Асинхронная загрузка ресурсов
    bool m_resourcesLoaded;                    ///< Флаг завершения загрузки ресурсов
    bool m_sceneInitialized;                   ///< Флаг инициализации сцены
    std::future<size_t> m_loadingFuture;       ///< Future для отслеживания загрузки
    float m_loadingProgress;                   ///< Прогресс загрузки (0.0 - 1.0)
    std::unique_ptr<sf::Text> m_loadingText;   ///< Текст экрана загрузки

    // Временные переменные для демонстрации
    double m_elapsedTime;                      ///< Время с начала игры
    int m_updateCount;                         ///< Счетчик обновлений

    // ECS
    entt::registry m_registry;                     ///< EnTT registry для управления сущностями
    std::unique_ptr<RenderSystem> m_renderSystem;  ///< Система рендеринга
    std::unique_ptr<UpdateSystem> m_updateSystem;  ///< Система обновления
    std::unique_ptr<LifetimeSystem> m_lifetimeSystem;  ///< Система времени жизни
    std::unique_ptr<CollisionSystem> m_collisionSystem;  ///< Система коллизий (AABB)
    std::unique_ptr<FSMSystem> m_fsmSystem;        ///< Система конечных автоматов (FSM)

    // Tile System (Milestone 1.3)
    std::unique_ptr<TilePositionSystem> m_tilePositionSystem;  ///< Система синхронизации тайловых позиций
    std::unique_ptr<AnimationSystem> m_animationSystem;        ///< Система анимации спрайтов (старая)
    std::unique_ptr<AnimationSystemV2> m_animationSystemV2;    ///< Система анимации спрайтов V2 (JSON метаданные)
    std::unique_ptr<OverlaySystem> m_overlaySystem;            ///< Система синхронизации оверлеев
    std::unique_ptr<rendering::TileMapSystem> m_tileMapSystem; ///< Система рендеринга тайловых карт (TMX)

    // Views для рендеринга
    sf::View m_worldView;                      ///< View для игрового мира (расширяется с окном)
    sf::View m_uiView;                         ///< View для UI (фиксированный)

    // Управление камерой (константы заменены на Config)
    float m_cameraZoom;                        ///< Текущий зум камеры

    // Physics (Milestone 2.1)
    std::unique_ptr<simulation::PhysicsWorld> m_physicsWorld;    ///< Физический мир Box2D
    std::unique_ptr<simulation::PhysicsSystem> m_physicsSystem;  ///< Система физики
    std::unique_ptr<simulation::PhysicsThread> m_physicsThread;  ///< Поток физики (Task 6)

    // Отладочная визуализация
    bool m_debugDrawGrid;                      ///< Флаг отрисовки тайловой сетки (F6)
    bool m_debugDrawPhysics = false;           ///< Флаг отрисовки физики (F7)
    std::unique_ptr<rendering::PhysicsDebugDraw> m_physicsDebugDraw;  ///< Debug draw для Box2D
    void drawDebugGrid(sf::RenderWindow& window);  ///< Отрисовка отладочной сетки

    // Размеры и позиции UI (в координатах фиксированного UI View)
    static constexpr unsigned int UI_WIDTH = 1280;
    static constexpr unsigned int UI_HEIGHT = 720;
    static constexpr unsigned int INFO_TEXT_FONT_SIZE = 24;
    static constexpr float INFO_TEXT_X = 20.0f;
    static constexpr float INFO_TEXT_Y = 20.0f;
};

} // namespace core
