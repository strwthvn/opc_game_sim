#include "rendering/TileMapSystem.h"
#include <spdlog/spdlog.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Tileset.hpp>
#include <cmath>
#include <algorithm>

namespace rendering {

TileMapSystem::TileMapSystem()
    : m_map(nullptr)
    , m_mapWidth(0)
    , m_mapHeight(0)
    , m_tileWidth(0)
    , m_tileHeight(0)
    , m_loaded(false)
    , m_vertices(sf::PrimitiveType::Triangles) {
}

TileMapSystem::~TileMapSystem() {
    unloadMap();
}

bool TileMapSystem::loadMap(const std::string& tmxPath) {
    spdlog::info("Loading TMX map: {}", tmxPath);

    // Создаём новую карту
    auto map = std::make_unique<tmx::Map>();
    if (!map->load(tmxPath)) {
        spdlog::error("Failed to load TMX map: {}", tmxPath);
        return false;
    }

    // Выгружаем предыдущую карту если была
    unloadMap();

    // Сохраняем карту
    m_map = std::move(map);

    // Получаем размеры карты
    const auto& mapSize = m_map->getTileCount();
    m_mapWidth = static_cast<int>(mapSize.x);
    m_mapHeight = static_cast<int>(mapSize.y);

    const auto& tileSize = m_map->getTileSize();
    m_tileWidth = static_cast<int>(tileSize.x);
    m_tileHeight = static_cast<int>(tileSize.y);

    spdlog::info("Map size: {}x{} tiles ({}x{} pixels per tile)",
                 m_mapWidth, m_mapHeight, m_tileWidth, m_tileHeight);

    // Загружаем тайлсеты
    const auto& tilesets = m_map->getTilesets();
    for (const auto& tileset : tilesets) {
        if (!loadTileset(tileset)) {
            spdlog::warn("Failed to load tileset: {}", tileset.getName());
        }
    }

    // Обрабатываем слои
    const auto& layers = m_map->getLayers();
    for (const auto& layer : layers) {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            processTileLayer(static_cast<const tmx::TileLayer&>(*layer));
        } else if (layer->getType() == tmx::Layer::Type::Object) {
            processObjectLayer(static_cast<const tmx::ObjectGroup&>(*layer));
        }
    }

    m_loaded = true;
    spdlog::info("TMX map loaded successfully: {} tilesets, {} tile layers",
                 m_tilesets.size(), m_tileLayers.size());

    return true;
}

void TileMapSystem::unloadMap() {
    if (!m_loaded) {
        return;
    }

    m_map.reset();
    m_tilesets.clear();
    m_tileLayers.clear();
    m_batchCache.clear();
    m_mapWidth = 0;
    m_mapHeight = 0;
    m_tileWidth = 0;
    m_tileHeight = 0;
    m_loaded = false;

    spdlog::debug("TMX map unloaded");
}

bool TileMapSystem::loadTileset(const tmx::Tileset& tileset) {
    Tileset ts;
    ts.firstGid = static_cast<int>(tileset.getFirstGID());
    ts.tileWidth = static_cast<int>(tileset.getTileSize().x);
    ts.tileHeight = static_cast<int>(tileset.getTileSize().y);
    ts.columns = static_cast<int>(tileset.getColumnCount());
    ts.tileCount = static_cast<int>(tileset.getTileCount());

    // Загружаем текстуру тайлсета
    const auto& imagePath = tileset.getImagePath();
    if (imagePath.empty()) {
        spdlog::error("Tileset '{}' has no image path", tileset.getName());
        return false;
    }

    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(imagePath)) {
        spdlog::error("Failed to load tileset texture: {}", imagePath);
        return false;
    }

    ts.texture = texture;
    m_tilesets.push_back(std::move(ts));

    spdlog::debug("Loaded tileset: {} (FirstGID: {}, {}x{} tiles)",
                  tileset.getName(), ts.firstGid, ts.columns, ts.tileCount / ts.columns);

    return true;
}

