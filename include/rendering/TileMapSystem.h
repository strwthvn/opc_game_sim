#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration для tmxlite
namespace tmx {
class Map;
class Tileset;
class TileLayer;
class ObjectGroup;
} // namespace tmx

namespace rendering {

/**
 * @brief Система управления тайловой картой (TMX)
 *
 * Отвечает за загрузку TMX карт через tmxlite,
 * управление тайлсетами и рендеринг карты с frustum culling.
 */
class TileMapSystem {
public:
    TileMapSystem();
    ~TileMapSystem();

    // Запретить копирование
    TileMapSystem(const TileMapSystem&) = delete;
    TileMapSystem& operator=(const TileMapSystem&) = delete;

    /**
     * @brief Загрузить TMX карту из файла
     * @param tmxPath Путь к TMX файлу
     * @return true если загрузка успешна
     */
    bool loadMap(const std::string& tmxPath);

    /**
     * @brief Выгрузить текущую карту
     */
    void unloadMap();

    /**
     * @brief Отрендерить видимые тайлы карты
     * @param target Цель рендеринга (окно или текстура)
     * @param camera Вид камеры для frustum culling
     */
    void render(sf::RenderTarget& target, const sf::View& camera);

    /**
     * @brief Получить ширину карты в тайлах
     * @return Ширина карты
     */
    int getMapWidth() const { return m_mapWidth; }

    /**
     * @brief Получить высоту карты в тайлах
     * @return Высота карты
     */
    int getMapHeight() const { return m_mapHeight; }

    /**
     * @brief Проверить, загружена ли карта
     * @return true если карта загружена
     */
    bool isLoaded() const { return m_loaded; }

private:
    /**
     * @brief Структура тайлсета
     */
    struct Tileset {
        std::shared_ptr<sf::Texture> texture; ///< Текстура тайлсета
        int firstGid;                         ///< Первый GID в тайлсете
        int tileWidth;                        ///< Ширина тайла
        int tileHeight;                       ///< Высота тайла
        int columns;                          ///< Количество столбцов в тайлсете
        int tileCount;                        ///< Общее количество тайлов

        /**
         * @brief Получить прямоугольник текстуры для тайла по GID
         * @param gid Global ID тайла
         * @return Прямоугольник текстуры
         */
        sf::IntRect getTileRect(int gid) const;
    };

    /**
     * @brief Структура слоя тайлов
     */
    struct TileLayer {
        std::vector<int> tiles;               ///< Массив ID тайлов (построчно)
        int width;                            ///< Ширина слоя в тайлах
        int height;                           ///< Высота слоя в тайлах
        float opacity;                        ///< Прозрачность слоя (0.0 - 1.0)
        bool visible;                         ///< Видимость слоя
    };

    /**
     * @brief Загрузить тайлсет из TMX
     * @param tileset Тайлсет из tmxlite
     * @return true если загрузка успешна
     */
    bool loadTileset(const tmx::Tileset& tileset);

    /**
     * @brief Обработать слой тайлов
     * @param layer Слой тайлов из tmxlite
     */
    void processTileLayer(const tmx::TileLayer& layer);

    /**
     * @brief Обработать слой объектов
     * @param layer Слой объектов из tmxlite
     */
    void processObjectLayer(const tmx::ObjectGroup& layer);

    /**
     * @brief Получить границы видимой области камеры
     * @param camera Вид камеры
     * @return Прямоугольник видимой области в мировых координатах
     */
    sf::FloatRect getViewBounds(const sf::View& camera) const;

    /**
     * @brief Отрендерить один тайл
     * @param target Цель рендеринга
     * @param tileX X координата тайла
     * @param tileY Y координата тайла
     * @param gid Global ID тайла
     */
    void renderTile(sf::RenderTarget& target, int tileX, int tileY, int gid);

    /**
     * @brief Найти тайлсет по GID тайла
     * @param gid Global ID тайла
     * @return Указатель на тайлсет или nullptr
     */
    const Tileset* getTilesetForGid(int gid) const;

    // Данные карты
    std::unique_ptr<tmx::Map> m_map;          ///< Загруженная TMX карта
    std::vector<Tileset> m_tilesets;          ///< Список тайлсетов
    std::vector<TileLayer> m_tileLayers;      ///< Список слоёв тайлов
    int m_mapWidth;                           ///< Ширина карты в тайлах
    int m_mapHeight;                          ///< Высота карты в тайлах
    int m_tileWidth;                          ///< Ширина тайла в пикселях
    int m_tileHeight;                         ///< Высота тайла в пикселях
    bool m_loaded;                            ///< Флаг загруженности карты

    // Кеш для оптимизации рендеринга
    mutable sf::VertexArray m_vertices;       ///< Массив вершин для батчинга
    mutable std::unordered_map<int, std::vector<sf::Vertex>> m_batchCache; ///< Кеш батчей по тайлсетам
};

} // namespace rendering
