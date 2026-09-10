#include "UI.hpp"
#include "Game.hpp"
#include "SaveSystem.hpp"
#include <raylib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace GameSave {

UI::UI(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
    font = GetFontDefault();
}

void UI::updateMouse() {
    // Mouse state is handled by raylib directly
}

bool UI::isButtonPressed(const Button& button) const {
    if (!button.enabled) return false;
    Vector2 mousePos = GetMousePosition();
    bool over = CheckCollisionPointRec(mousePos, button.bounds);
    return over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void UI::drawButton(const Button& button) const {
    Color bgColor;
    if (!button.enabled) {
        bgColor = button.disabledColor;
    } else if (button.pressed) {
        bgColor = button.pressColor;
    } else if (button.hovered) {
        bgColor = button.hoverColor;
    } else {
        bgColor = button.normalColor;
    }
    
    DrawRectangleRec(button.bounds, bgColor);
    DrawRectangleLinesEx(button.bounds, 2, { 100, 100, 120, 255 });
    
    Vector2 textSize = MeasureTextEx(font, button.text.c_str(), button.fontSize, 1);
    DrawText(button.text.c_str(),
             static_cast<int>(button.bounds.x + (button.bounds.width - textSize.x) / 2),
             static_cast<int>(button.bounds.y + (button.bounds.height - textSize.y) / 2),
             button.fontSize, button.textColor);
}

void UI::drawPanel(const Rectangle& bounds, Color color, Color borderColor) const {
    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 2, borderColor);
}

void UI::drawTextCentered(const std::string& text, Rectangle bounds, int fontSize, Color color) const {
    Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, 1);
    DrawText(text.c_str(),
             static_cast<int>(bounds.x + (bounds.width - textSize.x) / 2),
             static_cast<int>(bounds.y + (bounds.height - textSize.y) / 2),
             fontSize, color);
}

void UI::drawTextLeft(const std::string& text, Vector2 pos, int fontSize, Color color) const {
    DrawText(text.c_str(), static_cast<int>(pos.x), static_cast<int>(pos.y), fontSize, color);
}

Color UI::getHealthColor(float ratio) const {
    if (ratio > 0.6f) return { 80, 220, 80, 255 };
    if (ratio > 0.3f) return { 255, 200, 50, 255 };
    return { 255, 60, 60, 255 };
}

Color UI::getManaColor(float ratio) const {
    if (ratio > 0.6f) return { 80, 140, 255, 255 };
    if (ratio > 0.3f) return { 140, 100, 255, 255 };
    return { 100, 60, 255, 255 };
}

std::string UI::formatTime(std::chrono::seconds time) const {
    int totalSeconds = static_cast<int>(time.count());
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    
    std::stringstream ss;
    if (hours > 0) {
        ss << hours << "h " << minutes << "m " << seconds << "s";
    } else if (minutes > 0) {
        ss << minutes << "m " << seconds << "s";
    } else {
        ss << seconds << "s";
    }
    return ss.str();
}

void UI::createMainMenu(Menu& menu) {
    menu.title = "GAME SAVE SYSTEM";
    
    float panelWidth = 400;
    float panelHeight = 400;
    menu.panelBounds = { 
        screenWidth / 2.0f - panelWidth / 2.0f, 
        screenHeight / 2.0f - panelHeight / 2.0f, 
        panelWidth, panelHeight 
    };
    menu.buttons.clear();
    
    float btnWidth = 300;
    float btnHeight = 50;
    float startY = menu.panelBounds.y + 100;
    float spacing = 70;
    float centerX = screenWidth / 2.0f - btnWidth / 2.0f;
    
    menu.buttons.push_back({ { centerX, startY, btnWidth, btnHeight }, "NEW GAME", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing, btnWidth, btnHeight }, "LOAD GAME", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing * 2, btnWidth, btnHeight }, "SETTINGS", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing * 3, btnWidth, btnHeight }, "EXIT", false, false, true });
}

