#include "SaveSystem.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace GameSave;

int testsPassed = 0;
int testsFailed = 0;

#define TEST(name) \
    do { \
        std::cout << "Running " << name << "... "; \
        try { \
            name(); \
            std::cout << "PASSED\n"; \
            testsPassed++; \
        } catch (const std::exception& e) { \
            std::cout << "FAILED: " << e.what() << "\n"; \
            testsFailed++; \
        } catch (...) { \
            std::cout << "FAILED: Unknown exception\n"; \
            testsFailed++; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if (!((a) == (b))) { \
            throw std::runtime_error("Assertion failed: " #a " == " #b); \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            throw std::runtime_error("Assertion failed: " #a " != " #b); \
        } \
    } while(0)

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error("Assertion failed: " #cond " is false"); \
        } \
    } while(0)

#define ASSERT_FALSE(cond) \
    do { \
        if (cond) { \
            throw std::runtime_error("Assertion failed: " #cond " is true"); \
        } \
    } while(0)

#define ASSERT_ERROR(actual, expected) \
    do { \
        if (static_cast<int>(actual) != static_cast<int>(expected)) { \
            throw std::runtime_error("Assertion failed: expected error " #expected " but got different error"); \
        } \
    } while(0)

void writeTextFile(const std::string& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
}

void testCreateSave() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.playerName = "TestPlayer";
    state.level = 10;
    state.experience = 5000;
    state.experienceToNextLevel = 10000;
    state.health = 85;
    state.maxHealth = 100;
    state.mana = 50;
    state.maxMana = 100;
    state.coins = 1500;
    state.positionX = 10.5f;
    state.positionY = 20.3f;
    state.currentMap = "TestMap";
    state.playtime = std::chrono::seconds(3600);

    auto error = saveSystem.save(1, state);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_TRUE(saveSystem.exists(1));
}

void testLoadSave() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    auto error = saveSystem.load(1, state);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_EQ(state.playerName, "TestPlayer");
    ASSERT_EQ(state.level, 10);
    ASSERT_EQ(state.experience, 5000);
    ASSERT_EQ(state.experienceToNextLevel, 10000);
    ASSERT_EQ(state.health, 85);
    ASSERT_EQ(state.maxHealth, 100);
    ASSERT_EQ(state.mana, 50);
    ASSERT_EQ(state.maxMana, 100);
    ASSERT_EQ(state.coins, 1500);
    ASSERT_EQ(state.positionX, 10.5f);
    ASSERT_EQ(state.positionY, 20.3f);
    ASSERT_EQ(state.currentMap, "TestMap");
    ASSERT_EQ(state.playtime.count(), 3600);
}

void testLoadOptional() {
    SaveSystem saveSystem("test_saves");
    auto state = saveSystem.load(1);
    ASSERT_TRUE(state.has_value());
    ASSERT_EQ(state->playerName, "TestPlayer");
}

void testLoadNonExistent() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    auto error = saveSystem.load(99, state);
    ASSERT_ERROR(error, SaveError::SlotNotFound);

    auto opt = saveSystem.load(99);
    ASSERT_FALSE(opt.has_value());
}

void testDeleteSave() {
    SaveSystem saveSystem("test_saves");
    ASSERT_TRUE(saveSystem.exists(1));
    auto error = saveSystem.remove(1);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_FALSE(saveSystem.exists(1));
}

void testDeleteNonExistent() {
    SaveSystem saveSystem("test_saves");
    auto error = saveSystem.remove(99);
    ASSERT_ERROR(error, SaveError::SlotNotFound);
}

void testMultipleSlots() {
    SaveSystem saveSystem("test_saves");

    GameState state1;
    state1.playerName = "Player1";
    state1.level = 5;
    saveSystem.save(1, state1);

    GameState state2;
    state2.playerName = "Player2";
    state2.level = 15;
    saveSystem.save(2, state2);

    ASSERT_TRUE(saveSystem.exists(1));
    ASSERT_TRUE(saveSystem.exists(2));

    auto saves = saveSystem.listSaves();
    ASSERT_EQ(saves.size(), 2);
    ASSERT_EQ(saves[0].slot, 1);
    ASSERT_EQ(saves[0].playerName, "Player1");
    ASSERT_EQ(saves[1].slot, 2);
    ASSERT_EQ(saves[1].playerName, "Player2");
}

void testInvalidSlot() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.playerName = "Test";

    auto error = saveSystem.save(0, state);
    ASSERT_ERROR(error, SaveError::InvalidSlot);

    error = saveSystem.save(-1, state);
    ASSERT_ERROR(error, SaveError::InvalidSlot);
}

void testCorruptedFile() {
    SaveSystem saveSystem("test_saves");

    std::ofstream file("test_saves/slot_5.sav");
    file << "corrupted data";
    file.close();

    GameState state;
    auto error = saveSystem.load(5, state);
    ASSERT_ERROR(error, SaveError::InvalidFormat);
}

