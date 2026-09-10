#include "Game.hpp"
#include "UI.hpp"
#include <raylib.h>
#include <cmath>
#include <algorithm>

namespace GameSave {

Game::Game(SaveSystem& saveSystem)
    : m_saveSystem(saveSystem)
{
    m_camera.target = { m_state.positionX, m_state.positionY };
    m_camera.offset = { m_screenWidth / 2.0f, m_screenHeight / 2.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
    syncPlayerGeometry();
}

void Game::initNewGame() {
    m_state = GameState{};
    m_state.playerName = "Vitor";
    m_state.level = 1;
    m_state.experience = 0;
    m_state.experienceToNextLevel = getExperienceForLevel(1);
    m_state.health = 100;
    m_state.maxHealth = 100;
    m_state.mana = 100;
    m_state.maxMana = 100;
    m_state.coins = 0;
    m_state.positionX = m_worldWidth / 2.0f;
    m_state.positionY = m_worldHeight / 2.0f;
    m_state.currentMap = "Save System Arena";
    m_state.playtime = std::chrono::seconds(0);
    m_floatingTexts.clear();
    m_currentScale = 1.0f;
    m_targetScale = 1.0f;
    m_isLevelingUp = false;
    m_levelUpTimer = 0.0f;
    m_statusMessage.clear();
    m_statusMessageTimer = 0.0f;
    
    syncPlayerGeometry();
    spawnCollectiblesOrganized();
    m_currentState = State::Playing;
    m_newGameRequested = false;
}

void Game::spawnCollectiblesOrganized() {
    m_collectibles.clear();
    
    auto addCollectible = [this](float x, float y, int value, Collectible::Type type, float bobSpeed) {
        Collectible col;
        col.position = { x, y };
        col.value = value;
        col.type = type;
        col.bobOffset = static_cast<float>(m_collectibles.size()) * 0.7f;
        col.bobSpeed = bobSpeed;
        m_collectibles.push_back(col);
    };
    
    const float centerX = m_worldWidth / 2.0f;
    const float centerY = m_worldHeight / 2.0f;
    
    for (int i = 0; i < 12; ++i) {
        float angle = static_cast<float>(i) * 0.5235987756f;
        addCollectible(
            centerX + std::cos(angle) * 430.0f,
            centerY + std::sin(angle) * 270.0f,
            10 + (i % 3) * 10,
            Collectible::Coin,
            1.4f + static_cast<float>(i % 4) * 0.15f
        );
    }
    
    const Vector2 xpPositions[] = {
        { centerX - 210.0f, centerY - 130.0f },
        { centerX + 210.0f, centerY - 130.0f },
        { centerX - 210.0f, centerY + 130.0f },
        { centerX + 210.0f, centerY + 130.0f },
        { centerX, centerY - 230.0f },
        { centerX, centerY + 230.0f }
    };
    for (int i = 0; i < 6; ++i) {
        addCollectible(xpPositions[i].x, xpPositions[i].y, 100, Collectible::Experience, 1.8f);
    }
    
    const Vector2 utilityPositions[] = {
        { centerX - 500.0f, centerY },
        { centerX + 500.0f, centerY },
        { centerX, centerY - 330.0f },
        { centerX, centerY + 330.0f }
    };
    addCollectible(utilityPositions[0].x, utilityPositions[0].y, 25, Collectible::Health, 1.2f);
    addCollectible(utilityPositions[1].x, utilityPositions[1].y, 25, Collectible::Mana, 1.2f);
    addCollectible(utilityPositions[2].x, utilityPositions[2].y, 25, Collectible::Health, 1.2f);
    addCollectible(utilityPositions[3].x, utilityPositions[3].y, 25, Collectible::Mana, 1.2f);
}

void Game::update(float dt) {
    if (m_currentState == State::Playing) {
        m_state.playtime += std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::milliseconds(static_cast<int>(dt * 1000.0f)));
        handleInput(dt);
        updateCollectibles(dt);
        checkCollisions();
    }
    
    updateFloatingTexts(dt);
    updateGrowthAnimation(dt);
    updateCamera(0.0f);
    updateStatusMessage(dt);
}

void Game::handleInput(float dt) {
    // Movement
    float moveX = 0.0f, moveY = 0.0f;
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveY -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveY += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveX -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveX += 1.0f;
    
    // Normalize diagonal movement
    if (moveX != 0.0f && moveY != 0.0f) {
        float len = std::sqrt(moveX * moveX + moveY * moveY);
        moveX /= len;
        moveY /= len;
    }
    
    m_state.positionX += moveX * m_playerSpeed * dt;
    m_state.positionY += moveY * m_playerSpeed * dt;
    
    float radius = calculatePlayerRadius();
    m_state.positionX = std::clamp(m_state.positionX, radius, m_worldWidth - radius);
    m_state.positionY = std::clamp(m_state.positionY, radius, m_worldHeight - radius);
    
    syncPlayerGeometry();
    
    if (IsKeyPressed(KEY_H)) {
        m_state.health = std::max(0, m_state.health - 10);
        addFloatingText({ m_state.positionX, m_state.positionY - 30 }, "-10 HP", RED);
    }
    if (IsKeyPressed(KEY_J)) {
        m_state.health = std::min(m_state.maxHealth, m_state.health + 10);
        addFloatingText({ m_state.positionX, m_state.positionY - 30 }, "+10 HP", GREEN);
    }
    if (IsKeyPressed(KEY_M)) {
        m_state.mana = std::max(0, m_state.mana - 10);
        addFloatingText({ m_state.positionX, m_state.positionY - 30 }, "-10 MANA", BLUE);
    }
    if (IsKeyPressed(KEY_N)) {
        m_state.mana = std::min(m_state.maxMana, m_state.mana + 10);
        addFloatingText({ m_state.positionX, m_state.positionY - 30 }, "+10 MANA", BLUE);
    }
    
    // Pause menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (m_currentState == State::Playing) {
            m_currentState = State::PauseMenu;
        } else if (m_currentState == State::PauseMenu) {
            m_currentState = State::Playing;
        } else if (m_currentState == State::SaveMenu || m_currentState == State::LoadMenu || m_currentState == State::SettingsMenu) {
            m_currentState = State::PauseMenu;
        }
    }
    
    // Quick save/load
    if (IsKeyPressed(KEY_F5)) {
        auto error = quickSave();
        if (error == SaveError::None) {
        } else {
            setErrorMessage("Quick Save Failed: " + std::to_string(static_cast<int>(error)));
        }
    }
    if (IsKeyPressed(KEY_F9)) {
        auto error = quickLoad();
        if (error == SaveError::None) {
        } else {
            setErrorMessage("Quick Load Failed: " + std::to_string(static_cast<int>(error)));
        }
    }
}

