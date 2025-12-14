#include "core/states/SettingsState.h"
#include "core/StateManager.h"
#include "core/ResourceManager.h"
#include "core/AudioManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace core {

SettingsState::SettingsState(StateManager* stateManager)
    : State(stateManager)
    , m_font(nullptr)
    , m_selectedItem(0)
    , m_fontLoaded(false)
    , m_currentResolutionIndex(0)
    , m_vsyncEnabled(true)
    , m_masterVolume(100)
    , m_settingsChanged(false) {

    // Инициализируем список доступных разрешений
    m_availableResolutions = {
        {800, 600, "800x600"},
        {1024, 768, "1024x768"},
        {1280, 720, "1280x720"},
        {1366, 768, "1366x768"},
        {1920, 1080, "1920x1080"},
        {2560, 1440, "2560x1440"}
    };

    // Определяем текущее разрешение
    auto currentSize = getWindowSize();
    for (size_t i = 0; i < m_availableResolutions.size(); ++i) {
        if (m_availableResolutions[i].width == currentSize.x &&
            m_availableResolutions[i].height == currentSize.y) {
            m_currentResolutionIndex = i;
            break;
        }
    }
}

void SettingsState::onEnter() {
    LOG_INFO("Entering SettingsState");

    // Загружаем текущие настройки громкости из AudioManager
    auto* audioManager = getAudioManager();
    if (audioManager) {
        m_masterVolume = static_cast<int>(audioManager->getMasterVolume());
        LOG_DEBUG("Loaded current master volume: {}", m_masterVolume);
    }

    // Инициализируем UI View размером окна (1:1 пиксели)
    auto windowSize = getWindowSize();
    m_uiView.setSize(sf::Vector2f(windowSize.x, windowSize.y));
    m_uiView.setCenter(sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f));
    LOG_DEBUG("SettingsState UI View initialized: {}x{}", windowSize.x, windowSize.y);

    initializeUI();
    LOG_DEBUG("SettingsState::onEnter() completed. Font loaded: {}", m_fontLoaded);
}

void SettingsState::onExit() {
    LOG_INFO("Exiting SettingsState");

    // Применяем настройки при выходе, если они были изменены
    if (m_settingsChanged) {
        LOG_INFO("Applying settings before exit");
        applySettings();
    }
}

bool SettingsState::handleEvent(const sf::Event& event) {
    // Обработка нажатий клавиш
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Up:
                moveUp();
                return true;

            case sf::Keyboard::Key::Down:
                moveDown();
                return true;

            case sf::Keyboard::Key::Left:
                moveLeft();
                return true;

            case sf::Keyboard::Key::Right:
                moveRight();
                return true;

            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                activateSelected();
                return true;

            case sf::Keyboard::Key::Escape:
                // Escape = возврат в меню
                LOG_INFO("Returning to menu from settings");
                m_stateManager->popState();
                return true;

            default:
                break;
        }
    }

    return false;
}

void SettingsState::update(double dt) {
    (void)dt; // Пока нет анимаций
}

void SettingsState::onWindowResize(const sf::Vector2u& newSize) {
    LOG_DEBUG("SettingsState received window resize event: {}x{}", newSize.x, newSize.y);

    // Обновляем UI View размером окна
    m_uiView.setSize(sf::Vector2f(newSize.x, newSize.y));
    m_uiView.setCenter(sf::Vector2f(newSize.x / 2.0f, newSize.y / 2.0f));

    // Пересоздаем UI с новыми позициями
    if (m_fontLoaded) {
        initializeUI();
    }
}

void SettingsState::render(sf::RenderWindow& window) {
    // Устанавливаем фиксированный UI View для рендеринга меню
    window.setView(m_uiView);

    // Не рендерим текст если шрифт не загрузился
    if (!m_fontLoaded) {
        LOG_WARN("SettingsState::render() - font not loaded");
        return;
    }

    // Рисуем заголовок
    if (m_title) {
        window.draw(*m_title);
    }

    // Рисуем метки настроек
    for (const auto& label : m_settingLabels) {
        if (label) {
            window.draw(*label);
        }
    }

    // Рисуем значения настроек
    for (const auto& value : m_settingValues) {
        if (value) {
            window.draw(*value);
        }
    }
}