void testStringEscaping() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.playerName = "Player\nWith\tSpecial\\Chars=;";
    state.level = 1;

    auto error = saveSystem.save(10, state);
    ASSERT_ERROR(error, SaveError::None);

    GameState loaded;
    error = saveSystem.load(10, loaded);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_EQ(loaded.playerName, "Player\nWith\tSpecial\\Chars=;");
}

void testCollectiblesRoundTrip() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.level = 3;
    state.experience = 420;
    state.experienceToNextLevel = 3000;
    state.health = 95;
    state.maxHealth = 120;
    state.mana = 70;
    state.maxMana = 120;
    state.coins = 240;
    state.positionX = 712.5f;
    state.positionY = 388.25f;
    state.collectibleDataAvailable = true;
    state.collectibles = {
        { 120.0f, 240.0f, 30, 0, false, 2.4f, 1.5f },
        { 500.0f, 300.0f, 100, 1, true, 0.8f, 1.8f },
        { 700.0f, 500.0f, 25, 2, true, 1.2f, 1.2f },
        { 900.0f, 500.0f, 25, 3, false, 2.0f, 1.2f }
    };

    auto error = saveSystem.save(1, state);
    ASSERT_ERROR(error, SaveError::None);

    GameState loaded;
    error = saveSystem.load(1, loaded);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_EQ(loaded.collectibleDataAvailable, true);
    ASSERT_EQ(loaded.collectibles.size(), state.collectibles.size());
    for (size_t i = 0; i < state.collectibles.size(); ++i) {
        ASSERT_EQ(loaded.collectibles[i].positionX, state.collectibles[i].positionX);
        ASSERT_EQ(loaded.collectibles[i].positionY, state.collectibles[i].positionY);
        ASSERT_EQ(loaded.collectibles[i].value, state.collectibles[i].value);
        ASSERT_EQ(loaded.collectibles[i].type, state.collectibles[i].type);
        ASSERT_EQ(loaded.collectibles[i].active, state.collectibles[i].active);
    }
}

void testInvalidStateIsRejected() {
    SaveSystem saveSystem("test_saves");
    std::ofstream file("test_saves/slot_6.sav");
    file << "version=2\nlevel=0\nexperience=0\nexperienceToNextLevel=1000\n";
    file << "health=100\nmaxHealth=100\nmana=100\nmaxMana=100\ncoins=0\n";
    file << "positionX=100\npositionY=100\ncollectibleCount=0\n";
    file.close();

    GameState state;
    auto error = saveSystem.load(6, state);
    ASSERT_ERROR(error, SaveError::CorruptedData);
    ASSERT_EQ(state.level, 1);
}

void testVersion1Compatibility() {
    SaveSystem saveSystem("test_saves");
    writeTextFile(
        "test_saves/slot_7.sav",
        "version=1\n"
        "playerName=LegacyPlayer\n"
        "level=2\n"
        "experience=250\n"
        "experienceToNextLevel=2000\n"
        "health=80\n"
        "maxHealth=100\n"
        "mana=60\n"
        "maxMana=100\n"
        "coins=42\n"
        "positionX=125.5\n"
        "positionY=250.25\n"
        "playtime=120\n"
        "saveTime=2026-09-10 12:00:00\n"
    );

    GameState loaded;
    auto error = saveSystem.load(7, loaded);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_EQ(loaded.playerName, "LegacyPlayer");
    ASSERT_EQ(loaded.level, 2);
    ASSERT_EQ(loaded.experience, 250);
    ASSERT_EQ(loaded.experienceToNextLevel, 2000);
    ASSERT_EQ(loaded.maxHealth, 100);
    ASSERT_EQ(loaded.maxMana, 100);
    ASSERT_EQ(loaded.currentMap, "TrainingGround");
    ASSERT_EQ(loaded.collectibleDataAvailable, false);
    ASSERT_TRUE(loaded.collectibles.empty());
}

void testInvalidVersionIsRejected() {
    SaveSystem saveSystem("test_saves");
    writeTextFile(
        "test_saves/slot_8.sav",
        "version=3\n"
        "level=1\n"
        "experience=0\n"
        "experienceToNextLevel=1000\n"
        "health=100\n"
        "maxHealth=100\n"
        "mana=100\n"
        "maxMana=100\n"
        "coins=0\n"
        "positionX=100\n"
        "positionY=100\n"
        "collectibleCount=0\n"
    );

    GameState loaded;
    auto error = saveSystem.load(8, loaded);
    ASSERT_ERROR(error, SaveError::InvalidFormat);
}