void Game::updateCollectibles(float dt) {
    for (auto& col : m_collectibles) {
        if (!col.active) continue;
        col.bobOffset += col.bobSpeed * dt;
    }
}

void Game::updateFloatingTexts(float dt) {
    for (auto& ft : m_floatingTexts) {
        if (!ft.active) continue;
        ft.lifetime += dt;
        ft.position.y -= 30.0f * dt; // Float upward
        if (ft.lifetime >= ft.maxLifetime) {
            ft.active = false;
        }
    }
    // Remove inactive
    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                       [](const FloatingText& ft) { return !ft.active; }),
        m_floatingTexts.end());
}

void Game::updateGrowthAnimation(float dt) {
    m_targetScale = calculateTargetScale();
    float scaleDifference = m_targetScale - m_currentScale;
    if (std::abs(scaleDifference) > 0.001f) {
        float step = scaleDifference * std::min(1.0f, dt * 8.0f);
        m_currentScale += step;
        if (std::abs(m_targetScale - m_currentScale) <= 0.001f) {
            m_currentScale = m_targetScale;
        }
    }
    
    if (m_isLevelingUp) {
        m_levelUpTimer += dt;
        if (m_levelUpTimer >= m_levelUpDuration) {
            m_isLevelingUp = false;
            m_levelUpTimer = 0.0f;
        }
    }
    
    syncPlayerGeometry();
}