void UI::createPauseMenu(Menu& menu) {
    menu.title = "PAUSED";
    
    float panelWidth = 300;
    float panelHeight = 350;
    menu.panelBounds = { 
        screenWidth / 2.0f - panelWidth / 2.0f, 
        screenHeight / 2.0f - panelHeight / 2.0f, 
        panelWidth, panelHeight 
    };
    menu.buttons.clear();
    
    float btnWidth = 220;
    float btnHeight = 45;
    float startY = menu.panelBounds.y + 80;
    float spacing = 60;
    float centerX = screenWidth / 2.0f - btnWidth / 2.0f;
    
    menu.buttons.push_back({ { centerX, startY, btnWidth, btnHeight }, "RESUME", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing, btnWidth, btnHeight }, "SAVE GAME", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing * 2, btnWidth, btnHeight }, "LOAD GAME", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing * 3, btnWidth, btnHeight }, "MAIN MENU", false, false, true });
    menu.buttons.push_back({ { centerX, startY + spacing * 4, btnWidth, btnHeight }, "EXIT", false, false, true });
}

void UI::createSaveMenu(Menu& menu) {
    menu.title = "SAVE GAME";
    
    float panelWidth = std::min(600.0f, screenWidth * 0.85f);
    float panelHeight = std::min(500.0f, screenHeight * 0.85f);
    menu.panelBounds = { 
        screenWidth / 2.0f - panelWidth / 2.0f, 
        screenHeight / 2.0f - panelHeight / 2.0f, 
        panelWidth, panelHeight 
    };
    menu.buttons.clear();
    
    // Slot buttons will be updated dynamically
}

void UI::createLoadMenu(Menu& menu, const Game& game) {
    menu.title = "LOAD GAME";
    
    float panelWidth = std::min(600.0f, screenWidth * 0.85f);
    float panelHeight = std::min(500.0f, screenHeight * 0.85f);
    menu.panelBounds = { 
        screenWidth / 2.0f - panelWidth / 2.0f, 
        screenHeight / 2.0f - panelHeight / 2.0f, 
        panelWidth, panelHeight 
    };
    menu.buttons.clear();
    
    updateLoadMenuButtons(menu, game);
}

void UI::createSettingsMenu(Menu& menu) {
    menu.title = "SETTINGS";
    
    float panelWidth = 400;
    float panelHeight = 300;
    menu.panelBounds = { 
        screenWidth / 2.0f - panelWidth / 2.0f, 
        screenHeight / 2.0f - panelHeight / 2.0f, 
        panelWidth, panelHeight 
    };
    menu.buttons.clear();
    
    float btnWidth = 300;
    float btnHeight = 50;
    float startY = menu.panelBounds.y + 100;
    float centerX = screenWidth / 2.0f - btnWidth / 2.0f;
    
    menu.buttons.push_back({ { centerX, startY, btnWidth, btnHeight }, "QUICK SAVE SLOT: 1", false, false, true });
    menu.buttons.push_back({ { centerX, startY + 70, btnWidth, btnHeight }, "BACK", false, false, true });
}

void UI::updateSaveMenuButtons(Menu& menu, const Game& game) {
    menu.buttons.clear();
    
    auto saves = game.getSaveSystem().listSaves();
    float btnWidth = std::min(560.0f, menu.panelBounds.width - 80);
    float btnHeight = 80;
    float startY = menu.panelBounds.y + 80;
    float spacing = 95;
    float centerX = screenWidth / 2.0f - btnWidth / 2.0f;
    
    for (int slot = 1; slot <= 5; ++slot) {
        bool found = false;
        const SaveInfo* saveInfo = nullptr;
        for (const auto& s : saves) {
            if (s.slot == slot) {
                found = true;
                saveInfo = &s;
                break;
            }
        }
        
        std::string btnText;
        if (found) {
            auto timeT = std::chrono::system_clock::to_time_t(saveInfo->saveTime);
            std::tm* tm = std::localtime(&timeT);
            char timeStr[64];
            std::strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", tm);
            
            btnText = "Slot " + std::to_string(slot) + "  |  Level " + std::to_string(saveInfo->level) + 
                     "  |  " + saveInfo->playerName + "  |  " + timeStr;
        } else {
            btnText = "Slot " + std::to_string(slot) + "  |  Empty";
        }
        
        Button btn;
        btn.bounds = { centerX, startY + (slot - 1) * spacing, btnWidth, btnHeight };
        btn.text = btnText;
        btn.enabled = true;
        btn.fontSize = 18;
        menu.buttons.push_back(btn);
    }
    
    // Back button
    Button backBtn;
    backBtn.bounds = { centerX, startY + 5 * spacing, btnWidth, 45 };
    backBtn.text = "BACK";
    backBtn.fontSize = 20;
    menu.buttons.push_back(backBtn);
}

