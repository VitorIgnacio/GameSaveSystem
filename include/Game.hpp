#pragma once

#include "GameState.hpp"
#include "SaveSystem.hpp"
#include <vector>
#include <string>
#include <raylib.h>

namespace GameSave {

struct Collectible {
    Vector2 position;
    int value;
    bool active = true;
    enum Type { Coin, Experience, Health, Mana } type;
    float bobOffset = 0.0f;
    float bobSpeed = 1.0f;
};

struct FloatingText {
    Vector2 position;
    std::string text;
    float lifetime = 0.0f;
    float maxLifetime = 1.5f;
    Color color = WHITE;
    bool active = true;
};

class Game {
public:
    Game(SaveSystem& saveSystem);
    
    void initNewGame();
    void update(float dt);
    void draw() const;
    
    void handleInput(float dt);
    SaveError saveGame(int slot);
    SaveError loadGame(int slot);
    SaveError quickSave();
    SaveError quickLoad();
    
    const GameState& getState() const { return m_state; }
    GameState& getState() { return m_state; }
    const SaveSystem& getSaveSystem() const { return m_saveSystem; }
    SaveSystem& getSaveSystem() { return m_saveSystem; }
    
    int getQuickSaveSlot() const { return m_quickSaveSlot; }
    void setQuickSaveSlot(int slot) { m_quickSaveSlot = slot; }
    int getLastLoadedSlot() const { return m_lastLoadedSlot; }
    void setLastLoadedSlot(int slot) { m_lastLoadedSlot = slot; }
    
    bool isNewGameRequested() const { return m_newGameRequested; }
    const std::string& getStatusMessage() const { return m_statusMessage; }
    Color getStatusMessageColor() const { return m_statusMessageColor; }
    float getStatusMessageAlpha() const;
    
    void addFloatingText(const Vector2& pos, const std::string& text, Color color = WHITE);
    void showStatusMessage(const std::string& message, Color color = WHITE);
    
    enum class State {
        MainMenu,
        Playing,
        PauseMenu,
        SaveMenu,
        LoadMenu,
        SettingsMenu,
        ErrorMessage
    };
    
    State getCurrentState() const { return m_currentState; }
    void setCurrentState(State state) { m_currentState = state; }
    
    const std::string& getErrorMessage() const { return m_errorMessage; }
    void setErrorMessage(const std::string& msg) { m_errorMessage = msg; m_currentState = State::ErrorMessage; }
    
    int getSelectedSlot() const { return m_selectedSlot; }
    void setSelectedSlot(int slot) { m_selectedSlot = slot; }
    
    void setNewGameRequested(bool v) { m_newGameRequested = v; }

private:
    SaveSystem& m_saveSystem;
    GameState m_state;
    
    std::vector<Collectible> m_collectibles;
    std::vector<FloatingText> m_floatingTexts;
    
    State m_currentState = State::MainMenu;
    std::string m_errorMessage;
    int m_selectedSlot = 1;
    int m_quickSaveSlot = 1;
    int m_lastLoadedSlot = -1;
    bool m_newGameRequested = false;
    std::string m_statusMessage;
    Color m_statusMessageColor = WHITE;
    float m_statusMessageTimer = 0.0f;
    float m_statusMessageDuration = 1.0f;
    
    Rectangle m_playerRect;
    float m_playerSpeed = 200.0f;
    int m_screenWidth = 800;
    int m_screenHeight = 600;
    Font m_font;
    
    // Camera system
    Camera2D m_camera;
    float m_worldWidth = 1600;
    float m_worldHeight = 1200;
    
    // Character growth animation
    float m_currentScale = 1.0f;
    float m_targetScale = 1.0f;
    bool m_isLevelingUp = false;
    float m_levelUpTimer = 0.0f;
    float m_levelUpDuration = 0.5f;
    
    void spawnCollectiblesOrganized();
    void updateCollectibles(float dt);
    void updateFloatingTexts(float dt);
    void updateCamera(float dt);
    void updateGrowthAnimation(float dt);
    void updateStatusMessage(float dt);
    void checkCollisions();
    void checkLevelUpAndAnimate();
    void applyLoadedState(const GameState& loadedState);
    void syncPlayerGeometry();
    float calculateTargetScale() const;
    float calculatePlayerRadius() const;
    void drawPlayer() const;
    void drawCollectibles() const;
    void drawFloatingTexts() const;
    void drawBackground() const;
    
public:
    void setScreenSize(int width, int height) { m_screenWidth = width; m_screenHeight = height; }
    void setFont(const Font& font) { m_font = font; }
    float getCurrentScale() const { return m_currentScale; }
    float getTargetScale() const { return m_targetScale; }
    const Camera2D& getCamera() const { return m_camera; }
};

} // namespace GameSave