#pragma once

#include <chrono>
#include <deque>

namespace core {

/**
 * @brief Отслеживает метрики производительности приложения
 *
 * Предоставляет информацию о:
 * - FPS (Frames Per Second) - частота рендеринга
 * - UPS (Updates Per Second) - частота обновления логики
 * - Frame time - время на кадр
 * - Усредненные значения за последние N кадров
 */
class PerformanceMetrics {
public:
    /**
     * @brief Конструктор
     * @param sampleSize Количество кадров для усреднения (по умолчанию 60)
     */
    explicit PerformanceMetrics(size_t sampleSize = 60);

    /**
     * @brief Регистрирует новый кадр
     */
    void recordFrame();

    /**
     * @brief Регистрирует обновление логики
     */
    void recordUpdate();

    /**
     * @brief Возвращает текущий FPS
     * @return Частота кадров в секунду
     */
    double getFPS() const;

    /**
     * @brief Возвращает текущий UPS
     * @return Частота обновлений в секунду
     */
    double getUPS() const;

    /**
     * @brief Возвращает среднее время кадра
     * @return Время кадра в миллисекундах
     */
    double getAverageFrameTime() const;

    /**
     * @brief Возвращает минимальный FPS за период
     * @return Минимальный FPS
     */
    double getMinFPS() const;

    /**
     * @brief Возвращает максимальный FPS за период
     * @return Максимальный FPS
     */
    double getMaxFPS() const;

    /**
     * @brief Сбрасывает все метрики
     */
    void reset();

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    /**
     * @brief Обновляет кэшированные min/max FPS значения
     */
    void updateMinMaxCache() const;

    size_t m_sampleSize;                           ///< Размер выборки для усреднения
    std::deque<TimePoint> m_frameTimes;           ///< Времена кадров
    std::deque<TimePoint> m_updateTimes;          ///< Времена обновлений
    TimePoint m_lastFrameTime;                    ///< Время последнего кадра
    TimePoint m_lastUpdateTime;                   ///< Время последнего обновления

    size_t m_frameCount;                          ///< Счетчик кадров
    size_t m_updateCount;                         ///< Счетчик обновлений

    // Кэш для min/max FPS (помечены mutable для использования в const методах)
    mutable double m_cachedMinFPS;                ///< Кэшированный минимальный FPS
    mutable double m_cachedMaxFPS;                ///< Кэшированный максимальный FPS
    mutable bool m_minMaxDirty;                   ///< Флаг необходимости пересчета min/max
};

} // namespace core