void UI::updateLoadMenuButtons(Menu& menu, const Game& game) {
    menu.buttons.clear();
    
    auto saves = game.getSaveSystem().listSaves();
    float btnWidth = std::min(560.0f, menu.panelBounds.width - 80);
    float btnHeight = 80;
    float startY = menu.panelBounds.y + 80;
    float spacing = 95;
    float centerX = screenWidth / 2.0f - btnWidth / 2.0f;
    
    for (int slot = 1; slot <= 5; ++slot) {
        bool found = false;
        const SaveInfo* saveInfo = nullptr;
        for (const auto& s : saves) {
            if (s.slot == slot) {
                found = true;
                saveInfo = &s;
                break;
            }
        }
        
        std::string btnText;
        if (found) {
            auto timeT = std::chrono::system_clock::to_time_t(saveInfo->saveTime);
            std::tm* tm = std::localtime(&timeT);
            char timeStr[64];
            std::strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", tm);
            
            btnText = "Slot " + std::to_string(slot) + "  |  Level " + std::to_string(saveInfo->level) + 
                     "  |  " + saveInfo->playerName + "  |  " + timeStr;
        } else {
            btnText = "Slot " + std::to_string(slot) + "  |  Empty";
        }
        
        Button btn;
        btn.bounds = { centerX, startY + (slot - 1) * spacing, btnWidth, btnHeight };
        btn.text = btnText;
        btn.enabled = found; // Disable empty slots
        btn.fontSize = 18;
        menu.buttons.push_back(btn);
    }
    
    // Back button
    Button backBtn;
    backBtn.bounds = { centerX, startY + 5 * spacing, btnWidth, 45 };
    backBtn.text = "BACK";
    backBtn.fontSize = 20;
    menu.buttons.push_back(backBtn);
}

void UI::drawMainMenu(const Menu& menu) const {
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 200 });
    
    // Panel
    drawPanel(menu.panelBounds, menu.panelColor, menu.borderColor);
    
    // Title
    Rectangle titleRect = { menu.panelBounds.x, menu.panelBounds.y + 20, menu.panelBounds.width, 60 };
    drawTextCentered(menu.title, titleRect, 36, { 220, 220, 100, 255 });
    
    // Subtitle
    Rectangle subRect = { menu.panelBounds.x, menu.panelBounds.y + 70, menu.panelBounds.width, 30 };
    drawTextCentered("C++ Save System Demo", subRect, 16, { 120, 120, 140, 255 });
    
    // Buttons
    for (const auto& btn : menu.buttons) {
        drawButton(btn);
    }
}

void UI::drawPauseMenu(const Menu& menu) const {
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 180 });
    
    // Panel
    drawPanel(menu.panelBounds, menu.panelColor, menu.borderColor);
    
    // Title
    Rectangle titleRect = { menu.panelBounds.x, menu.panelBounds.y + 20, menu.panelBounds.width, 50 };
    drawTextCentered(menu.title, titleRect, 32, { 220, 220, 100, 255 });
    
    // Buttons
    for (const auto& btn : menu.buttons) {
        drawButton(btn);
    }
}

