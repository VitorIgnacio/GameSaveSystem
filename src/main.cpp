#include "SaveSystem.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

using namespace GameSave;

void printMenu() {
    std::cout << "\n========== GAME SAVE SYSTEM ==========\n";
    std::cout << "1. Create Save\n";
    std::cout << "2. Load Save\n";
    std::cout << "3. List Saves\n";
    std::cout << "4. Delete Save\n";
    std::cout << "5. Check Save Exists\n";
    std::cout << "6. Exit\n";
    std::cout << "======================================\n";
    std::cout << "Choose option: ";
}

void printSaveInfo(const SaveInfo& info) {
    auto timeT = std::chrono::system_clock::to_time_t(info.saveTime);
    std::cout << "  Slot " << info.slot
              << " | " << info.playerName
              << " | Level " << info.level
              << " | " << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S")
              << "\n";
}

void printGameState(const GameState& state) {
    auto timeT = std::chrono::system_clock::to_time_t(state.saveTime);
    std::cout << "\n--- Save Data ---\n";
    std::cout << "Player: " << state.playerName << "\n";
    std::cout << "Level: " << state.level << "\n";
    std::cout << "Health: " << state.health << "\n";
    std::cout << "Mana: " << state.mana << "\n";
    std::cout << "Experience: " << state.experience << "\n";
    std::cout << "Coins: " << state.coins << "\n";
    std::cout << "Position: (" << state.positionX << ", "
              << state.positionY << ", " << state.positionZ << ")\n";
    std::cout << "Playtime: " << state.playtime.count() << " seconds\n";
    std::cout << "Saved at: " << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S") << "\n";
}

int getSlotInput() {
    int slot;
    std::cout << "Enter slot number (1-99): ";
    while (!(std::cin >> slot) || slot <= 0 || slot > 99) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid slot. Enter a number between 1 and 99: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return slot;
}

std::string getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int getIntInput(const std::string& prompt, int minVal = 0, int maxVal = INT_MAX) {
    int value;
    std::cout << prompt;
    while (!(std::cin >> value) || value < minVal || value > maxVal) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. " << prompt;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

float getFloatInput(const std::string& prompt) {
    float value;
    std::cout << prompt;
    while (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. " << prompt;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void createSave(SaveSystem& saveSystem) {
    std::cout << "\n--- Create New Save ---\n";

    GameState state;
    state.playerName = getStringInput("Player name: ");
    if (state.playerName.empty()) {
        state.playerName = "Player";
    }

    state.level = getIntInput("Level: ", 1, 100);
    state.health = getIntInput("Health: ", 0, 9999);
    state.mana = getIntInput("Mana: ", 0, 9999);
    state.experience = getIntInput("Experience: ", 0, INT_MAX);
    state.coins = getIntInput("Coins: ", 0, INT_MAX);
    state.positionX = getFloatInput("Position X: ");
    state.positionY = getFloatInput("Position Y: ");
    state.positionZ = getFloatInput("Position Z: ");
    state.playtime = std::chrono::seconds(getIntInput("Playtime (seconds): ", 0, INT_MAX));

    int slot = getSlotInput();

    auto error = saveSystem.save(slot, state);
    if (error == SaveError::None) {
        std::cout << "\nSave created successfully in slot " << slot << "!\n";
    } else {
        std::cout << "\nError creating save: ";
        switch (error) {
            case SaveError::InvalidSlot: std::cout << "Invalid slot number"; break;
            case SaveError::IOError: std::cout << "I/O error"; break;
            default: std::cout << "Unknown error"; break;
        }
        std::cout << "\n";
    }
}

void loadSave(SaveSystem& saveSystem) {
    std::cout << "\n--- Load Save ---\n";
    int slot = getSlotInput();

    GameState state;
    auto error = saveSystem.load(slot, state);

    if (error == SaveError::None) {
        std::cout << "\nSave loaded successfully!\n";
        printGameState(state);
    } else {
        std::cout << "\nError loading save: ";
        switch (error) {
            case SaveError::SlotNotFound: std::cout << "Save not found in slot " << slot; break;
            case SaveError::InvalidSlot: std::cout << "Invalid slot number"; break;
            case SaveError::IOError: std::cout << "I/O error"; break;
            case SaveError::CorruptedData: std::cout << "Save data corrupted"; break;
            case SaveError::InvalidFormat: std::cout << "Invalid save format"; break;
            default: std::cout << "Unknown error"; break;
        }
        std::cout << "\n";
    }
}

void listSaves(const SaveSystem& saveSystem) {
    std::cout << "\n--- Saved Games ---\n";
    auto saves = saveSystem.listSaves();

    if (saves.empty()) {
        std::cout << "No saves found.\n";
    } else {
        for (const auto& save : saves) {
            printSaveInfo(save);
        }
    }
}

void deleteSave(SaveSystem& saveSystem) {
    std::cout << "\n--- Delete Save ---\n";
    int slot = getSlotInput();

    auto error = saveSystem.remove(slot);
    if (error == SaveError::None) {
        std::cout << "\nSave deleted successfully from slot " << slot << "!\n";
    } else {
        std::cout << "\nError deleting save: ";
        switch (error) {
            case SaveError::SlotNotFound: std::cout << "Save not found in slot " << slot; break;
            case SaveError::InvalidSlot: std::cout << "Invalid slot number"; break;
            case SaveError::IOError: std::cout << "I/O error"; break;
            default: std::cout << "Unknown error"; break;
        }
        std::cout << "\n";
    }
}

void checkSaveExists(const SaveSystem& saveSystem) {
    std::cout << "\n--- Check Save Exists ---\n";
    int slot = getSlotInput();

    if (saveSystem.exists(slot)) {
        std::cout << "Save exists in slot " << slot << ".\n";
    } else {
        std::cout << "No save found in slot " << slot << ".\n";
    }
}

int main() {
    SaveSystem saveSystem("saves");

    std::cout << "Welcome to Game Save System!\n";
    std::cout << "Save directory: " << saveSystem.getSaveDirectory() << "\n";

    int choice = 0;
    while (choice != 6) {
        printMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: createSave(saveSystem); break;
            case 2: loadSave(saveSystem); break;
            case 3: listSaves(saveSystem); break;
            case 4: deleteSave(saveSystem); break;
            case 5: checkSaveExists(saveSystem); break;
            case 6: std::cout << "\nGoodbye!\n"; break;
            default: std::cout << "\nInvalid option. Please try again.\n"; break;
        }
    }

    return 0;
}