void Game::updateStatusMessage(float dt) {
    if (m_statusMessage.empty()) return;
    
    m_statusMessageTimer -= dt;
    if (m_statusMessageTimer <= 0.0f) {
        m_statusMessage.clear();
        m_statusMessageTimer = 0.0f;
    }
}

void Game::checkCollisions() {
    float collectionRadius = calculatePlayerRadius() + 12.0f;
    
    for (auto& col : m_collectibles) {
        if (!col.active) continue;
        
        float dx = m_state.positionX - col.position.x;
        float dy = m_state.positionY - col.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist < collectionRadius) {
            col.active = false;
            
            if (col.type == Collectible::Coin) {
                m_state.coins += col.value;
                addFloatingText(col.position, "+" + std::to_string(col.value) + " COINS", GOLD);
            } else if (col.type == Collectible::Experience) {
                m_state.experience += col.value;
                addFloatingText(col.position, "+" + std::to_string(col.value) + " XP", PURPLE);
                
                if (checkLevelUp(m_state)) {
                    checkLevelUpAndAnimate();
                }
            }
        }
    }
}

void Game::addFloatingText(const Vector2& pos, const std::string& text, Color color) {
    FloatingText ft;
    ft.position = pos;
    ft.text = text;
    ft.color = color;
    ft.lifetime = 0.0f;
    ft.active = true;
    m_floatingTexts.push_back(ft);
}

void Game::showStatusMessage(const std::string& message, Color color) {
    m_statusMessage = message;
    m_statusMessageColor = color;
    m_statusMessageTimer = m_statusMessageDuration;
}

float Game::getStatusMessageAlpha() const {
    if (m_statusMessage.empty() || m_statusMessageTimer <= 0.0f) return 0.0f;
    float fadeStart = m_statusMessageDuration * 0.65f;
    if (m_statusMessageTimer < fadeStart) {
        return m_statusMessageTimer / fadeStart;
    }
    return 1.0f;
}

void Game::checkLevelUpAndAnimate() {
    m_isLevelingUp = true;
    m_levelUpTimer = 0.0f;
    m_targetScale = calculateTargetScale();
    addFloatingText({ m_state.positionX, m_state.positionY - 55 }, "LEVEL UP!", { 255, 205, 90, 255 });
}

void Game::applyLoadedState(const GameState& loadedState) {
    m_state = loadedState;
    m_floatingTexts.clear();
    m_isLevelingUp = false;
    m_levelUpTimer = 0.0f;
    m_currentScale = calculateTargetScale();
    m_targetScale = m_currentScale;
    
    m_collectibles.clear();
    if (m_state.collectibleDataAvailable) {
        for (const auto& saved : m_state.collectibles) {
            if (saved.type < Collectible::Coin || saved.type > Collectible::Mana) continue;
            Collectible col;
            col.position = { saved.positionX, saved.positionY };
            col.value = saved.value;
            col.type = static_cast<Collectible::Type>(saved.type);
            col.active = saved.active;
            col.bobOffset = saved.bobOffset;
            col.bobSpeed = saved.bobSpeed;
            m_collectibles.push_back(col);
        }
    } else {
        spawnCollectiblesOrganized();
    }
    
    syncPlayerGeometry();
    updateCamera(0.0f);
}

void Game::draw() const {
    drawBackground();
    drawCollectibles();
    drawPlayer();
    drawFloatingTexts();
}