void testCollectibleActiveValues() {
    SaveSystem saveSystem("test_saves");
    const std::string prefix =
        "version=2\n"
        "level=1\n"
        "experience=0\n"
        "experienceToNextLevel=1000\n"
        "health=100\n"
        "maxHealth=100\n"
        "mana=100\n"
        "maxMana=100\n"
        "coins=0\n"
        "positionX=100\n"
        "positionY=100\n"
        "collectibleCount=1\n";

    writeTextFile("test_saves/slot_9.sav", prefix +
        "collectible0Type=0\n"
        "collectible0Value=10\n"
        "collectible0Active=0\n"
        "collectible0PositionX=120\n"
        "collectible0PositionY=130\n"
        "collectible0BobOffset=0\n"
        "collectible0BobSpeed=1\n");
    GameState loadedFalse;
    ASSERT_ERROR(saveSystem.load(9, loadedFalse), SaveError::None);
    ASSERT_FALSE(loadedFalse.collectibles[0].active);

    writeTextFile("test_saves/slot_9.sav", prefix +
        "collectible0Type=0\n"
        "collectible0Value=10\n"
        "collectible0Active=1\n"
        "collectible0PositionX=120\n"
        "collectible0PositionY=130\n"
        "collectible0BobOffset=0\n"
        "collectible0BobSpeed=1\n");
    GameState loadedTrue;
    ASSERT_ERROR(saveSystem.load(9, loadedTrue), SaveError::None);
    ASSERT_TRUE(loadedTrue.collectibles[0].active);

    writeTextFile("test_saves/slot_9.sav", prefix +
        "collectible0Type=0\n"
        "collectible0Value=10\n"
        "collectible0Active=yes\n"
        "collectible0PositionX=120\n"
        "collectible0PositionY=130\n"
        "collectible0BobOffset=0\n"
        "collectible0BobSpeed=1\n");
    GameState state;
    auto error = saveSystem.load(9, state);
    ASSERT_ERROR(error, SaveError::CorruptedData);
}

void testCollectibleIndicesAndCount() {
    SaveSystem saveSystem("test_saves");
    const std::string base =
        "version=2\n"
        "level=1\n"
        "experience=0\n"
        "experienceToNextLevel=1000\n"
        "health=100\n"
        "maxHealth=100\n"
        "mana=100\n"
        "maxMana=100\n"
        "coins=0\n"
        "positionX=100\n"
        "positionY=100\n";

    const std::vector<std::string> invalidFiles = {
        base + "collectibleCount=1\ncollectible1Type=0\n",
        base + "collectibleCount=1\ncollectible-1Type=0\n",
        base + "collectibleCount=1\ncollectibleType=0\n",
        base + "collectibleCount=1\ncollectible0Unknown=0\n",
        base + "collectibleCount=1001\n",
        base + "collectibleCount=-1\n",
        base + "collectibleCount=1\ncollectible0Type=nope\n"
    };

    for (size_t i = 0; i < invalidFiles.size(); ++i) {
        writeTextFile("test_saves/slot_10.sav", invalidFiles[i]);
        GameState loaded;
        ASSERT_ERROR(saveSystem.load(10, loaded), SaveError::CorruptedData);
    }
}

void testZeroCollectibles() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.collectibleDataAvailable = true;
    auto error = saveSystem.save(11, state);
    ASSERT_ERROR(error, SaveError::None);

    GameState loaded;
    error = saveSystem.load(11, loaded);
    ASSERT_ERROR(error, SaveError::None);
    ASSERT_TRUE(loaded.collectibleDataAvailable);
    ASSERT_TRUE(loaded.collectibles.empty());
}

void testEmptyDirectory() {
    // Create a fresh save system with empty directory
    std::filesystem::remove_all("test_saves_empty");
    SaveSystem saveSystem("test_saves_empty");

    auto saves = saveSystem.listSaves();
    ASSERT_EQ(saves.size(), 0);
}

int main() {
    std::cout << "=== GameSaveSystem Tests ===\n\n";

    // Clean up any previous test data
    std::filesystem::remove_all("test_saves");
    std::filesystem::remove_all("test_saves_empty");

    TEST(testCreateSave);
    TEST(testLoadSave);
    TEST(testLoadOptional);
    TEST(testLoadNonExistent);
    TEST(testDeleteSave);
    TEST(testDeleteNonExistent);
    TEST(testMultipleSlots);
    TEST(testInvalidSlot);
    TEST(testCorruptedFile);
    TEST(testStringEscaping);
    TEST(testCollectiblesRoundTrip);
    TEST(testInvalidStateIsRejected);
    TEST(testVersion1Compatibility);
    TEST(testInvalidVersionIsRejected);
    TEST(testCollectibleActiveValues);
    TEST(testCollectibleIndicesAndCount);
    TEST(testZeroCollectibles);
    TEST(testEmptyDirectory);

    std::cout << "\n=== Results ===\n";
    std::cout << "Passed: " << testsPassed << "\n";
    std::cout << "Failed: " << testsFailed << "\n";

    // Cleanup
    std::filesystem::remove_all("test_saves");
    std::filesystem::remove_all("test_saves_empty");

    return testsFailed == 0 ? 0 : 1;
}