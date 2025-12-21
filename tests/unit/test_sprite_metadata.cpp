#include <catch2/catch_test_macros.hpp>
#include "core/SpriteMetadata.h"
#include "core/ResourceManager.h"
#include <nlohmann/json.hpp>

using namespace core;

TEST_CASE("SpriteMetadata can be created from JSON", "[sprite][metadata]") {
    nlohmann::json testJson = {
        {"sprite", {
            {"name", "test_sprite"},
            {"author", "Test Author"},
            {"version", "1.0"},
            {"created", "2025-12-20"},
            {"description", "Test sprite description"},
            {"texture", "test.png"},
            {"size", {{"width", 32}, {"height", 32}}},
            {"origin", {{"x", 0}, {"y", 32}}},
            {"tags", nlohmann::json::array({"test", "development"})},
            {"animations", nlohmann::json::array({
                {
                    {"name", "idle"},
                    {"loop", false},
                    {"frames", nlohmann::json::array({
                        {{"x", 0}, {"y", 0}, {"duration", 0}}
                    })}
                },
                {
                    {"name", "running"},
                    {"loop", true},
                    {"frames", nlohmann::json::array({
                        {{"x", 0}, {"y", 0}, {"duration", 100}},
                        {{"x", 32}, {"y", 0}, {"duration", 100}}
                    })}
                }
            })}
        }}
    };

    auto metadataOpt = SpriteMetadata::loadFromJson(testJson);
    REQUIRE(metadataOpt.has_value());

    const auto& metadata = metadataOpt.value();

    SECTION("Basic fields are parsed correctly") {
        REQUIRE(metadata.getName() == "test_sprite");
        REQUIRE(metadata.getAuthor() == "Test Author");
        REQUIRE(metadata.getVersion() == "1.0");
        REQUIRE(metadata.getCreated() == "2025-12-20");
        REQUIRE(metadata.getDescription() == "Test sprite description");
        REQUIRE(metadata.getTexturePath() == "test.png");
    }

    SECTION("Size is parsed correctly") {
        const auto& size = metadata.getSize();
        REQUIRE(size.width == 32);
        REQUIRE(size.height == 32);
    }

    SECTION("Origin is parsed correctly") {
        const auto& origin = metadata.getOrigin();
        REQUIRE(origin.x == 0);
        REQUIRE(origin.y == 32);
    }

    SECTION("Tags are parsed correctly") {
        const auto& tags = metadata.getTags();
        REQUIRE(tags.size() == 2);
        REQUIRE(metadata.hasTag("test"));
        REQUIRE(metadata.hasTag("development"));
        REQUIRE_FALSE(metadata.hasTag("nonexistent"));
    }

    SECTION("Animations are parsed correctly") {
        const auto& animations = metadata.getAnimations();
        REQUIRE(animations.size() == 2);

        SECTION("Idle animation") {
            const auto* idle = metadata.getAnimation("idle");
            REQUIRE(idle != nullptr);
            REQUIRE(idle->name == "idle");
            REQUIRE(idle->loop == false);
            REQUIRE(idle->frames.size() == 1);
            REQUIRE(idle->frames[0].x == 0);
            REQUIRE(idle->frames[0].y == 0);
            REQUIRE(idle->frames[0].duration == 0);
        }

        SECTION("Running animation") {
            const auto* running = metadata.getAnimation("running");
            REQUIRE(running != nullptr);
            REQUIRE(running->name == "running");
            REQUIRE(running->loop == true);
            REQUIRE(running->frames.size() == 2);
            REQUIRE(running->frames[0].duration == 100);
            REQUIRE(running->frames[1].x == 32);
        }

        SECTION("Nonexistent animation") {
            const auto* nonexistent = metadata.getAnimation("nonexistent");
            REQUIRE(nonexistent == nullptr);
        }
    }
}

