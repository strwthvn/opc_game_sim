/**
 * @file EventBusExample.cpp
 * @brief Пример использования EventBus для подписки на события и воспроизведения звуков
 *
 * Этот пример демонстрирует:
 * 1. Подписку на CollisionEvent
 * 2. Воспроизведение звука при коллизии
 * 3. Подписку на StateChangedEvent
 * 4. Логирование изменений состояния
 */

#include "core/EventBus.h"
#include "core/AudioManager.h"
#include "core/Logger.h"
#include "core/Components.h"
#include <entt/entt.hpp>

using namespace core;

/**
 * @brief Пример подписки на события коллизии с воспроизведением звука
 *
 * Этот класс демонстрирует, как подписаться на CollisionEvent
 * и воспроизвести звук при столкновении.
 */
class CollisionSoundHandler {
public:
    /**
     * @brief Конструктор
     * @param audioManager Менеджер аудио для воспроизведения звуков
     */
    explicit CollisionSoundHandler(AudioManager& audioManager)
        : m_audioManager(audioManager) {

        // Подписываемся на события коллизии
        m_collisionConnection = EventBus::getInstance().subscribe<CollisionEvent>(
            [this](const CollisionEvent& evt) {
                this->onCollision(evt);
            }
        );

        LOG_INFO("CollisionSoundHandler: Subscribed to CollisionEvent");
    }

    /**
     * @brief Деструктор (автоматически отписывается благодаря RAII)
     */
    ~CollisionSoundHandler() {
        // Connection автоматически отписывается при уничтожении
        LOG_INFO("CollisionSoundHandler: Unsubscribed from CollisionEvent");
    }

private:
    /**
     * @brief Обработчик события коллизии
     * @param evt Событие коллизии
     */
    void onCollision(const CollisionEvent& evt) {
        // Воспроизводим звук только при начале коллизии (не Stay/Exit)
        if (evt.type == CollisionEvent::Type::Enter) {
            LOG_INFO("Collision detected: Entity {} <-> Entity {} at ({}, {})",
                static_cast<uint32_t>(evt.entityA),
                static_cast<uint32_t>(evt.entityB),
                evt.contactPoint.x,
                evt.contactPoint.y
            );

            // Воспроизводим звук столкновения
            m_audioManager.playSound("collision.wav");
        }
    }

    AudioManager& m_audioManager;
    EventBus::Connection m_collisionConnection;
};

/**
 * @brief Пример подписки на события изменения состояния
 *
 * Логирует все изменения состояния сущностей в системе.
 */
class StateChangeLogger {
public:
    StateChangeLogger() {
        // Подписываемся на события изменения состояния
        m_stateConnection = EventBus::getInstance().subscribe<StateChangedEvent>(
            [this](const StateChangedEvent& evt) {
                this->onStateChanged(evt);
            }
        );

        LOG_INFO("StateChangeLogger: Subscribed to StateChangedEvent");
    }

    ~StateChangeLogger() {
        LOG_INFO("StateChangeLogger: Unsubscribed from StateChangedEvent");
    }

private:
    void onStateChanged(const StateChangedEvent& evt) {
        LOG_INFO("State changed for entity {}: '{}' -> '{}'",
            static_cast<uint32_t>(evt.entity),
            evt.previousState,
            evt.newState
        );

        // Можно добавить дополнительную логику в зависимости от состояния
        if (evt.newState == "error") {
            LOG_WARN("Entity {} entered ERROR state!", static_cast<uint32_t>(evt.entity));
        }
    }

    EventBus::Connection m_stateConnection;
};

/**
 * @brief Пример создания игровой сцены с обработчиками событий
 */
void exampleGameScene() {
    // Инициализируем менеджеры
    AudioManager audioManager;

    // Загружаем звуковой ресурс
    audioManager.loadSound("collision.wav", "assets/sounds/collision.wav");

    // Создаем обработчики событий
    CollisionSoundHandler collisionHandler(audioManager);
    StateChangeLogger stateLogger;

    // Создаем ECS реестр
    entt::registry registry;

    // Создаем две сущности, которые будут сталкиваться
    auto entityA = registry.create();
    registry.emplace<NameComponent>(entityA, "ObjectA");
    registry.emplace<TransformComponent>(entityA, 100.0f, 100.0f);
    registry.emplace<CollisionComponent>(entityA, true, false, "default");

    auto entityB = registry.create();
    registry.emplace<NameComponent>(entityB, "ObjectB");
    registry.emplace<TransformComponent>(entityB, 100.0f, 100.0f);
    registry.emplace<CollisionComponent>(entityB, true, false, "default");

    // Симулируем коллизию (в реальной игре это делает CollisionSystem)
    LOG_INFO("Simulating collision between ObjectA and ObjectB...");
    CollisionEvent collisionEvent(entityA, entityB, CollisionEvent::Type::Enter, sf::Vector2f(100.0f, 100.0f));
    EventBus::getInstance().publish(collisionEvent);
    // Звук "collision.wav" будет воспроизведен автоматически через collisionHandler

    // Создаем сущность с FSM компонентом
    auto machine = registry.create();
    registry.emplace<NameComponent>(machine, "IndustrialMachine");
    auto& fsm = registry.emplace<EntityStateComponent>(machine, "idle");

    // Регистрируем коллбеки для состояний
    fsm.registerOnEnter("running", []() {
        LOG_INFO("Machine started running!");
    });

    fsm.registerOnExit("running", []() {
        LOG_INFO("Machine stopped running!");
    });

    // Меняем состояние (публикует StateChangedEvent)
    LOG_INFO("Changing machine state: idle -> running");
    fsm.setState("running", machine);
    // StateChangeLogger автоматически залогирует изменение

    LOG_INFO("Changing machine state: running -> error");
    fsm.setState("error", machine);
    // StateChangeLogger залогирует предупреждение об ошибке

    // Очистка EventBus при завершении
    EventBus::getInstance().clear();
}

