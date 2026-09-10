# Game Save System

A C++17 game save system with a dependency-free core library and a playable 2D demonstration built with raylib. The project separates persistence from gameplay so the save system can be tested and reused independently, while the demo shows player progression, collectibles, camera follow, and restoring a complete saved session.

## Features

- Save, load, list, check, and delete save slots
- Multiple slots using `slot_<number>.sav` files
- Human-readable, text-based serialization
- Save format versioning with validation for versions 1 and 2
- String escaping for names and map data
- Atomic state replacement after successful deserialization
- Typed error reporting through `SaveError`
- Metadata listing with player name, level, slot, and timestamp
- Persistent gameplay state:
  - Player position, level, XP, HP, and mana
  - Coins and play time
  - Current map and save timestamp
  - Collectible position, type, value, animation state, and collection state
- Deterministic character scaling derived from level
- Smooth level-up growth animation
- Camera-following demo arena
- Quick Save and Quick Load
- Standard-library test runner for persistence behavior

## Demo

The repository does not include prebuilt executables. A ready-to-run Windows build is available in the GitHub Releases section. To build the visual demo from source, follow the instructions below and run the generated executable from the repository root.

The demo starts with a title screen. Select **New Game** to enter the arena.

### Controls

| Input | Action |
|---|---|
| `WASD` or arrow keys | Move |
| `H` | Reduce HP by 10 |
| `J` | Restore HP by 10 |
| `M` | Reduce mana by 10 |
| `N` | Restore mana by 10 |
| `Esc` | Open or return from the pause menu |
| `F5` | Quick Save |
| `F9` | Quick Load from the most recently loaded slot |

The pause menu provides five save slots, five load slots, settings for the quick-save slot, and a return to the main menu.

## Requirements

- A C++17-compatible compiler
- C++ standard library support for `<filesystem>`
- raylib 6.0 or a compatible raylib release for the visual demo
- On Windows with MinGW, the raylib import library and these system libraries:
  - `-lopengl32`
  - `-lgdi32`
  - `-lwinmm`

The core persistence library uses only the C++17 standard library. The visual demo requires raylib headers and libraries.

## Project Structure

```text
GameSaveSystem/
├── include/
│   ├── Game.hpp
│   ├── GameState.hpp
│   ├── SaveSystem.hpp
│   └── UI.hpp
├── src/
│   ├── Game.cpp
│   ├── main.cpp
│   ├── SaveSystem.cpp
│   └── UI.cpp
├── tests/
│   └── test_runner.cpp
├── assets/
├── saves/
├── .gitignore
├── README.md
└── docs/
    └── architecture.md
```

`assets/` is retained for future project assets; the current demo draws its scene procedurally and does not require external media files. `saves/` is created at runtime and local save files are ignored by Git.

## Architecture

The project has three main layers:

1. **Persistence layer**
   - `GameState` defines the serializable state.
   - `SaveSystem` owns slot paths and handles serialization, deserialization, validation, metadata, and errors.
2. **Gameplay layer**
   - `Game` owns player state, collectibles, input, collisions, progression, camera behavior, and save/load orchestration.
3. **Presentation layer**
   - `UI` owns menus, buttons, the HUD, and status feedback.
   - `main.cpp` owns the raylib window, frame loop, state machine, and rendering order.

The core save API does not depend on raylib or the gameplay classes. This keeps persistence independently testable.

## Build

The commands below use a MinGW-compatible `g++`. Adjust the raylib include and library directories to match the installation on your machine.

### Visual demo

From the repository root:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude -I"<RAYLIB_INCLUDE_DIR>" src/*.cpp -L"<RAYLIB_LIB_DIR>" -lraylib -lopengl32 -lgdi32 -lwinmm -o GameSaveDemo.exe
```

For example, replace the placeholders with the paths to raylib's `include` and `lib` directories. Do not commit machine-specific paths or generated executables.

### Persistence tests

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic src/SaveSystem.cpp tests/test_runner.cpp -Iinclude -o test_runner.exe
```

## Run

```powershell
.\GameSaveDemo.exe
```

The demo creates the `saves/` directory when it starts. Save files are stored relative to the executable's working directory.

## Testing

Run the test executable after building it:

```powershell
.\test_runner.exe
```

The test runner covers save creation, loading, optional loading, missing slots, deletion, multiple slots, invalid slots, corrupted files, string escaping, collectible state round trips, invalid-state rejection, and empty save directories. It creates and removes temporary `test_saves/` and `test_saves_empty/` directories.

## Save Format

Save files use UTF-8-compatible text lines in the form `key=value`. The current format is version 2 and includes:

- Format version
- Player and progression data
- Health, mana, coins, position, map, and play time
- Collectible count and per-collectible state
- Local save timestamp

Strings are escaped before serialization. The loader validates supported versions, numeric ranges, finite floating-point values, collectible types, and collectible metadata before replacing the destination state.

See [docs/architecture.md](docs/architecture.md) for the complete save/load flow and component responsibilities.

## Technical Highlights

- The core library is independent of the rendering and gameplay layers.
- `SaveSystem::load(int, GameState&)` parses into a temporary state and assigns it only after validation succeeds.
- Save format versions are explicit, allowing older files to remain readable while new fields are added.
- Character scale is derived from the current level rather than accumulated at each level-up.
- Saved collectibles preserve collection state, position, value, type, and animation parameters.
- The visual loop is owned by `main.cpp`; loading updates existing game state instead of recreating the render loop.

## Limitations

- The save format is text-based rather than binary.
- Save files do not currently include a checksum or cryptographic integrity field.
- Writes are not atomic; a failed write can leave a partial file.
- In-memory state replacement after a successful load is atomic.
- The demo is single-threaded.
- The test runner is a standalone executable rather than a framework-based suite.

## License

No license file is included in this repository. Add the intended license before publishing if the project should be reusable by others.