void TileMapSystem::processTileLayer(const tmx::TileLayer& layer) {
    TileLayer tileLayer;
    tileLayer.width = static_cast<int>(layer.getSize().x);
    tileLayer.height = static_cast<int>(layer.getSize().y);
    tileLayer.opacity = layer.getOpacity();
    tileLayer.visible = layer.getVisible();

    // Копируем тайлы
    const auto& tiles = layer.getTiles();
    tileLayer.tiles.reserve(tiles.size());

    for (const auto& tile : tiles) {
        tileLayer.tiles.push_back(static_cast<int>(tile.ID));
    }

    m_tileLayers.push_back(std::move(tileLayer));

    spdlog::debug("Processed tile layer: {}x{} tiles, visible: {}, opacity: {}",
                  tileLayer.width, tileLayer.height, layer.getVisible(), layer.getOpacity());
}

void TileMapSystem::processObjectLayer(const tmx::ObjectGroup& layer) {
    // TODO: Обработка объектов (будет реализовано позже)
    // Объекты могут использоваться для размещения промышленных объектов,
    // зон столкновений и т.д.
    spdlog::debug("Object layer '{}' skipped (not implemented yet)", layer.getName());
}

void TileMapSystem::render(sf::RenderTarget& target, const sf::View& camera) {
    if (!m_loaded) {
        return;
    }

    // Получаем видимую область
    sf::FloatRect viewBounds = getViewBounds(camera);

    // Вычисляем диапазон видимых тайлов
    int startX = std::max(0, static_cast<int>(std::floor(viewBounds.position.x / m_tileWidth)));
    int startY = std::max(0, static_cast<int>(std::floor(viewBounds.position.y / m_tileHeight)));
    int endX = std::min(m_mapWidth, static_cast<int>(std::ceil((viewBounds.position.x + viewBounds.size.x) / m_tileWidth)) + 1);
    int endY = std::min(m_mapHeight, static_cast<int>(std::ceil((viewBounds.position.y + viewBounds.size.y) / m_tileHeight)) + 1);

    // Отрисовываем каждый слой
    for (const auto& layer : m_tileLayers) {
        if (!layer.visible) {
            continue;
        }

        // Группируем тайлы по тайлсетам для батчинга
        // Ключ - индекс тайлсета, значение - список тайлов (x, y, gid)
        std::unordered_map<size_t, std::vector<std::tuple<int, int, int>>> tilesByTileset;

        // Собираем видимые тайлы слоя
        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                int tileIndex = y * layer.width + x;
                if (tileIndex >= 0 && tileIndex < static_cast<int>(layer.tiles.size())) {
                    int gid = layer.tiles[tileIndex];
                    if (gid > 0) {  // 0 = пустой тайл
                        // Находим тайлсет для этого тайла
                        size_t tilesetIdx = 0;
                        for (size_t i = 0; i < m_tilesets.size(); ++i) {
                            if (m_tilesets[i].firstGid <= gid) {
                                tilesetIdx = i;
                            } else {
                                break;
                            }
                        }
                        tilesByTileset[tilesetIdx].emplace_back(x, y, gid);
                    }
                }
            }
        }

        // Рендерим батчи по тайлсетам
        for (const auto& [tilesetIdx, tiles] : tilesByTileset) {
            if (tilesetIdx >= m_tilesets.size()) {
                continue;
            }

            const Tileset& tileset = m_tilesets[tilesetIdx];
            if (!tileset.texture) {
                continue;
            }

            // Очищаем VertexArray и резервируем память
            m_vertices.clear();
            m_vertices.resize(tiles.size() * 6);  // 6 вершин на тайл (2 треугольника)

            // Добавляем все тайлы этого тайлсета в VertexArray
            for (size_t i = 0; i < tiles.size(); ++i) {
                const auto& [tileX, tileY, gid] = tiles[i];

                // Получаем прямоугольник текстуры для этого тайла
                sf::IntRect texRect = tileset.getTileRect(gid);

                // Позиция тайла в мире
                float posX = static_cast<float>(tileX * m_tileWidth);
                float posY = static_cast<float>(tileY * m_tileHeight);
                float sizeX = static_cast<float>(m_tileWidth);
                float sizeY = static_cast<float>(m_tileHeight);

                // Координаты текстуры
                float texLeft = static_cast<float>(texRect.position.x);
                float texTop = static_cast<float>(texRect.position.y);
                float texRight = texLeft + static_cast<float>(texRect.size.x);
                float texBottom = texTop + static_cast<float>(texRect.size.y);

                // Добавляем 6 вершин (2 треугольника образуют quad)
                size_t vertexIdx = i * 6;

                // Первый треугольник
                m_vertices[vertexIdx + 0].position = sf::Vector2f(posX, posY);
                m_vertices[vertexIdx + 0].texCoords = sf::Vector2f(texLeft, texTop);

                m_vertices[vertexIdx + 1].position = sf::Vector2f(posX + sizeX, posY);
                m_vertices[vertexIdx + 1].texCoords = sf::Vector2f(texRight, texTop);

                m_vertices[vertexIdx + 2].position = sf::Vector2f(posX, posY + sizeY);
                m_vertices[vertexIdx + 2].texCoords = sf::Vector2f(texLeft, texBottom);

                // Второй треугольник
                m_vertices[vertexIdx + 3].position = sf::Vector2f(posX + sizeX, posY);
                m_vertices[vertexIdx + 3].texCoords = sf::Vector2f(texRight, texTop);

                m_vertices[vertexIdx + 4].position = sf::Vector2f(posX + sizeX, posY + sizeY);
                m_vertices[vertexIdx + 4].texCoords = sf::Vector2f(texRight, texBottom);

                m_vertices[vertexIdx + 5].position = sf::Vector2f(posX, posY + sizeY);
                m_vertices[vertexIdx + 5].texCoords = sf::Vector2f(texLeft, texBottom);
            }

            // Одна отрисовка для всех тайлов этого тайлсета
            sf::RenderStates states;
            states.texture = tileset.texture.get();
            target.draw(m_vertices, states);
        }
    }
}

