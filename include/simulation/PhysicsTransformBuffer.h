#pragma once

#include <core/Components.h>
#include <entt/entt.hpp>

#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_map>

/**
 * @file PhysicsTransformBuffer.h
 * @brief Double buffering для синхронизации трансформаций между потоками
 *
 * Обеспечивает безблокировочную передачу данных трансформаций из потока
 * физики в главный поток рендеринга.
 */

namespace simulation {

/**
 * @brief Данные трансформации для буферизации
 *
 * Хранит позицию и угол поворота для одной сущности.
 */
struct BufferedTransform {
    float x = 0.0f;           ///< Позиция X в пикселях
    float y = 0.0f;           ///< Позиция Y в пикселях
    float rotation = 0.0f;    ///< Угол поворота в градусах

    BufferedTransform() = default;

    BufferedTransform(float px, float py, float rot)
        : x(px), y(py), rotation(rot) {}

    explicit BufferedTransform(const core::TransformComponent& transform)
        : x(transform.x), y(transform.y), rotation(transform.rotation) {}

    /**
     * @brief Применить буферизованные данные к TransformComponent
     * @param transform Компонент для обновления
     */
    void applyTo(core::TransformComponent& transform) const {
        transform.x = x;
        transform.y = y;
        transform.rotation = rotation;
    }
};

/**
 * @brief Double buffer для трансформаций физических объектов
 *
 * Реализует паттерн double buffering для минимизации блокировок
 * между потоком физики и главным потоком:
 *
 * - **Буфер записи (write buffer)**: поток физики записывает новые позиции
 * - **Буфер чтения (read buffer)**: главный поток читает позиции для рендеринга
 * - **Swap**: атомарная смена буферов при синхронизации
 *
 * **Использование:**
 * @code
 * PhysicsTransformBuffer buffer;
 *
 * // В потоке физики (после каждого шага):
 * buffer.writeTransform(entity, x, y, rotation);
 *
 * // В главном потоке (перед рендерингом):
 * buffer.swapBuffers();
 * buffer.applyToRegistry(registry);
 * @endcode
 *
 * **Гарантии потокобезопасности:**
 * - writeTransform() может вызываться только из потока физики
 * - swapBuffers() и applyToRegistry() могут вызываться только из главного потока
 * - Между swapBuffers() и следующим writeTransform() буферы не перекрываются
 */
class PhysicsTransformBuffer {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    PhysicsTransformBuffer() = default;

    /**
     * @brief Записать трансформацию в буфер записи
     *
     * Вызывается из потока физики после обновления позиции тела.
     *
     * @param entity Сущность
     * @param x Позиция X в пикселях
     * @param y Позиция Y в пикселях
     * @param rotation Угол поворота в градусах
     *
     * @note Потокобезопасно относительно swapBuffers() (разные буферы).
     */
    void writeTransform(entt::entity entity, float x, float y, float rotation);

    /**
     * @brief Записать трансформацию из TransformComponent
     *
     * @param entity Сущность
     * @param transform Компонент трансформации
     */
    void writeTransform(entt::entity entity, const core::TransformComponent& transform);

    /**
     * @brief Поменять буферы местами
     *
     * Вызывается из главного потока в безопасной точке (например, в начале кадра).
     * После вызова буфер чтения содержит последние данные из потока физики.
     *
     * @note Требует краткой блокировки мьютекса для атомарности swap.
     */
    void swapBuffers();

    /**
     * @brief Применить буфер чтения к registry
     *
     * Обновляет TransformComponent всех сущностей из буфера чтения.
     * Вызывается из главного потока после swapBuffers().
     *
     * @param registry EnTT registry
     *
     * @note Не требует блокировки — работает только с буфером чтения.
     */
    void applyToRegistry(entt::registry& registry);

    /**
     * @brief Очистить оба буфера
     *
     * Удаляет все записи. Вызывается при перезапуске симуляции.
     */
    void clear();

    /**
     * @brief Удалить сущность из буферов
     *
     * Вызывается при удалении физического тела.
     *
     * @param entity Сущность для удаления
     */
    void removeEntity(entt::entity entity);

    /**
     * @brief Получить количество записей в буфере записи
     *
     * @return Количество сущностей
     */
    size_t getWriteBufferSize() const;

    /**
     * @brief Получить количество записей в буфере чтения
     *
     * @return Количество сущностей
     */
    size_t getReadBufferSize() const;

    /**
     * @brief Зарезервировать место в буферах
     *
     * Оптимизация для известного количества физических объектов.
     *
     * @param capacity Ожидаемое количество сущностей
     */
    void reserve(size_t capacity);

private:
    using TransformMap = std::unordered_map<entt::entity, BufferedTransform>;

    TransformMap m_bufferA;     ///< Буфер A
    TransformMap m_bufferB;     ///< Буфер B

    TransformMap* m_writeBuffer = &m_bufferA;   ///< Текущий буфер записи
    TransformMap* m_readBuffer = &m_bufferB;    ///< Текущий буфер чтения

    mutable std::mutex m_swapMutex;             ///< Мьютекс для swap операции
};

} // namespace simulation