void Game::drawBackground() const {
    DrawRectangle(0, 0, static_cast<int>(m_worldWidth), static_cast<int>(m_worldHeight), { 28, 38, 34, 255 });
    
    for (int x = 0; x <= static_cast<int>(m_worldWidth); x += 80) {
        DrawLine(x, 0, x, static_cast<int>(m_worldHeight), { 38, 50, 45, 180 });
    }
    for (int y = 0; y <= static_cast<int>(m_worldHeight); y += 80) {
        DrawLine(0, y, static_cast<int>(m_worldWidth), y, { 38, 50, 45, 180 });
    }
    
    DrawRectangleLinesEx({ 40, 40, m_worldWidth - 80, m_worldHeight - 80 }, 4, { 75, 95, 82, 255 });
    DrawRectangleLinesEx({ 72, 72, m_worldWidth - 144, m_worldHeight - 144 }, 2, { 52, 68, 58, 220 });
    
    float centerX = m_worldWidth / 2.0f;
    float centerY = m_worldHeight / 2.0f;
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), 72, { 42, 58, 50, 220 });
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), 72, { 95, 125, 102, 255 });
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), 48, { 70, 95, 78, 220 });
    
    Vector2 mapLabel = { 70, 62 };
    DrawText(m_state.currentMap.c_str(), static_cast<int>(mapLabel.x), static_cast<int>(mapLabel.y), 18, { 135, 160, 140, 255 });
    DrawText("SAVE / LOAD DEMO", static_cast<int>(mapLabel.x), static_cast<int>(mapLabel.y + 24), 13, { 95, 115, 100, 230 });
}

void Game::drawPlayer() const {
    float radius = calculatePlayerRadius();
    float pulse = 1.0f;
    if (m_isLevelingUp && m_levelUpDuration > 0.0f) {
        float progress = std::clamp(m_levelUpTimer / m_levelUpDuration, 0.0f, 1.0f);
        pulse += std::sin(progress * 3.1415926535f) * 0.14f;
    }
    float visualRadius = radius * pulse;
    
    DrawCircle(static_cast<int>(m_state.positionX), static_cast<int>(m_state.positionY + visualRadius * 0.32f), static_cast<int>(visualRadius * 0.95f), { 0, 0, 0, 90 });
    DrawCircle(static_cast<int>(m_state.positionX), static_cast<int>(m_state.positionY), static_cast<int>(visualRadius * 1.28f), { 55, 125, 190, 42 });
    DrawCircle(static_cast<int>(m_state.positionX), static_cast<int>(m_state.positionY), static_cast<int>(visualRadius), { 62, 145, 210, 255 });
    DrawCircleLines(static_cast<int>(m_state.positionX), static_cast<int>(m_state.positionY), static_cast<int>(visualRadius), { 155, 215, 255, 255 });
    DrawCircleLines(static_cast<int>(m_state.positionX), static_cast<int>(m_state.positionY), static_cast<int>(visualRadius * 0.62f), { 125, 195, 240, 150 });
    
    Vector2 dir = { 0, -1 };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dir = { 0, -1 };
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dir = { 0, 1 };
    else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dir = { -1, 0 };
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir = { 1, 0 };
    
    float indicatorOffset = visualRadius * 0.42f;
    float indicatorRadius = std::max(3.0f, visualRadius * 0.22f);
    DrawCircle(static_cast<int>(m_state.positionX + dir.x * indicatorOffset),
               static_cast<int>(m_state.positionY + dir.y * indicatorOffset),
               static_cast<int>(indicatorRadius), { 220, 240, 255, 255 });
}

void Game::drawCollectibles() const {
    for (const auto& col : m_collectibles) {
        if (!col.active) continue;
        
        float bob = std::sin(col.bobOffset) * 3.0f;
        Vector2 pos = { col.position.x, col.position.y + bob };
        
        if (col.type == Collectible::Coin) {
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 215, 170, 35, 255 });
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 255, 225, 120, 255 });
            DrawText("$", static_cast<int>(pos.x - 3), static_cast<int>(pos.y - 7), 16, { 110, 82, 0, 255 });
        } else if (col.type == Collectible::Experience) {
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 145, 75, 205, 255 });
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 220, 160, 255, 255 });
            DrawText("XP", static_cast<int>(pos.x - 8), static_cast<int>(pos.y - 6), 12, { 255, 255, 255, 255 });
        } else if (col.type == Collectible::Health) {
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 170, 45, 55, 255 });
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 255, 120, 130, 255 });
            DrawText("+", static_cast<int>(pos.x - 3), static_cast<int>(pos.y - 8), 18, WHITE);
        } else {
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 45, 95, 190, 255 });
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 10, { 125, 175, 255, 255 });
            DrawText("M", static_cast<int>(pos.x - 4), static_cast<int>(pos.y - 7), 16, { 220, 235, 255, 255 });
        }
    }
}