sf::FloatRect TileMapSystem::getViewBounds(const sf::View& camera) const {
    sf::Vector2f center = camera.getCenter();
    sf::Vector2f size = camera.getSize();

    sf::Vector2f position(center.x - size.x / 2.0f, center.y - size.y / 2.0f);
    return sf::FloatRect(position, size);
}

void TileMapSystem::renderTile(sf::RenderTarget& target, int tileX, int tileY, int gid) {
    // DEPRECATED: Этот метод больше не используется.
    // Рендеринг теперь выполняется через батчинг в методе render()
    // для оптимизации производительности (избегаем создания sf::Sprite каждый кадр).
    // Оставлен для возможной будущей совместимости.
    (void)target;
    (void)tileX;
    (void)tileY;
    (void)gid;
}

const TileMapSystem::Tileset* TileMapSystem::getTilesetForGid(int gid) const {
    // Тайлсеты должны быть отсортированы по firstGid
    // Ищем последний тайлсет, у которого firstGid <= gid
    const Tileset* result = nullptr;

    for (const auto& tileset : m_tilesets) {
        if (tileset.firstGid <= gid) {
            result = &tileset;
        } else {
            break;
        }
    }

    return result;
}

sf::IntRect TileMapSystem::Tileset::getTileRect(int gid) const {
    // Локальный ID тайла в этом тайлсете
    int localId = gid - firstGid;

    // Вычисляем позицию в тайлсете
    int column = localId % columns;
    int row = localId / columns;

    sf::Vector2i position(column * tileWidth, row * tileHeight);
    sf::Vector2i size(tileWidth, tileHeight);
    return sf::IntRect(position, size);
}

} // namespace rendering
