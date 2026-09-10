#include "SaveSystem.hpp"
#include "Game.hpp"
#include "UI.hpp"
#include <raylib.h>
#include <iostream>

using namespace GameSave;

int main() {
    // Window setup
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Game Save System Demo");
    SetTargetFPS(60);

    // Initialize systems
    SaveSystem saveSystem("saves");
    Game game(saveSystem);
    UI ui(screenWidth, screenHeight);

    // Set font for floating texts
    game.setFont(ui.font);

    Menu mainMenu;
    Menu pauseMenu;
    Menu saveMenu;
    Menu loadMenu;
    Menu settingsMenu;

    // Create menus once at startup
    ui.createMainMenu(mainMenu);
    ui.createPauseMenu(pauseMenu);
    ui.createSettingsMenu(settingsMenu);

    bool exitRequested = false;

    while (!WindowShouldClose() && !exitRequested) {
        float dt = GetFrameTime();

        // Handle window resize
        if (IsWindowResized()) {
            int newWidth = GetScreenWidth();
            int newHeight = GetScreenHeight();

            // Update UI dimensions
            ui.screenWidth = newWidth;
            ui.screenHeight = newHeight;

            // Update Game dimensions
            game.setScreenSize(newWidth, newHeight);

            // Recreate menus with new dimensions
            ui.createMainMenu(mainMenu);
            ui.createPauseMenu(pauseMenu);
            ui.createSettingsMenu(settingsMenu);
        }

        // Update mouse hover states for current menu
        Vector2 mousePos = GetMousePosition();
        Menu* currentMenu = nullptr;

        switch (game.getCurrentState()) {
            case Game::State::MainMenu:
                currentMenu = &mainMenu;
                break;
            case Game::State::PauseMenu:
                currentMenu = &pauseMenu;
                break;
            case Game::State::SaveMenu:
                currentMenu = &saveMenu;
                break;
            case Game::State::LoadMenu:
                currentMenu = &loadMenu;
                break;
            case Game::State::SettingsMenu:
                currentMenu = &settingsMenu;
                break;
            default:
                break;
        }

        if (currentMenu) {
            for (auto& btn : currentMenu->buttons) {
                btn.hovered = CheckCollisionPointRec(mousePos, btn.bounds) && btn.enabled;
                btn.pressed = btn.hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            }
        }

        // State machine
        switch (game.getCurrentState()) {
            case Game::State::MainMenu: {
                // Handle button clicks
                if (ui.isButtonPressed(mainMenu.buttons[0])) { // NEW GAME
                    game.initNewGame();
                } else if (ui.isButtonPressed(mainMenu.buttons[1])) { // LOAD GAME
                    ui.createLoadMenu(loadMenu, game);
                    game.setCurrentState(Game::State::LoadMenu);
                } else if (ui.isButtonPressed(mainMenu.buttons[2])) { // SETTINGS
                    // Update quick save slot text before showing
                    settingsMenu.buttons[0].text = "QUICK SAVE SLOT: " + std::to_string(game.getQuickSaveSlot());
                    game.setCurrentState(Game::State::SettingsMenu);
                } else if (ui.isButtonPressed(mainMenu.buttons[3])) { // EXIT
                    exitRequested = true;
                }
                break;
            }

            case Game::State::Playing: {
                game.update(dt);

                // Check for new game request from pause menu
                if (game.isNewGameRequested()) {
                    game.initNewGame();
                    game.setNewGameRequested(false);
                }
                break;
            }

            case Game::State::PauseMenu: {
                if (ui.isButtonPressed(pauseMenu.buttons[0])) { // RESUME
                    game.setCurrentState(Game::State::Playing);
                } else if (ui.isButtonPressed(pauseMenu.buttons[1])) { // SAVE GAME
                    ui.updateSaveMenuButtons(saveMenu, game);
                    game.setCurrentState(Game::State::SaveMenu);
                } else if (ui.isButtonPressed(pauseMenu.buttons[2])) { // LOAD GAME
                    ui.createLoadMenu(loadMenu, game);
                    game.setCurrentState(Game::State::LoadMenu);
                } else if (ui.isButtonPressed(pauseMenu.buttons[3])) { // MAIN MENU
                    game.setCurrentState(Game::State::MainMenu);
                } else if (ui.isButtonPressed(pauseMenu.buttons[4])) { // EXIT
                    exitRequested = true;
                }
                break;
            }

            case Game::State::SaveMenu: {
                // Check slot buttons (first 5)
                for (int i = 0; i < 5 && i < static_cast<int>(saveMenu.buttons.size() - 1); ++i) {
                    if (ui.isButtonPressed(saveMenu.buttons[i])) {
                        int slot = i + 1;
                        SaveError error = game.saveGame(slot);
                        if (error == SaveError::None) {
                            game.setQuickSaveSlot(slot);
                            game.setLastLoadedSlot(slot);
                            game.setCurrentState(Game::State::PauseMenu);
                        } else {
                            game.setErrorMessage("Failed to save game: " + std::to_string(static_cast<int>(error)));
                        }
                        break;
                    }
                }

                // Back button (last)
                if (ui.isButtonPressed(saveMenu.buttons.back())) {
                    game.setCurrentState(Game::State::PauseMenu);
                }
                break;
            }

            case Game::State::LoadMenu: {
                // Check slot buttons (first 5)
                for (int i = 0; i < 5 && i < static_cast<int>(loadMenu.buttons.size() - 1); ++i) {
                    if (ui.isButtonPressed(loadMenu.buttons[i])) {
                        int slot = i + 1;
                        auto error = game.loadGame(slot);
                        if (error == SaveError::None) {
                            game.setLastLoadedSlot(slot);
                            game.setQuickSaveSlot(slot);
                            game.setCurrentState(Game::State::Playing);
                        } else {
                            game.setErrorMessage("Failed to load save: " + std::to_string(static_cast<int>(error)));
                        }
                        break;
                    }
                }

                // Back button (last)
                if (ui.isButtonPressed(loadMenu.buttons.back())) {
                    game.setCurrentState(Game::State::PauseMenu);
                }
                break;
            }

            case Game::State::SettingsMenu: {
                if (ui.isButtonPressed(settingsMenu.buttons[0])) { // Quick save slot
                    // Cycle through slots 1-5
                    int newSlot = game.getQuickSaveSlot() + 1;
                    if (newSlot > 5) newSlot = 1;
                    game.setQuickSaveSlot(newSlot);
                    settingsMenu.buttons[0].text = "QUICK SAVE SLOT: " + std::to_string(newSlot);
                } else if (ui.isButtonPressed(settingsMenu.buttons[1])) { // BACK
                    game.setCurrentState(Game::State::MainMenu);
                }
                break;
            }

            case Game::State::ErrorMessage: {
                // Check OK button click
                Vector2 mousePos = GetMousePosition();
                Rectangle okBtnBounds = {
                    ui.screenWidth / 2.0f - 50,
                    ui.screenHeight / 2.0f + 40,
                    100, 40
                };
                if (CheckCollisionPointRec(mousePos, okBtnBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    game.setCurrentState(Game::State::PauseMenu);
                }
                break;
            }
        }

        // Render
        BeginDrawing();
        ClearBackground({ 20, 20, 25, 255 });

        switch (game.getCurrentState()) {
            case Game::State::MainMenu:
                ui.drawMainMenu(mainMenu);
                break;

            case Game::State::Playing:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                if (!game.getStatusMessage().empty()) {
                    ui.drawStatusMessage(game.getStatusMessage(), game.getStatusMessageColor(), game.getStatusMessageAlpha());
                }
                break;

            case Game::State::PauseMenu:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                ui.drawPauseMenu(pauseMenu);
                break;

            case Game::State::SaveMenu:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                ui.drawSaveMenu(saveMenu, game);
                break;

            case Game::State::LoadMenu:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                ui.drawLoadMenu(loadMenu, game);
                break;

            case Game::State::SettingsMenu:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                ui.drawSettingsMenu(settingsMenu, game);
                break;

            case Game::State::ErrorMessage:
                BeginMode2D(game.getCamera());
                game.draw();
                EndMode2D();
                ui.drawHUD(game);
                ui.drawErrorMessage(game.getErrorMessage());
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