void Game::drawFloatingTexts() const {
    for (const auto& ft : m_floatingTexts) {
        if (!ft.active) continue;
        
        float alpha = 1.0f - (ft.lifetime / ft.maxLifetime);
        Color c = ft.color;
        c.a = static_cast<unsigned char>(255 * alpha);
        
        Vector2 textSize = MeasureTextEx(m_font, ft.text.c_str(), 18, 1);
        DrawTextEx(m_font, ft.text.c_str(), 
                  { ft.position.x - textSize.x / 2, ft.position.y }, 18, 1, c);
    }
}

float Game::calculateTargetScale() const {
    constexpr float baseScale = 1.0f;
    constexpr float scalePerLevel = 0.10f;
    constexpr float maxScale = 1.80f;
    int safeLevel = std::max(1, m_state.level);
    return std::min(baseScale + static_cast<float>(safeLevel - 1) * scalePerLevel, maxScale);
}

float Game::calculatePlayerRadius() const {
    constexpr float baseRadius = 16.0f;
    return baseRadius * m_currentScale;
}

void Game::syncPlayerGeometry() {
    float radius = calculatePlayerRadius();
    m_playerRect.x = m_state.positionX - radius;
    m_playerRect.y = m_state.positionY - radius;
    m_playerRect.width = radius * 2.0f;
    m_playerRect.height = radius * 2.0f;
}

void Game::updateCamera(float dt) {
    (void)dt;
    m_camera.target = { m_state.positionX, m_state.positionY };
    m_camera.offset = { m_screenWidth / 2.0f, m_screenHeight / 2.0f };
    
    float viewWidth = static_cast<float>(m_screenWidth) / m_camera.zoom;
    float viewHeight = static_cast<float>(m_screenHeight) / m_camera.zoom;
    if (viewWidth < m_worldWidth) {
        m_camera.target.x = std::clamp(m_camera.target.x, viewWidth / 2.0f, m_worldWidth - viewWidth / 2.0f);
    } else {
        m_camera.target.x = m_worldWidth / 2.0f;
    }
    if (viewHeight < m_worldHeight) {
        m_camera.target.y = std::clamp(m_camera.target.y, viewHeight / 2.0f, m_worldHeight - viewHeight / 2.0f);
    } else {
        m_camera.target.y = m_worldHeight / 2.0f;
    }
}

SaveError Game::saveGame(int slot) {
    syncPlayerGeometry();
    m_state.positionX = m_playerRect.x + m_playerRect.width / 2.0f;
    m_state.positionY = m_playerRect.y + m_playerRect.height / 2.0f;
    m_state.collectibles.clear();
    for (const auto& col : m_collectibles) {
        CollectibleState saved;
        saved.positionX = col.position.x;
        saved.positionY = col.position.y;
        saved.value = col.value;
        saved.type = static_cast<int>(col.type);
        saved.active = col.active;
        saved.bobOffset = col.bobOffset;
        saved.bobSpeed = col.bobSpeed;
        m_state.collectibles.push_back(saved);
    }
    m_state.collectibleDataAvailable = true;
    
    SaveError error = m_saveSystem.save(slot, m_state);
    if (error == SaveError::None) {
        showStatusMessage("GAME SAVED", { 120, 230, 140, 255 });
    }
    return error;
}

SaveError Game::loadGame(int slot) {
    GameState loadedState;
    SaveError error = m_saveSystem.load(slot, loadedState);
    if (error == SaveError::None) {
        applyLoadedState(loadedState);
        showStatusMessage("GAME LOADED", { 130, 190, 255, 255 });
    }
    return error;
}

SaveError Game::quickSave() {
    return saveGame(m_quickSaveSlot);
}

SaveError Game::quickLoad() {
    if (m_lastLoadedSlot <= 0) {
        return SaveError::SlotNotFound;
    }
    return loadGame(m_lastLoadedSlot);
}

} // namespace GameSave