#include "core/AudioManager.h"
#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <algorithm>

namespace core {

AudioManager::AudioManager(ResourceManager* resourceManager, size_t soundPoolSize)
    : m_resourceManager(resourceManager)
    , m_music(std::make_unique<sf::Music>())
    , m_masterVolume(100.0f)
    , m_soundVolume(100.0f)
    , m_musicVolume(100.0f) {

    if (!m_resourceManager) {
        LOG_ERROR("AudioManager: ResourceManager is null");
        throw std::runtime_error("AudioManager: ResourceManager cannot be null");
    }

    // Резервируем место в пуле звуков (но не создаем объекты, так как у sf::Sound нет конструктора по умолчанию в SFML 3)
    m_soundPool.reserve(soundPoolSize);
    LOG_INFO("AudioManager initialized with sound pool capacity: {}", soundPoolSize);
}

AudioManager::~AudioManager() {
    stopMusic();
    stopAllSounds();
    LOG_DEBUG("AudioManager destroyed");
}

bool AudioManager::playSound(const std::string& name, float volume) {
    // Проверяем, что звук загружен
    if (!m_resourceManager->hasSound(name)) {
        LOG_WARN("AudioManager: Sound '{}' not found in ResourceManager", name);
        return false;
    }

    // Находим свободный слот
    int freeSlot = findFreeSound();
    if (freeSlot < 0) {
        LOG_WARN("AudioManager: No free sound slots available, cannot play '{}'", name);
        return false;
    }

    // Получаем звуковой буфер
    try {
        const sf::SoundBuffer& buffer = m_resourceManager->getSound(name);

        // Создаем новый звук с буфером (SFML 3 требует буфер в конструкторе)
        auto sound = std::make_unique<sf::Sound>(buffer);
        sound->setVolume(calculateEffectiveSoundVolume(volume));
        sound->play();

        // Если слот существует, заменяем, иначе добавляем
        if (freeSlot < static_cast<int>(m_soundPool.size())) {
            m_soundPool[freeSlot] = std::move(sound);
        } else {
            m_soundPool.push_back(std::move(sound));
        }

        LOG_DEBUG("AudioManager: Playing sound '{}' in slot {}", name, freeSlot);
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("AudioManager: Failed to play sound '{}': {}", name, e.what());
        return false;
    }
}

bool AudioManager::playMusic(const std::string& name, bool loop) {
    // Формируем полный путь к музыкальному файлу
    std::string musicPath = "assets/music/" + name;

    // Останавливаем текущую музыку
    stopMusic();

    // Загружаем и воспроизводим новую музыку
    if (!m_music->openFromFile(musicPath)) {
        LOG_ERROR("AudioManager: Failed to load music from '{}'", musicPath);
        return false;
    }

    m_music->setLooping(loop);
    m_music->setVolume(calculateEffectiveMusicVolume());
    m_music->play();

    LOG_INFO("AudioManager: Playing music '{}' (loop: {})", name, loop);
    return true;
}

void AudioManager::stopMusic() {
    if (m_music) {
        m_music->stop();
        LOG_DEBUG("AudioManager: Music stopped");
    }
}

void AudioManager::pauseMusic() {
    if (m_music) {
        m_music->pause();
        LOG_DEBUG("AudioManager: Music paused");
    }
}

void AudioManager::resumeMusic() {
    if (m_music && m_music->getStatus() == sf::Music::Status::Paused) {
        m_music->play();
        LOG_DEBUG("AudioManager: Music resumed");
    }
}

void AudioManager::stopAllSounds() {
    for (auto& sound : m_soundPool) {
        if (sound) {
            sound->stop();
        }
    }
    LOG_DEBUG("AudioManager: All sounds stopped");
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 100.0f);
    LOG_DEBUG("AudioManager: Master volume set to {}", m_masterVolume);

    // Обновляем громкость текущей музыки
    if (m_music) {
        m_music->setVolume(calculateEffectiveMusicVolume());
    }

    // Обновляем громкость всех активных звуков
    for (auto& sound : m_soundPool) {
        if (sound && sound->getStatus() == sf::Sound::Status::Playing) {
            // Пересчитываем громкость с новым мастер-значением
            float currentVolume = sound->getVolume();
            // Восстанавливаем базовую громкость и применяем новую
            float baseVolume = (currentVolume / (m_soundVolume / 100.0f)) / (m_masterVolume / 100.0f);
            sound->setVolume(calculateEffectiveSoundVolume(baseVolume));
        }
    }
}

void AudioManager::setSoundVolume(float volume) {
    m_soundVolume = std::clamp(volume, 0.0f, 100.0f);
    LOG_DEBUG("AudioManager: Sound volume set to {}", m_soundVolume);

    // Обновляем громкость всех активных звуков
    for (auto& sound : m_soundPool) {
        if (sound && sound->getStatus() == sf::Sound::Status::Playing) {
            float currentVolume = sound->getVolume();
            float baseVolume = (currentVolume / (m_soundVolume / 100.0f)) / (m_masterVolume / 100.0f);
            sound->setVolume(calculateEffectiveSoundVolume(baseVolume));
        }
    }
}

void AudioManager::setMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 100.0f);
    LOG_DEBUG("AudioManager: Music volume set to {}", m_musicVolume);

    // Обновляем громкость текущей музыки
    if (m_music) {
        m_music->setVolume(calculateEffectiveMusicVolume());
    }
}

bool AudioManager::isMusicPlaying() const {
    return m_music && m_music->getStatus() == sf::Music::Status::Playing;
}

void AudioManager::update() {
    // Очищаем завершившиеся звуки (не обязательно, но полезно для освобождения ресурсов)
    // В SFML звуки автоматически переходят в состояние Stopped, так что просто логируем статистику

    // Можно добавить периодическую статистику
    static int updateCounter = 0;
    if (++updateCounter >= 600) { // Раз в ~10 секунд при 60 FPS
        int activeSounds = 0;
        for (const auto& sound : m_soundPool) {
            if (sound && sound->getStatus() == sf::Sound::Status::Playing) {
                ++activeSounds;
            }
        }
        if (activeSounds > 0) {
            LOG_DEBUG("AudioManager: {} active sounds in pool", activeSounds);
        }
        updateCounter = 0;
    }
}

int AudioManager::findFreeSound() const {
    for (size_t i = 0; i < m_soundPool.size(); ++i) {
        if (!m_soundPool[i] || m_soundPool[i]->getStatus() != sf::Sound::Status::Playing) {
            return static_cast<int>(i);
        }
    }
    // Если все слоты заняты, возвращаем следующий индекс (будет добавлен новый слот)
    return static_cast<int>(m_soundPool.size());
}

float AudioManager::calculateEffectiveSoundVolume(float baseVolume) const {
    float effective = baseVolume * (m_soundVolume / 100.0f) * (m_masterVolume / 100.0f);
    return std::clamp(effective, 0.0f, 100.0f);
}

float AudioManager::calculateEffectiveMusicVolume() const {
    float effective = m_musicVolume * (m_masterVolume / 100.0f);
    return std::clamp(effective, 0.0f, 100.0f);
}

} // namespace core
