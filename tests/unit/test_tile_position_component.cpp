/**
 * @file test_tile_position_component.cpp
 * @brief Unit tests for TilePositionComponent
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <core/Components.h>

using namespace core;

TEST_CASE("TilePositionComponent: Default constructor", "[TilePositionComponent]") {
    TilePositionComponent tilePos;

    REQUIRE(tilePos.tileX == 0);
    REQUIRE(tilePos.tileY == 0);
    REQUIRE(tilePos.widthTiles == 1);
    REQUIRE(tilePos.heightTiles == 1);
    REQUIRE(tilePos.autoSync == true);
}

TEST_CASE("TilePositionComponent: getPixelPosition()", "[TilePositionComponent]") {
    SECTION("Single tile object at origin") {
        TilePositionComponent tilePos;
        tilePos.tileX = 0;
        tilePos.tileY = 0;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        sf::Vector2f pixelPos = tilePos.getPixelPosition();

        // Bottom-left corner: (0*32, (0+1)*32) = (0, 32)
        REQUIRE_THAT(pixelPos.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(pixelPos.y, Catch::Matchers::WithinAbs(32.0f, 0.001f));
    }

    SECTION("Single tile object at (3, 6)") {
        TilePositionComponent tilePos;
        tilePos.tileX = 3;
        tilePos.tileY = 6;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        sf::Vector2f pixelPos = tilePos.getPixelPosition();

        // Bottom-left corner: (3*32, (6+1)*32) = (96, 224)
        REQUIRE_THAT(pixelPos.x, Catch::Matchers::WithinAbs(96.0f, 0.001f));
        REQUIRE_THAT(pixelPos.y, Catch::Matchers::WithinAbs(224.0f, 0.001f));
    }

    SECTION("Multi-tile object (2x3) at (5, 10)") {
        TilePositionComponent tilePos;
        tilePos.tileX = 5;
        tilePos.tileY = 10;
        tilePos.widthTiles = 2;
        tilePos.heightTiles = 3;

        sf::Vector2f pixelPos = tilePos.getPixelPosition();

        // Bottom-left corner: (5*32, (10+3)*32) = (160, 416)
        REQUIRE_THAT(pixelPos.x, Catch::Matchers::WithinAbs(160.0f, 0.001f));
        REQUIRE_THAT(pixelPos.y, Catch::Matchers::WithinAbs(416.0f, 0.001f));
    }

    SECTION("Negative tile coordinates") {
        TilePositionComponent tilePos;
        tilePos.tileX = -2;
        tilePos.tileY = -3;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        sf::Vector2f pixelPos = tilePos.getPixelPosition();

        // Bottom-left corner: (-2*32, (-3+1)*32) = (-64, -64)
        REQUIRE_THAT(pixelPos.x, Catch::Matchers::WithinAbs(-64.0f, 0.001f));
        REQUIRE_THAT(pixelPos.y, Catch::Matchers::WithinAbs(-64.0f, 0.001f));
    }
}

TEST_CASE("TilePositionComponent: getCenterPixel()", "[TilePositionComponent]") {
    SECTION("Single tile object at (0, 0)") {
        TilePositionComponent tilePos;
        tilePos.tileX = 0;
        tilePos.tileY = 0;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        sf::Vector2f center = tilePos.getCenterPixel();

        // Center: ((0 + 1*0.5)*32, (0 + 1*0.5)*32) = (16, 16)
        REQUIRE_THAT(center.x, Catch::Matchers::WithinAbs(16.0f, 0.001f));
        REQUIRE_THAT(center.y, Catch::Matchers::WithinAbs(16.0f, 0.001f));
    }

    SECTION("Multi-tile object (4x2) at (2, 3)") {
        TilePositionComponent tilePos;
        tilePos.tileX = 2;
        tilePos.tileY = 3;
        tilePos.widthTiles = 4;
        tilePos.heightTiles = 2;

        sf::Vector2f center = tilePos.getCenterPixel();

        // Center: ((2 + 4*0.5)*32, (3 + 2*0.5)*32) = (4*32, 4*32) = (128, 128)
        REQUIRE_THAT(center.x, Catch::Matchers::WithinAbs(128.0f, 0.001f));
        REQUIRE_THAT(center.y, Catch::Matchers::WithinAbs(128.0f, 0.001f));
    }
}

TEST_CASE("TilePositionComponent: setFromPixelPosition()", "[TilePositionComponent]") {
    SECTION("Convert pixel (96, 224) to tile coordinates") {
        TilePositionComponent tilePos;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        tilePos.setFromPixelPosition(sf::Vector2f(96.0f, 224.0f));

        // tileX = 96 / 32 = 3
        // tileY = (224 / 32) - 1 = 7 - 1 = 6
        REQUIRE(tilePos.tileX == 3);
        REQUIRE(tilePos.tileY == 6);
    }

    SECTION("Convert pixel (160, 416) with multi-tile height") {
        TilePositionComponent tilePos;
        tilePos.widthTiles = 2;
        tilePos.heightTiles = 3;

        tilePos.setFromPixelPosition(sf::Vector2f(160.0f, 416.0f));

        // tileX = 160 / 32 = 5
        // tileY = (416 / 32) - 3 = 13 - 3 = 10
        REQUIRE(tilePos.tileX == 5);
        REQUIRE(tilePos.tileY == 10);
    }

    SECTION("Round-trip conversion") {
        TilePositionComponent original;
        original.tileX = 7;
        original.tileY = 12;
        original.widthTiles = 2;
        original.heightTiles = 4;

        sf::Vector2f pixelPos = original.getPixelPosition();

        TilePositionComponent converted;
        converted.widthTiles = 2;
        converted.heightTiles = 4;
        converted.setFromPixelPosition(pixelPos);

        REQUIRE(converted.tileX == original.tileX);
        REQUIRE(converted.tileY == original.tileY);
    }
}

TEST_CASE("TilePositionComponent: containsTile()", "[TilePositionComponent]") {
    SECTION("Single tile object") {
        TilePositionComponent tilePos;
        tilePos.tileX = 3;
        tilePos.tileY = 6;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;

        REQUIRE(tilePos.containsTile(3, 6) == true);
        REQUIRE(tilePos.containsTile(2, 6) == false);
        REQUIRE(tilePos.containsTile(4, 6) == false);
        REQUIRE(tilePos.containsTile(3, 5) == false);
        REQUIRE(tilePos.containsTile(3, 7) == false);
    }

    SECTION("Multi-tile object (3x2)") {
        TilePositionComponent tilePos;
        tilePos.tileX = 5;
        tilePos.tileY = 10;
        tilePos.widthTiles = 3;
        tilePos.heightTiles = 2;

        // Should contain tiles from (5,10) to (7,11)
        REQUIRE(tilePos.containsTile(5, 10) == true);
        REQUIRE(tilePos.containsTile(6, 10) == true);
        REQUIRE(tilePos.containsTile(7, 10) == true);
        REQUIRE(tilePos.containsTile(5, 11) == true);
        REQUIRE(tilePos.containsTile(6, 11) == true);
        REQUIRE(tilePos.containsTile(7, 11) == true);

        // Edge cases - should not contain
        REQUIRE(tilePos.containsTile(4, 10) == false);
        REQUIRE(tilePos.containsTile(8, 10) == false);
        REQUIRE(tilePos.containsTile(5, 9) == false);
        REQUIRE(tilePos.containsTile(5, 12) == false);
    }

    SECTION("Edge alignment") {
        TilePositionComponent tilePos;
        tilePos.tileX = 0;
        tilePos.tileY = 0;
        tilePos.widthTiles = 2;
        tilePos.heightTiles = 2;

        // Contains (0,0), (1,0), (0,1), (1,1)
        REQUIRE(tilePos.containsTile(0, 0) == true);
        REQUIRE(tilePos.containsTile(1, 0) == true);
        REQUIRE(tilePos.containsTile(0, 1) == true);
        REQUIRE(tilePos.containsTile(1, 1) == true);

        // Boundary: (2,0) is outside (x >= tileX + widthTiles)
        REQUIRE(tilePos.containsTile(2, 0) == false);
        REQUIRE(tilePos.containsTile(0, 2) == false);
    }
}

TEST_CASE("TilePositionComponent: Tile size constant", "[TilePositionComponent]") {
    // Verify TILE_SIZE constant is 32
    REQUIRE(TILE_SIZE == 32);
}