void SettingsState::initializeUI() {
    LOG_DEBUG("SettingsState::initializeUI() called");

    // Получаем шрифт из ResourceManager
    auto* resources = getResourceManager();
    if (!resources) {
        LOG_ERROR("ResourceManager not available in SettingsState");
        return;
    }

    try {
        m_font = &resources->getFont("default");
        m_fontLoaded = true;
        LOG_INFO("Font loaded successfully in SettingsState");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load default font for SettingsState: {}", e.what());
        m_fontLoaded = false;
        return;
    }

    // Получаем текущий размер окна для относительного позиционирования
    auto windowSize = getWindowSize();
    float centerX = windowSize.x / 2.0f;

    // Настройка заголовка
    m_title = std::make_unique<sf::Text>(*m_font);
    m_title->setString("SETTINGS");
    m_title->setCharacterSize(TITLE_FONT_SIZE);
    m_title->setFillColor(TITLE_COLOR);

    // Центрируем заголовок
    auto titleBounds = m_title->getLocalBounds();
    float titleX = centerX - titleBounds.size.x / 2.0f;
    float titleY = windowSize.y * 0.1f;
    m_title->setPosition(sf::Vector2f(titleX, titleY));
    LOG_DEBUG("Title created at ({}, {})", titleX, titleY);

    // Создание пунктов настроек
    const std::vector<std::string> itemLabels = {
        "Resolution:",
        "VSync:",
        "Master Volume:",
        "Controls:",
        "< Back >"
    };

    m_settingLabels.clear();
    m_settingValues.clear();

    float settingsStartY = windowSize.y * 0.3f;
    float labelX = centerX - 250.0f; // Метка слева от центра
    float valueX = centerX + 50.0f;  // Значение справа от центра

    for (size_t i = 0; i < itemLabels.size(); ++i) {
        // Создаем метку
        auto label = std::make_unique<sf::Text>(*m_font);
        label->setString(itemLabels[i]);
        label->setCharacterSize(ITEM_FONT_SIZE);
        float itemY = settingsStartY + i * ITEM_SPACING;
        label->setPosition(sf::Vector2f(labelX, itemY));
        m_settingLabels.push_back(std::move(label));

        // Создаем значение (если не Back)
        auto value = std::make_unique<sf::Text>(*m_font);
        value->setCharacterSize(ITEM_FONT_SIZE);
        value->setFillColor(VALUE_COLOR);
        value->setPosition(sf::Vector2f(valueX, itemY));

        if (i < static_cast<size_t>(SettingsItem::Back)) {
            value->setString(getSettingValueString(static_cast<SettingsItem>(i)));
        }

        m_settingValues.push_back(std::move(value));

        LOG_DEBUG("Settings item {} '{}' created at ({}, {})", i, itemLabels[i], labelX, itemY);
    }

    updateMenuColors();
    LOG_INFO("SettingsState UI initialized with {} items", m_settingLabels.size());
}

void SettingsState::moveUp() {
    m_selectedItem--;
    if (m_selectedItem < 0) {
        m_selectedItem = static_cast<int>(SettingsItem::Count) - 1;
    }
    updateMenuColors();
    LOG_DEBUG("Settings selection: {}", m_selectedItem);
}

void SettingsState::moveDown() {
    m_selectedItem++;
    if (m_selectedItem >= static_cast<int>(SettingsItem::Count)) {
        m_selectedItem = 0;
    }
    updateMenuColors();
    LOG_DEBUG("Settings selection: {}", m_selectedItem);
}

void SettingsState::moveLeft() {
    auto selectedSetting = static_cast<SettingsItem>(m_selectedItem);

    switch (selectedSetting) {
        case SettingsItem::Resolution:
            if (m_currentResolutionIndex > 0) {
                m_currentResolutionIndex--;
                m_settingsChanged = true;
                updateSettingsDisplay();
                LOG_DEBUG("Resolution changed to: {}",
                         m_availableResolutions[m_currentResolutionIndex].label);
            }
            break;

        case SettingsItem::VSync:
            m_vsyncEnabled = !m_vsyncEnabled;
            m_settingsChanged = true;
            updateSettingsDisplay();
            LOG_DEBUG("VSync toggled: {}", m_vsyncEnabled);
            break;

        case SettingsItem::MasterVolume:
            if (m_masterVolume > 0) {
                m_masterVolume = std::max(0, m_masterVolume - 10);
                m_settingsChanged = true;
                updateSettingsDisplay();
                LOG_DEBUG("Master volume changed to: {}", m_masterVolume);
            }
            break;

        default:
            break;
    }
}