void UI::drawSaveMenu(const Menu& menu, const Game& game) const {
    (void)game;
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 200 });
    
    // Panel
    drawPanel(menu.panelBounds, menu.panelColor, menu.borderColor);
    
    // Title
    Rectangle titleRect = { menu.panelBounds.x, menu.panelBounds.y + 15, menu.panelBounds.width, 50 };
    drawTextCentered(menu.title, titleRect, 32, { 220, 220, 100, 255 });
    
    // Buttons (slots)
    for (const auto& btn : menu.buttons) {
        drawButton(btn);
    }
}

void UI::drawLoadMenu(const Menu& menu, const Game& game) const {
    (void)game;
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 200 });
    
    // Panel
    drawPanel(menu.panelBounds, menu.panelColor, menu.borderColor);
    
    // Title
    Rectangle titleRect = { menu.panelBounds.x, menu.panelBounds.y + 15, menu.panelBounds.width, 50 };
    drawTextCentered(menu.title, titleRect, 32, { 220, 220, 100, 255 });
    
    // Buttons (slots)
    for (const auto& btn : menu.buttons) {
        drawButton(btn);
    }
}

void UI::drawSettingsMenu(const Menu& menu, const Game& game) const {
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 200 });
    
    // Panel
    drawPanel(menu.panelBounds, menu.panelColor, menu.borderColor);
    
    // Title
    Rectangle titleRect = { menu.panelBounds.x, menu.panelBounds.y + 20, menu.panelBounds.width, 60 };
    drawTextCentered(menu.title, titleRect, 32, { 220, 220, 100, 255 });
    
    // Quick save slot indicator - use actual game quick save slot
    Rectangle slotRect = { menu.panelBounds.x + 20, menu.panelBounds.y + 100, menu.panelBounds.width - 40, 40 };
    drawTextLeft("Quick Save Slot: " + std::to_string(game.getQuickSaveSlot()), { slotRect.x, slotRect.y }, 20, WHITE);
    
    // Buttons
    for (const auto& btn : menu.buttons) {
        drawButton(btn);
    }
}

void UI::drawErrorMessage(const std::string& message) const {
    // Dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 200 });
    
    // Panel
    Rectangle panel = { 200, 200, 400, 200 };
    drawPanel(panel, { 40, 20, 20, 240 }, { 200, 60, 60, 255 });
    
    // Title
    Rectangle titleRect = { panel.x, panel.y + 20, panel.width, 40 };
    drawTextCentered("ERROR", titleRect, 28, { 255, 100, 100, 255 });
    
    // Message
    DrawText(message.c_str(), 
             static_cast<int>(panel.x + 20), 
             static_cast<int>(panel.y + 80), 18, WHITE);
    
    // OK button
    Button okBtn = { { panel.x + panel.width / 2 - 50, panel.y + 140, 100, 40 }, "OK", false, false, true };
    drawButton(okBtn);
}

