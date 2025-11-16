#include "core/PerformanceMetrics.h"
#include <algorithm>
#include <numeric>

namespace core {

PerformanceMetrics::PerformanceMetrics(size_t sampleSize)
    : m_sampleSize(sampleSize)
    , m_lastFrameTime(Clock::now())
    , m_lastUpdateTime(Clock::now())
    , m_frameCount(0)
    , m_updateCount(0)
    , m_cachedMinFPS(0.0)
    , m_cachedMaxFPS(0.0)
    , m_minMaxDirty(true) {
}

void PerformanceMetrics::recordFrame() {
    auto currentTime = Clock::now();
    m_frameTimes.push_back(currentTime);

    // Ограничиваем размер очереди
    if (m_frameTimes.size() > m_sampleSize) {
        m_frameTimes.pop_front();
    }

    m_lastFrameTime = currentTime;
    m_frameCount++;
    m_minMaxDirty = true;  // Помечаем кэш как устаревший
}

void PerformanceMetrics::recordUpdate() {
    auto currentTime = Clock::now();
    m_updateTimes.push_back(currentTime);

    // Ограничиваем размер очереди
    if (m_updateTimes.size() > m_sampleSize) {
        m_updateTimes.pop_front();
    }

    m_lastUpdateTime = currentTime;
    m_updateCount++;
}

double PerformanceMetrics::getFPS() const {
    if (m_frameTimes.size() < 2) {
        return 0.0;
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_frameTimes.back() - m_frameTimes.front()
    );

    double seconds = duration.count() / 1000000.0;
    return (m_frameTimes.size() - 1) / seconds;
}

double PerformanceMetrics::getUPS() const {
    if (m_updateTimes.size() < 2) {
        return 0.0;
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_updateTimes.back() - m_updateTimes.front()
    );

    double seconds = duration.count() / 1000000.0;
    return (m_updateTimes.size() - 1) / seconds;
}

double PerformanceMetrics::getAverageFrameTime() const {
    if (m_frameTimes.size() < 2) {
        return 0.0;
    }

    std::vector<double> frameTimes;
    for (size_t i = 1; i < m_frameTimes.size(); ++i) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            m_frameTimes[i] - m_frameTimes[i - 1]
        );
        frameTimes.push_back(duration.count() / 1000.0); // в миллисекундах
    }

    double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
    return sum / frameTimes.size();
}

void PerformanceMetrics::updateMinMaxCache() const {
    if (m_frameTimes.size() < 2) {
        m_cachedMinFPS = 0.0;
        m_cachedMaxFPS = 0.0;
        m_minMaxDirty = false;
        return;
    }

    std::vector<double> fps;
    for (size_t i = 1; i < m_frameTimes.size(); ++i) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            m_frameTimes[i] - m_frameTimes[i - 1]
        );
        double seconds = duration.count() / 1000000.0;
        if (seconds > 0.0) {
            fps.push_back(1.0 / seconds);
        }
    }

    if (fps.empty()) {
        m_cachedMinFPS = 0.0;
        m_cachedMaxFPS = 0.0;
    } else {
        m_cachedMinFPS = *std::min_element(fps.begin(), fps.end());
        m_cachedMaxFPS = *std::max_element(fps.begin(), fps.end());
    }

    m_minMaxDirty = false;
}

double PerformanceMetrics::getMinFPS() const {
    if (m_minMaxDirty) {
        updateMinMaxCache();
    }
    return m_cachedMinFPS;
}

double PerformanceMetrics::getMaxFPS() const {
    if (m_minMaxDirty) {
        updateMinMaxCache();
    }
    return m_cachedMaxFPS;
}

void PerformanceMetrics::reset() {
    m_frameTimes.clear();
    m_updateTimes.clear();
    m_lastFrameTime = Clock::now();
    m_lastUpdateTime = Clock::now();
    m_frameCount = 0;
    m_updateCount = 0;
    m_cachedMinFPS = 0.0;
    m_cachedMaxFPS = 0.0;
    m_minMaxDirty = true;
}

} // namespace core
