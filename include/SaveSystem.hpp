#pragma once

#include "GameState.hpp"
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <chrono>
#include <cstdint>

namespace GameSave {

struct SaveInfo {
    int slot;
    std::string playerName;
    int level;
    std::chrono::system_clock::time_point saveTime;
    std::filesystem::path filePath;
};

enum class SaveError {
    None,
    SlotNotFound,
    InvalidSlot,
    IOError,
    CorruptedData,
    InvalidFormat
};

class SaveSystem {
public:
    explicit SaveSystem(const std::string& saveDirectory);

    SaveError save(int slot, const GameState& state);
    SaveError load(int slot, GameState& state) const;
    std::optional<GameState> load(int slot) const;

    bool exists(int slot) const;
    SaveError remove(int slot);
    std::vector<SaveInfo> listSaves() const;

    std::string getSaveDirectory() const;

private:
    std::filesystem::path m_saveDirectory;
    std::filesystem::path getSlotPath(int slot) const;
    static std::string formatTimePoint(const std::chrono::system_clock::time_point& tp);
    static std::chrono::system_clock::time_point parseTimePoint(const std::string& str);
    static std::string escapeString(const std::string& str);
    static std::string unescapeString(const std::string& str);
};

} // namespace GameSave