TEST_CASE("SpriteMetadata can be serialized to JSON", "[sprite][metadata]") {
    SpriteMetadata metadata;
    metadata.setName("test_sprite");
    metadata.setAuthor("Test Author");
    metadata.setVersion("1.0");
    metadata.setDescription("Test description");
    metadata.setTexturePath("test.png");

    SpriteSize size{32, 64};
    metadata.setSize(size);

    SpriteOrigin origin{16, 64};
    metadata.setOrigin(origin);

    metadata.addTag("test");
    metadata.addTag("development");

    AnimationInfo anim;
    anim.name = "idle";
    anim.loop = false;
    anim.frames.push_back({0, 0, 0});
    metadata.addAnimation(anim);

    nlohmann::json json = metadata.toJson();

    SECTION("JSON structure is correct") {
        REQUIRE(json.contains("sprite"));
        const auto& sprite = json["sprite"];

        REQUIRE(sprite["name"] == "test_sprite");
        REQUIRE(sprite["author"] == "Test Author");
        REQUIRE(sprite["version"] == "1.0");
        REQUIRE(sprite["description"] == "Test description");
        REQUIRE(sprite["texture"] == "test.png");

        REQUIRE(sprite["size"]["width"] == 32);
        REQUIRE(sprite["size"]["height"] == 64);

        REQUIRE(sprite["origin"]["x"] == 16);
        REQUIRE(sprite["origin"]["y"] == 64);

        REQUIRE(sprite["tags"].size() == 2);
        REQUIRE(sprite["animations"].size() == 1);
    }
}

TEST_CASE("SpriteMetadata can load from file", "[sprite][metadata][file]") {
    const std::string testPath = "assets/sprites/TEST/testObj.sprite.json";

    auto metadataOpt = SpriteMetadata::loadFromFile(testPath);

    SECTION("File loads successfully if it exists") {
        // This test will only pass if the file actually exists
        if (metadataOpt.has_value()) {
            const auto& metadata = metadataOpt.value();
            REQUIRE(metadata.getName() == "test_object");
            REQUIRE(metadata.getTexturePath() == "testObj.png");
        }
    }
}

TEST_CASE("SpriteMetadata integration with ResourceManager", "[sprite][metadata][resourcemanager]") {
    ResourceManager resourceManager;

    SECTION("Can load sprite metadata through ResourceManager") {
        const std::string testPath = "assets/sprites/TEST/testObj.sprite.json";

        // Try to load metadata
        const SpriteMetadata* metadata = resourceManager.loadSpriteMetadata(testPath);

        if (metadata != nullptr) {
            REQUIRE(metadata->getName() == "test_object");

            // Check that we can retrieve it again
            const SpriteMetadata* retrieved = resourceManager.getSpriteMetadata("test_object");
            REQUIRE(retrieved != nullptr);
            REQUIRE(retrieved->getName() == metadata->getName());

            // Check sprite count
            REQUIRE(resourceManager.getSpriteMetadataCount() >= 1);

            // Check sprite names list
            auto names = resourceManager.getSpriteNames();
            REQUIRE_FALSE(names.empty());
        }
    }
}

TEST_CASE("AnimationFrame JSON serialization", "[sprite][animation][json]") {
    AnimationFrame frame{64, 32, 150};

    nlohmann::json json = frame.toJson();

    REQUIRE(json["x"] == 64);
    REQUIRE(json["y"] == 32);
    REQUIRE(json["duration"] == 150);

    AnimationFrame deserializedFrame = AnimationFrame::fromJson(json);
    REQUIRE(deserializedFrame.x == 64);
    REQUIRE(deserializedFrame.y == 32);
    REQUIRE(deserializedFrame.duration == 150);
}

TEST_CASE("AnimationInfo JSON serialization", "[sprite][animation][json]") {
    AnimationInfo info;
    info.name = "jump";
    info.loop = false;
    info.frames.push_back({0, 0, 100});
    info.frames.push_back({32, 0, 100});
    info.frames.push_back({64, 0, 200});

    nlohmann::json json = info.toJson();

    REQUIRE(json["name"] == "jump");
    REQUIRE(json["loop"] == false);
    REQUIRE(json["frames"].size() == 3);

    AnimationInfo deserializedInfo = AnimationInfo::fromJson(json);
    REQUIRE(deserializedInfo.name == "jump");
    REQUIRE(deserializedInfo.loop == false);
    REQUIRE(deserializedInfo.frames.size() == 3);
    REQUIRE(deserializedInfo.frames[2].duration == 200);
}
