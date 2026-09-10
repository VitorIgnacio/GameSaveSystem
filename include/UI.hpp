#pragma once

#include "Game.hpp"
#include <string>
#include <raylib.h>

namespace GameSave {

struct Button {
    Rectangle bounds;
    std::string text;
    bool hovered = false;
    bool pressed = false;
    bool enabled = true;
    Color normalColor = { 50, 50, 60, 255 };
    Color hoverColor = { 70, 70, 85, 255 };
    Color pressColor = { 90, 90, 110, 255 };
    Color disabledColor = { 40, 40, 45, 255 };
    Color textColor = WHITE;
    int fontSize = 20;
};

struct Menu {
    std::string title;
    std::vector<Button> buttons;
    Rectangle panelBounds;
    bool centered = true;
    Color panelColor = { 25, 25, 30, 240 };
    Color borderColor = { 80, 80, 100, 255 };
};

class UI {
public:
    UI(int screenWidth, int screenHeight);
    
    void updateMouse();
    bool isButtonPressed(const Button& button) const;
    
    void drawMainMenu(const Menu& menu) const;
    void drawPauseMenu(const Menu& menu) const;
    void drawSaveMenu(const Menu& menu, const Game& game) const;
    void drawLoadMenu(const Menu& menu, const Game& game) const;
    void drawSettingsMenu(const Menu& menu, const Game& game) const;
    void drawErrorMessage(const std::string& message) const;
    void drawHUD(const Game& game) const;
    void drawStatusMessage(const std::string& message, Color color, float alpha) const;
    
    void createMainMenu(Menu& menu);
    void createPauseMenu(Menu& menu);
    void createSaveMenu(Menu& menu);
    void createLoadMenu(Menu& menu, const Game& game);
    void createSettingsMenu(Menu& menu);
    
    void updateSaveMenuButtons(Menu& menu, const Game& game);
    void updateLoadMenuButtons(Menu& menu, const Game& game);
    
    int screenWidth;
    int screenHeight;
    Font font;
    
private:
    void drawButton(const Button& button) const;
    void drawPanel(const Rectangle& bounds, Color color, Color borderColor) const;
    void drawTextCentered(const std::string& text, Rectangle bounds, int fontSize, Color color) const;
    void drawTextLeft(const std::string& text, Vector2 pos, int fontSize, Color color) const;
    Color getHealthColor(float ratio) const;
    Color getManaColor(float ratio) const;
    std::string formatTime(std::chrono::seconds time) const;
};

} // namespace GameSave