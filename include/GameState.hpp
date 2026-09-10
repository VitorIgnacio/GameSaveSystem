#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <vector>

namespace GameSave {

struct CollectibleState {
    float positionX = 0.0f;
    float positionY = 0.0f;
    int value = 1;
    int type = 0;
    bool active = true;
    float bobOffset = 0.0f;
    float bobSpeed = 1.0f;
};

struct GameState {
    std::string playerName = "Vitor";
    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 1000;
    int health = 100;
    int maxHealth = 100;
    int mana = 100;
    int maxMana = 100;
    int coins = 0;
    float positionX = 400.0f;
    float positionY = 300.0f;
    std::string currentMap = "TrainingGround";
    std::chrono::seconds playtime{0};
    std::chrono::system_clock::time_point saveTime{};
    std::vector<CollectibleState> collectibles;
    bool collectibleDataAvailable = false;
};

inline int getExperienceForLevel(int level) {
    return 1000 * level;
}

inline bool checkLevelUp(GameState& state) {
    bool leveledUp = false;
    while (state.experienceToNextLevel > 0 && state.experience >= state.experienceToNextLevel) {
        state.experience -= state.experienceToNextLevel;
        state.level++;
        state.experienceToNextLevel = getExperienceForLevel(state.level);
        state.maxHealth += 10;
        state.maxMana += 10;
        state.health = state.maxHealth;
        state.mana = state.maxMana;
        leveledUp = true;
    }
    return leveledUp;
}

} // namespace GameSave