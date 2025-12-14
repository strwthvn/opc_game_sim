#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>
#include <memory>
#include <vector>
#include <string>

namespace core {

class ResourceManager;

/**
 * @brief Менеджер аудио системы
 *
 * Управляет воспроизведением звуков и музыки с использованием пула sf::Sound объектов.
 * Предоставляет контроль громкости для звуков, музыки и общей громкости.
 *
 * Использование:
 * @code
 * audioManager.playSound("click");
 * audioManager.playMusic("background", true);
 * audioManager.setMasterVolume(80.0f);
 * @endcode
 */
class AudioManager {
public:
    /**
     * @brief Конструктор
     * @param resourceManager Указатель на менеджер ресурсов для получения звуковых буферов
     * @param soundPoolSize Размер пула звуковых объектов (по умолчанию 16)
     */
    explicit AudioManager(ResourceManager* resourceManager, size_t soundPoolSize = 16);

    /**
     * @brief Деструктор
     */
    ~AudioManager();

    // Запрет копирования и перемещения
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) = delete;
    AudioManager& operator=(AudioManager&&) = delete;

    /**
     * @brief Воспроизводит звук
     * @param name Имя звука из ResourceManager
     * @param volume Громкость звука (0.0-100.0, по умолчанию 100.0)
     * @return true если звук начал воспроизводиться
     */
    bool playSound(const std::string& name, float volume = 100.0f);

    /**
     * @brief Воспроизводит музыку
     * @param name Имя музыкального файла (путь относительно assets/music/)
     * @param loop Зацикливать ли музыку (по умолчанию true)
     * @return true если музыка начала воспроизводиться
     */
    bool playMusic(const std::string& name, bool loop = true);

    /**
     * @brief Останавливает музыку
     */
    void stopMusic();

    /**
     * @brief Приостанавливает музыку
     */
    void pauseMusic();

    /**
     * @brief Возобновляет музыку
     */
    void resumeMusic();

    /**
     * @brief Останавливает все звуки
     */
    void stopAllSounds();

    /**
     * @brief Устанавливает общую громкость
     * @param volume Громкость (0.0-100.0)
     */
    void setMasterVolume(float volume);

    /**
     * @brief Устанавливает громкость звуковых эффектов
     * @param volume Громкость (0.0-100.0)
     */
    void setSoundVolume(float volume);

    /**
     * @brief Устанавливает громкость музыки
     * @param volume Громкость (0.0-100.0)
     */
    void setMusicVolume(float volume);

    /**
     * @brief Получает общую громкость
     * @return Громкость (0.0-100.0)
     */
    float getMasterVolume() const { return m_masterVolume; }

    /**
     * @brief Получает громкость звуковых эффектов
     * @return Громкость (0.0-100.0)
     */
    float getSoundVolume() const { return m_soundVolume; }

    /**
     * @brief Получает громкость музыки
     * @return Громкость (0.0-100.0)
     */
    float getMusicVolume() const { return m_musicVolume; }

    /**
     * @brief Проверяет, воспроизводится ли музыка
     * @return true если музыка воспроизводится
     */
    bool isMusicPlaying() const;

    /**
     * @brief Обновляет состояние аудио системы
     *
     * Очищает завершившиеся звуки из пула.
     * Должно вызываться каждый кадр.
     */
    void update();

private:
    /**
     * @brief Находит свободный слот в пуле звуков
     * @return Индекс свободного слота или -1 если все заняты
     */
    int findFreeSound() const;

    /**
     * @brief Вычисляет эффективную громкость звука с учетом всех множителей
     * @param baseVolume Базовая громкость звука
     * @return Итоговая громкость
     */
    float calculateEffectiveSoundVolume(float baseVolume) const;

    /**
     * @brief Вычисляет эффективную громкость музыки с учетом всех множителей
     * @return Итоговая громкость
     */
    float calculateEffectiveMusicVolume() const;

    ResourceManager* m_resourceManager;                ///< Указатель на менеджер ресурсов
    std::vector<std::unique_ptr<sf::Sound>> m_soundPool; ///< Пул звуковых объектов
    std::unique_ptr<sf::Music> m_music;               ///< Текущая музыка

    float m_masterVolume;                              ///< Общая громкость (0-100)
    float m_soundVolume;                               ///< Громкость звуков (0-100)
    float m_musicVolume;                               ///< Громкость музыки (0-100)
};

} // namespace core
