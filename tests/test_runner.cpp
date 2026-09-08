#include "SaveSystem.hpp"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

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

void testCreateSave() {
    SaveSystem saveSystem("test_saves");
    GameState state;
    state.playerName = "TestPlayer";
    state.level = 10;
    state.health = 85;
    state.mana = 50;
    state.experience = 5000;
    state.coins = 1500;
    state.positionX = 10.5f;
    state.positionY = 20.3f;
    state.positionZ = 0.0f;
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
    ASSERT_EQ(state.health, 85);
    ASSERT_EQ(state.mana, 50);
    ASSERT_EQ(state.experience, 5000);
    ASSERT_EQ(state.coins, 1500);
    ASSERT_EQ(state.positionX, 10.5f);
    ASSERT_EQ(state.positionY, 20.3f);
    ASSERT_EQ(state.positionZ, 0.0f);
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
    
    // Create a corrupted save file (missing version)
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
    TEST(testEmptyDirectory);
    
    std::cout << "\n=== Results ===\n";
    std::cout << "Passed: " << testsPassed << "\n";
    std::cout << "Failed: " << testsFailed << "\n";
    
    // Cleanup
    std::filesystem::remove_all("test_saves");
    std::filesystem::remove_all("test_saves_empty");
    
    return testsFailed == 0 ? 0 : 1;
}