void UI::drawHUD(const Game& game) const {
    const GameState& state = game.getState();
    int hudX = 10;
    int hudY = 10;
    int hudWidth = std::min(280, screenWidth - 20);
    int barWidth = std::max(120, hudWidth - 75);
    int controlsFontSize = screenWidth < 700 ? 11 : 14;
    
    DrawRectangle(hudX, hudY, hudWidth, 180, { 18, 22, 25, 215 });
    DrawRectangleLines(hudX, hudY, hudWidth, 180, { 68, 78, 76, 230 });
    DrawText(state.playerName.c_str(), hudX + 10, hudY + 6, 20, WHITE);
    DrawText(("LEVEL " + std::to_string(state.level)).c_str(), hudX + 10, hudY + 30, 17, { 205, 205, 120, 255 });
    
    int barX = hudX + 42;
    int barY = hudY + 60;
    int barHeight = 15;
    float hpRatio = static_cast<float>(state.health) / std::max(1, state.maxHealth);
    DrawRectangle(barX, barY, barWidth, barHeight, { 58, 22, 24, 255 });
    DrawRectangle(barX, barY, static_cast<int>(barWidth * hpRatio), barHeight, getHealthColor(hpRatio));
    DrawRectangleLines(barX, barY, barWidth, barHeight, { 95, 100, 96, 230 });
    DrawText("HP", hudX + 8, barY + 1, 13, WHITE);
    DrawText((std::to_string(state.health) + "/" + std::to_string(state.maxHealth)).c_str(),
             barX + barWidth + 5, barY - 1, 12, WHITE);
    
    barY = hudY + 85;
    float manaRatio = static_cast<float>(state.mana) / std::max(1, state.maxMana);
    DrawRectangle(barX, barY, barWidth, barHeight, { 22, 25, 58, 255 });
    DrawRectangle(barX, barY, static_cast<int>(barWidth * manaRatio), barHeight, getManaColor(manaRatio));
    DrawRectangleLines(barX, barY, barWidth, barHeight, { 95, 100, 96, 230 });
    DrawText("MANA", hudX + 8, barY + 1, 13, WHITE);
    DrawText((std::to_string(state.mana) + "/" + std::to_string(state.maxMana)).c_str(),
             barX + barWidth + 5, barY - 1, 12, WHITE);
    
    barY = hudY + 110;
    float xpRatio = state.experienceToNextLevel > 0 ?
        static_cast<float>(state.experience) / state.experienceToNextLevel : 0.0f;
    DrawRectangle(barX, barY, barWidth, barHeight, { 54, 22, 58, 255 });
    DrawRectangle(barX, barY, static_cast<int>(barWidth * std::clamp(xpRatio, 0.0f, 1.0f)), barHeight, { 175, 90, 220, 255 });
    DrawRectangleLines(barX, barY, barWidth, barHeight, { 95, 100, 96, 230 });
    DrawText("XP", hudX + 8, barY + 1, 13, WHITE);
    DrawText((std::to_string(state.experience) + "/" + std::to_string(state.experienceToNextLevel)).c_str(),
             barX + barWidth + 5, barY - 1, 12, WHITE);
    
    DrawText(("COINS: " + std::to_string(state.coins)).c_str(), hudX + 10, hudY + 145, 17, GOLD);
    DrawText(("SCALE: " + std::to_string(static_cast<int>(game.getCurrentScale() * 100)) + "%").c_str(),
             hudX + 10, hudY + 164, 12, { 150, 165, 155, 255 });
    
    const char* controlsText = "WASD/Arrows - Move  |  H/J - HP  |  M/N - Mana  |  ESC - Menu  |  F5 - Quick Save  |  F9 - Quick Load";
    Vector2 textSize = MeasureTextEx(font, controlsText, controlsFontSize, 1);
    DrawText(controlsText,
             screenWidth / 2 - static_cast<int>(textSize.x / 2),
             screenHeight - 24, controlsFontSize, { 105, 112, 108, 235 });
}

void UI::drawStatusMessage(const std::string& message, Color color, float alpha) const {
    int textWidth = std::max(150, MeasureText(message.c_str(), 20) + 48);
    int x = screenWidth / 2 - textWidth / 2;
    float clampedAlpha = std::clamp(alpha, 0.0f, 1.0f);
    Color faded = color;
    faded.a = static_cast<unsigned char>(255.0f * clampedAlpha);
    Color borderColor = faded;
    borderColor.r = std::min(255, borderColor.r + 45);
    borderColor.g = std::min(255, borderColor.g + 45);
    borderColor.b = std::min(255, borderColor.b + 45);
    
    DrawRectangle(x, 42, textWidth, 42, { 15, 18, 22, static_cast<unsigned char>(220 * clampedAlpha) });
    DrawRectangleLines(x, 42, textWidth, 42, borderColor);
    DrawText(message.c_str(), screenWidth / 2 - MeasureText(message.c_str(), 20) / 2, 51, 20, faded);
}

} // namespace GameSave