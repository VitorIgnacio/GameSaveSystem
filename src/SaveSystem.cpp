#include "SaveSystem.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <cctype>

namespace GameSave {

SaveSystem::SaveSystem(const std::string& saveDirectory)
    : m_saveDirectory(saveDirectory)
{
    std::filesystem::create_directories(m_saveDirectory);
}

std::filesystem::path SaveSystem::getSlotPath(int slot) const {
    return m_saveDirectory / ("slot_" + std::to_string(slot) + ".sav");
}

std::string SaveSystem::formatTimePoint(const std::chrono::system_clock::time_point& tp) {
    auto timeT = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point SaveSystem::parseTimePoint(const std::string& str) {
    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return std::chrono::system_clock::time_point{};
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string SaveSystem::escapeString(const std::string& str) {
    std::string result;
    result.reserve(str.size() * 2);
    for (char c : str) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\\': result += "\\\\"; break;
            case '=': result += "\\="; break;
            case ';': result += "\\;"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string SaveSystem::unescapeString(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            switch (str[i + 1]) {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '=': result += '='; break;
                case ';': result += ';'; break;
                default: result += str[i + 1]; break;
            }
            ++i;
        } else {
            result += str[i];
        }
    }
    return result;
}

SaveError SaveSystem::save(int slot, const GameState& state) {
    if (slot <= 0) {
        return SaveError::InvalidSlot;
    }

    auto path = getSlotPath(slot);
    std::ofstream file(path, std::ios::trunc);
    if (!file) {
        return SaveError::IOError;
    }

    GameState saveState = state;
    saveState.saveTime = std::chrono::system_clock::now();

    file << "version=2\n";
    file << "playerName=" << escapeString(saveState.playerName) << "\n";
    file << "level=" << saveState.level << "\n";
    file << "experience=" << saveState.experience << "\n";
    file << "experienceToNextLevel=" << saveState.experienceToNextLevel << "\n";
    file << "health=" << saveState.health << "\n";
    file << "maxHealth=" << saveState.maxHealth << "\n";
    file << "mana=" << saveState.mana << "\n";
    file << "maxMana=" << saveState.maxMana << "\n";
    file << "coins=" << saveState.coins << "\n";
    file << "positionX=" << saveState.positionX << "\n";
    file << "positionY=" << saveState.positionY << "\n";
    file << "currentMap=" << escapeString(saveState.currentMap) << "\n";
    file << "playtime=" << saveState.playtime.count() << "\n";
    file << "collectibleCount=" << saveState.collectibles.size() << "\n";
    for (size_t i = 0; i < saveState.collectibles.size(); ++i) {
        const CollectibleState& col = saveState.collectibles[i];
        file << "collectible" << i << "Type=" << col.type << "\n";
        file << "collectible" << i << "Value=" << col.value << "\n";
        file << "collectible" << i << "Active=" << (col.active ? 1 : 0) << "\n";
        file << "collectible" << i << "PositionX=" << col.positionX << "\n";
        file << "collectible" << i << "PositionY=" << col.positionY << "\n";
        file << "collectible" << i << "BobOffset=" << col.bobOffset << "\n";
        file << "collectible" << i << "BobSpeed=" << col.bobSpeed << "\n";
    }
    file << "saveTime=" << formatTimePoint(saveState.saveTime) << "\n";

    if (!file) {
        std::filesystem::remove(path);
        return SaveError::IOError;
    }

    return SaveError::None;
}

SaveError SaveSystem::load(int slot, GameState& state) const {
    auto path = getSlotPath(slot);
    if (!std::filesystem::exists(path)) {
        return SaveError::SlotNotFound;
    }

    std::ifstream file(path);
    if (!file) {
        return SaveError::IOError;
    }

    std::string line;
    std::string versionStr;
    int collectibleCount = -1;
    GameState loadedState;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        try {
            if (key == "version") {
                versionStr = value;
            } else if (key == "playerName") {
                loadedState.playerName = unescapeString(value);
            } else if (key == "level") {
                loadedState.level = std::stoi(value);
            } else if (key == "experience") {
                loadedState.experience = std::stoi(value);
            } else if (key == "experienceToNextLevel") {
                loadedState.experienceToNextLevel = std::stoi(value);
            } else if (key == "health") {
                loadedState.health = std::stoi(value);
            } else if (key == "maxHealth") {
                loadedState.maxHealth = std::stoi(value);
            } else if (key == "mana") {
                loadedState.mana = std::stoi(value);
            } else if (key == "maxMana") {
                loadedState.maxMana = std::stoi(value);
            } else if (key == "coins") {
                loadedState.coins = std::stoi(value);
            } else if (key == "positionX") {
                loadedState.positionX = std::stof(value);
            } else if (key == "positionY") {
                loadedState.positionY = std::stof(value);
            } else if (key == "currentMap") {
                loadedState.currentMap = unescapeString(value);
            } else if (key == "playtime") {
                loadedState.playtime = std::chrono::seconds(std::stoll(value));
            } else if (key == "collectibleCount") {
                collectibleCount = std::stoi(value);
                if (collectibleCount < 0 || collectibleCount > 1000) {
                    return SaveError::CorruptedData;
                }
                loadedState.collectibles.resize(static_cast<size_t>(collectibleCount));
                loadedState.collectibleDataAvailable = true;
            } else if (key.rfind("collectible", 0) == 0) {
                if (collectibleCount < 0) {
                    return SaveError::CorruptedData;
                }
                size_t separator = std::string::npos;
                for (size_t j = 11; j < key.size(); ++j) {
                    if (!std::isdigit(static_cast<unsigned char>(key[j]))) {
                        separator = j;
                        break;
                    }
                }
                if (separator == std::string::npos) {
                    return SaveError::CorruptedData;
                }
                int index = std::stoi(key.substr(11, separator - 11));
                if (index < 0 || index >= collectibleCount) {
                    return SaveError::CorruptedData;
                }
                std::string field = key.substr(separator);
                CollectibleState& col = loadedState.collectibles[static_cast<size_t>(index)];
                if (field == "Type") {
                    col.type = std::stoi(value);
                } else if (field == "Value") {
                    col.value = std::stoi(value);
                } else if (field == "Active") {
                    if (value == "1") {
                        col.active = true;
                    } else if (value == "0") {
                        col.active = false;
                    } else {
                        return SaveError::CorruptedData;
                    }
                } else if (field == "PositionX") {
                    col.positionX = std::stof(value);
                } else if (field == "PositionY") {
                    col.positionY = std::stof(value);
                } else if (field == "BobOffset") {
                    col.bobOffset = std::stof(value);
                } else if (field == "BobSpeed") {
                    col.bobSpeed = std::stof(value);
                } else {
                    return SaveError::CorruptedData;
                }
            } else if (key == "saveTime") {
                loadedState.saveTime = parseTimePoint(value);
            }
        } catch (...) {
            return SaveError::CorruptedData;
        }
    }

    if (versionStr.empty()) {
        return SaveError::InvalidFormat;
    }
    if (versionStr != "1" && versionStr != "2") {
        return SaveError::InvalidFormat;
    }

    if (versionStr == "2" && collectibleCount < 0) {
        return SaveError::CorruptedData;
    }

    if (loadedState.level < 1 || loadedState.experience < 0 || loadedState.experienceToNextLevel <= 0 ||
        loadedState.health < 0 || loadedState.maxHealth <= 0 || loadedState.mana < 0 || loadedState.maxMana <= 0 ||
        loadedState.coins < 0 || !std::isfinite(loadedState.positionX) || !std::isfinite(loadedState.positionY)) {
        return SaveError::CorruptedData;
    }

    for (const auto& col : loadedState.collectibles) {
        if (col.type < 0 || col.type > 3 || col.value <= 0 || col.bobSpeed < 0.0f ||
            !std::isfinite(col.positionX) || !std::isfinite(col.positionY) ||
            !std::isfinite(col.bobOffset) || !std::isfinite(col.bobSpeed)) {
            return SaveError::CorruptedData;
        }
    }

    state = loadedState;
    return SaveError::None;
}

std::optional<GameState> SaveSystem::load(int slot) const {
    GameState state;
    auto error = load(slot, state);
    if (error == SaveError::None) {
        return state;
    }
    return std::nullopt;
}

bool SaveSystem::exists(int slot) const {
    if (slot <= 0) return false;
    return std::filesystem::exists(getSlotPath(slot));
}

SaveError SaveSystem::remove(int slot) {
    if (slot <= 0) {
        return SaveError::InvalidSlot;
    }

    auto path = getSlotPath(slot);
    if (!std::filesystem::exists(path)) {
        return SaveError::SlotNotFound;
    }

    if (!std::filesystem::remove(path)) {
        return SaveError::IOError;
    }

    return SaveError::None;
}

std::vector<SaveInfo> SaveSystem::listSaves() const {
    std::vector<SaveInfo> saves;

    if (!std::filesystem::exists(m_saveDirectory)) {
        return saves;
    }

    for (const auto& entry : std::filesystem::directory_iterator(m_saveDirectory)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".sav") continue;

        std::string filename = entry.path().filename().string();
        if (filename.rfind("slot_", 0) != 0) continue;

        std::string slotStr = filename.substr(5, filename.size() - 9);
        int slot = 0;
        try {
            slot = std::stoi(slotStr);
        } catch (...) {
            continue;
        }

        GameState state;
        auto error = load(slot, state);
        if (error != SaveError::None) continue;

        SaveInfo info;
        info.slot = slot;
        info.playerName = state.playerName;
        info.level = state.level;
        info.saveTime = state.saveTime;
        info.filePath = entry.path();
        saves.push_back(info);
    }

    std::sort(saves.begin(), saves.end(),
              [](const SaveInfo& a, const SaveInfo& b) {
                  return a.slot < b.slot;
              });

    return saves;
}

std::string SaveSystem::getSaveDirectory() const {
    return m_saveDirectory.string();
}

} // namespace GameSave