/**
 * @brief Пример множественных подписчиков на одно событие
 */
void exampleMultipleSubscribers() {
    // Подписчик 1: Воспроизведение звука
    auto connection1 = EventBus::getInstance().subscribe<CollisionEvent>(
        [](const CollisionEvent& evt) {
            LOG_INFO("Subscriber 1: Playing collision sound");
        }
    );

    // Подписчик 2: Создание визуального эффекта
    auto connection2 = EventBus::getInstance().subscribe<CollisionEvent>(
        [](const CollisionEvent& evt) {
            LOG_INFO("Subscriber 2: Spawning particle effect at ({}, {})",
                evt.contactPoint.x, evt.contactPoint.y);
        }
    );

    // Подписчик 3: Учет статистики
    auto connection3 = EventBus::getInstance().subscribe<CollisionEvent>(
        [](const CollisionEvent& evt) {
            static int collisionCount = 0;
            collisionCount++;
            LOG_INFO("Subscriber 3: Total collisions: {}", collisionCount);
        }
    );

    // Публикуем событие - все 3 подписчика получат уведомление
    entt::registry registry;
    auto e1 = registry.create();
    auto e2 = registry.create();

    CollisionEvent evt(e1, e2, CollisionEvent::Type::Enter);
    EventBus::getInstance().publish(evt);

    // Вывод:
    // Subscriber 1: Playing collision sound
    // Subscriber 2: Spawning particle effect at (0, 0)
    // Subscriber 3: Total collisions: 1

    // Отписываемся (автоматически при выходе из scope, но можно и вручную)
    connection1.disconnect();
    connection2.disconnect();
    connection3.disconnect();
}

/**
 * @brief Пример условной обработки событий
 */
void exampleConditionalHandling() {
    entt::registry registry;

    // Подписываемся на коллизии, но обрабатываем только коллизии с "player"
    auto connection = EventBus::getInstance().subscribe<CollisionEvent>(
        [&registry](const CollisionEvent& evt) {
            // Проверяем, является ли одна из сущностей игроком
            auto* tagA = registry.try_get<TagComponent>(evt.entityA);
            auto* tagB = registry.try_get<TagComponent>(evt.entityB);

            bool isPlayerCollision = (tagA && tagA->tag == "player") ||
                                     (tagB && tagB->tag == "player");

            if (isPlayerCollision && evt.type == CollisionEvent::Type::Enter) {
                LOG_INFO("Player collision detected!");
                // Обработка коллизии игрока (урон, подбор предметов, и т.д.)
            }
        }
    );

    // Создаем игрока и врага
    auto player = registry.create();
    registry.emplace<TagComponent>(player, "player");

    auto enemy = registry.create();
    registry.emplace<TagComponent>(enemy, "enemy");

    // Симулируем коллизию игрока с врагом
    CollisionEvent playerEnemyCollision(player, enemy, CollisionEvent::Type::Enter);
    EventBus::getInstance().publish(playerEnemyCollision);
    // Вывод: "Player collision detected!"

    // Симулируем коллизию двух врагов (не обработается)
    auto enemy2 = registry.create();
    registry.emplace<TagComponent>(enemy2, "enemy");

    CollisionEvent enemyEnemyCollision(enemy, enemy2, CollisionEvent::Type::Enter);
    EventBus::getInstance().publish(enemyEnemyCollision);
    // Нет вывода, так как это не коллизия игрока

    connection.disconnect();
}

/**
 * @brief Главная функция с примерами
 */
int main() {
    // Инициализируем логгер
    Logger::initialize();

    LOG_INFO("=== EventBus Examples ===");

    LOG_INFO("\n--- Example 1: Game Scene with Event Handlers ---");
    exampleGameScene();

    LOG_INFO("\n--- Example 2: Multiple Subscribers ---");
    exampleMultipleSubscribers();

    LOG_INFO("\n--- Example 3: Conditional Event Handling ---");
    exampleConditionalHandling();

    LOG_INFO("\n=== All examples completed successfully ===");

    return 0;
}
