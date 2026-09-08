#include "SaveSystem.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

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

    file << "version=1\n";
    file << "playerName=" << escapeString(saveState.playerName) << "\n";
    file << "level=" << saveState.level << "\n";
    file << "health=" << saveState.health << "\n";
    file << "mana=" << saveState.mana << "\n";
    file << "experience=" << saveState.experience << "\n";
    file << "coins=" << saveState.coins << "\n";
    file << "positionX=" << saveState.positionX << "\n";
    file << "positionY=" << saveState.positionY << "\n";
    file << "positionZ=" << saveState.positionZ << "\n";
    file << "playtime=" << saveState.playtime.count() << "\n";
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
            } else if (key == "health") {
                loadedState.health = std::stoi(value);
            } else if (key == "mana") {
                loadedState.mana = std::stoi(value);
            } else if (key == "experience") {
                loadedState.experience = std::stoi(value);
            } else if (key == "coins") {
                loadedState.coins = std::stoi(value);
            } else if (key == "positionX") {
                loadedState.positionX = std::stof(value);
            } else if (key == "positionY") {
                loadedState.positionY = std::stof(value);
            } else if (key == "positionZ") {
                loadedState.positionZ = std::stof(value);
            } else if (key == "playtime") {
                loadedState.playtime = std::chrono::seconds(std::stoll(value));
            } else if (key == "saveTime") {
                loadedState.saveTime = parseTimePoint(value);
            }
        } catch (...) {
            return SaveError::CorruptedData;
        }
    }

    if (versionStr != "1") {
        return SaveError::InvalidFormat;
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