void SettingsState::moveRight() {
    auto selectedSetting = static_cast<SettingsItem>(m_selectedItem);

    switch (selectedSetting) {
        case SettingsItem::Resolution:
            if (m_currentResolutionIndex < m_availableResolutions.size() - 1) {
                m_currentResolutionIndex++;
                m_settingsChanged = true;
                updateSettingsDisplay();
                LOG_DEBUG("Resolution changed to: {}",
                         m_availableResolutions[m_currentResolutionIndex].label);
            }
            break;

        case SettingsItem::VSync:
            m_vsyncEnabled = !m_vsyncEnabled;
            m_settingsChanged = true;
            updateSettingsDisplay();
            LOG_DEBUG("VSync toggled: {}", m_vsyncEnabled);
            break;

        case SettingsItem::MasterVolume:
            if (m_masterVolume < 100) {
                m_masterVolume = std::min(100, m_masterVolume + 10);
                m_settingsChanged = true;
                updateSettingsDisplay();
                LOG_DEBUG("Master volume changed to: {}", m_masterVolume);
            }
            break;

        default:
            break;
    }
}

void SettingsState::activateSelected() {
    auto selectedSetting = static_cast<SettingsItem>(m_selectedItem);

    if (selectedSetting == SettingsItem::Back) {
        LOG_INFO("Back button activated, returning to menu");
        m_stateManager->popState();
    }
}

void SettingsState::applySettings() {
    LOG_INFO("Applying settings:");
    LOG_INFO("  Resolution: {}", m_availableResolutions[m_currentResolutionIndex].label);
    LOG_INFO("  VSync: {}", m_vsyncEnabled);
    LOG_INFO("  Master Volume: {}", m_masterVolume);

    // Применяем громкость через AudioManager
    auto* audioManager = getAudioManager();
    if (audioManager) {
        audioManager->setMasterVolume(static_cast<float>(m_masterVolume));
        LOG_INFO("Applied master volume: {}", m_masterVolume);
    } else {
        LOG_WARN("AudioManager not available, cannot apply volume settings");
    }

    // TODO: Применение остальных настроек
    // Для разрешения потребуется рестарт окна или пересоздание
    // VSync можно применить через window.setVerticalSyncEnabled()

    LOG_WARN("Resolution/VSync application not implemented yet - requires restart");
    m_settingsChanged = false;
}

void SettingsState::updateSettingsDisplay() {
    for (size_t i = 0; i < m_settingValues.size() && i < static_cast<size_t>(SettingsItem::Back); ++i) {
        if (m_settingValues[i]) {
            m_settingValues[i]->setString(getSettingValueString(static_cast<SettingsItem>(i)));
        }
    }
}

void SettingsState::updateMenuColors() {
    for (size_t i = 0; i < m_settingLabels.size(); ++i) {
        if (m_settingLabels[i]) {
            if (static_cast<int>(i) == m_selectedItem) {
                m_settingLabels[i]->setFillColor(SELECTED_COLOR);
            } else {
                m_settingLabels[i]->setFillColor(NORMAL_COLOR);
            }
        }
    }
}

std::string SettingsState::getSettingValueString(SettingsItem item) const {
    switch (item) {
        case SettingsItem::Resolution:
            return "< " + m_availableResolutions[m_currentResolutionIndex].label + " >";

        case SettingsItem::VSync:
            return m_vsyncEnabled ? "< ON >" : "< OFF >";

        case SettingsItem::MasterVolume:
            return "< " + std::to_string(m_masterVolume) + "% >";

        case SettingsItem::Controls:
            return "WASD/Arrows, Enter, Esc";

        default:
            return "";
    }
}

} // namespace core
