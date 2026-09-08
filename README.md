# Game Save System

A lightweight, engine-agnostic game save system library written in modern C++17.

## Objective

Create a portable, dependency-free save system that demonstrates professional C++ practices including RAII, proper error handling, serialization, and clean architecture.

## Features

- **Create/Load/Delete saves** with multiple slots (1-99)
- **List existing saves** with metadata (player name, level, timestamp)
- **Check if save exists** without loading
- **Human-readable text format** for save files
- **String escaping** for special characters in player names
- **Comprehensive error handling** via `enum class SaveError`
- **Cross-platform** using `std::filesystem`
- **No external dependencies** - pure C++17 standard library

## Architecture

```
GameSaveSystem/
├── include/
│   └── SaveSystem.hpp      # Public API
├── src/
│   ├── SaveSystem.cpp      # Implementation
│   └── main.cpp            # CLI demo
├── tests/
│   └── test_runner.cpp     # Unit tests
├── saves/                  # Default save directory
├── README.md
└── .gitignore
```

### Core Types

- **`GameState`** - Aggregates all player data (name, level, health, mana, XP, coins, position, playtime, save timestamp)
- **`SaveInfo`** - Metadata for listing saves (slot, player name, level, save time, file path)
- **`SaveError`** - Error enumeration: `None`, `SlotNotFound`, `InvalidSlot`, `IOError`, `CorruptedData`, `InvalidFormat`
- **`SaveSystem`** - Main class managing saves in a directory

## Requirements

- C++17 compatible compiler (g++/MinGW-w64 tested)
- No external libraries required

## Compilation

```bash
# Main application
g++ -std=c++17 -Wall -Wextra -Wpedantic src/*.cpp -Iinclude -o GameSave.exe

# Tests
g++ -std=c++17 -Wall -Wextra -Wpedantic src/SaveSystem.cpp tests/test_runner.cpp -Iinclude -o test_runner.exe
```

## Execution

```bash
# Run CLI demo
./GameSave.exe

# Run tests
./test_runner.exe
```

## Usage Example

```cpp
#include "SaveSystem.hpp"

using namespace GameSave;

SaveSystem saveSystem("saves");

GameState state;
state.playerName = "Hero";
state.level = 10;
state.health = 85;
state.mana = 50;
state.experience = 5000;
state.coins = 1500;
state.positionX = 100.5f;
state.positionY = 200.3f;
state.positionZ = 0.0f;
state.playtime = std::chrono::seconds(3600);

// Save to slot 1
auto error = saveSystem.save(1, state);
if (error == SaveError::None) {
    // Success
}

// Load from slot 1
GameState loadedState;
error = saveSystem.load(1, loadedState);
if (error == SaveError::None) {
    // Use loadedState
}

// Or use optional for cleaner code
if (auto opt = saveSystem.load(1)) {
    // opt.value() contains the state
}

// List all saves
auto saves = saveSystem.listSaves();
for (const auto& save : saves) {
    std::cout << "Slot " << save.slot << ": " << save.playerName << "\n";
}

// Check existence
if (saveSystem.exists(1)) { ... }

// Delete save
saveSystem.remove(1);
```

## Save File Format

Saves are stored as human-readable `.sav` files with key=value pairs:

```
version=1
playerName=Hero
level=10
health=85
mana=50
experience=5000
coins=1500
positionX=100.5
positionY=200.3
positionZ=0
playtime=3600
saveTime=2026-09-08 15:30:45
```

Special characters in strings are escaped: `\n`, `\r`, `\t`, `\\`, `\=`, `\;`

## Technical Decisions

| Decision | Rationale |
|----------|-----------|
| Text-based format | Human-readable, debuggable, no binary parsing issues |
| `enum class SaveError` | Type-safe, exhaustive error handling, no exceptions needed |
| `std::optional<GameState> load()` | Modern C++ pattern for nullable returns |
| `std::filesystem` | Cross-platform directory/file operations |
| RAII for file streams | Automatic resource management |
| No `using namespace std` | Explicit, avoids pollution |
| Separate header/impl | Clean separation, faster compilation |

## Current Limitations

- No binary format (planned)
- No checksum/integrity verification (planned)
- No compression (planned)
- No atomic writes (planned - currently writes directly to final file)
- No save versioning/migration (planned)
- Single-threaded only
- Maximum 99 slots (arbitrary limit)

## Roadmap

- **v0.2**: Binary format + checksum
- **v0.3**: Atomic saves (write to temp, rename)
- **v0.4**: Save versioning + migration
- **v0.5**: Compression (zlib)
- **v0.6**: Autosave support
- **v1.0**: Benchmarks, fuzz testing, full documentation

## License

MIT License - Feel free to use in your projects.