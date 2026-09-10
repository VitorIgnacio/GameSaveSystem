# Architecture

This document describes the architecture implemented in the current repository. It focuses on the persistence flow and the relationship between the gameplay and presentation components.

## Component Overview

### `GameState`

`include/GameState.hpp` is the persistence model. It contains:

- Player identity and progression
- Health and mana
- Coins and world position
- Current map and play time
- Save timestamp
- Collectible state

It also contains the XP threshold calculation and level-up state transition used by the demo.

### `SaveSystem`

`include/SaveSystem.hpp` and `src/SaveSystem.cpp` implement persistence. The class owns the save directory and exposes these operations:

- `save`
- `load`
- `exists`
- `remove`
- `listSaves`

The persistence layer depends on `GameState`, but it does not depend on raylib, `Game`, or `UI`.

### `Game`

`include/Game.hpp` and `src/Game.cpp` own the playable demo state. Responsibilities include:

- Player movement and world bounds
- Collectible spawning and animation
- Collision and collection
- XP and level progression
- Character scale calculation and growth animation
- Camera follow
- Save/load orchestration
- Floating feedback text

### `UI`

`include/UI.hpp` and `src/UI.cpp` own the interface. Responsibilities include:

- Button and menu structures
- Main, pause, save, load, settings, and error screens
- HUD rendering
- Save/load status feedback
- Responsive control hints

### `main.cpp`

`src/main.cpp` is the application entry point. It owns:

- raylib window initialization
- The single game loop
- Window resizing
- Menu state transitions
- Rendering order

The render loop is not recreated during load.

## Save Flow

1. The gameplay layer updates `Game::m_state`.
2. `Game::saveGame` synchronizes player geometry and snapshots every collectible into `GameState::collectibles`.
3. `SaveSystem::save` validates the slot number.
4. It copies the supplied state and replaces `saveTime` with the current time.
5. It writes `version=2` followed by scalar state fields.
6. It writes `collectibleCount` and one group of fields per collectible.
7. It writes the formatted local save timestamp.
8. The stream is checked before `SaveError::None` is returned.

The current format is:

```text
version=2
playerName=...
level=...
experience=...
experienceToNextLevel=...
health=...
maxHealth=...
mana=...
maxMana=...
coins=...
positionX=...
positionY=...
currentMap=...
playtime=...
collectibleCount=N
collectible0Type=...
collectible0Value=...
collectible0Active=...
collectible0PositionX=...
collectible0PositionY=...
collectible0BobOffset=...
collectible0BobSpeed=...
saveTime=...
```

Strings in `playerName` and `currentMap` are escaped before writing.

## Load Flow

1. `Game::loadGame` reads into a temporary `GameState`; it does not modify the active state first.
2. `SaveSystem::load` locates `slot_<number>.sav`.
3. It parses each `key=value` line into the temporary state.
4. It rejects unsupported format versions.
5. It validates level, XP threshold, health, mana, coins, and finite player coordinates.
6. It validates collectible types, values, animation values, and finite collectible coordinates.
7. Only after parsing and validation succeed is the temporary state assigned to the destination state.
8. `Game::applyLoadedState` replaces gameplay state, restores collectibles, recalculates character scale, synchronizes player geometry, and updates the camera.
9. The existing game loop continues rendering the updated state.

This separation prevents a failed or corrupt load from partially replacing the active game state.

## Versioning and Compatibility

The loader accepts format versions 1 and 2. The writer emits version 2.

Version 2 adds collectible persistence. When version 1 data is loaded without collectible records, the demo recreates its organized collectible set instead of treating the missing records as a complete collectible snapshot.

The implementation has format-version recognition but does not provide a general migration framework or forward compatibility.

## Error Handling

`SaveError` is the persistence API's typed result:

- `None`
- `SlotNotFound`
- `InvalidSlot`
- `IOError`
- `CorruptedData`
- `InvalidFormat`

Parsing exceptions are converted to `CorruptedData`. Invalid versions return `InvalidFormat`. Missing files return `SlotNotFound`, and invalid slot numbers return `InvalidSlot`.

The visual layer displays load failures in the error screen. Successful saves and loads produce transient in-game status messages.

## Runtime State and Derived Values

Character scale is not serialized. It is derived from the loaded level:

```text
targetScale = min(1.0 + (level - 1) * 0.10, 1.80)
```

After loading, `Game::applyLoadedState` immediately recalculates the current and target scale. Normal level changes converge toward the target through a frame-time interpolation. Level-up feedback adds a short visual pulse without changing the final derived scale.

## Rendering and Loop Ownership

`main.cpp` calls `BeginDrawing` and `EndDrawing` exactly once per frame. `Game::draw` only renders world content. The main loop wraps that content in one `Camera2D` transform, then renders the HUD and menus in screen space.

Load changes data owned by `Game`; it does not create another raylib window, frame loop, render mode, or input listener.

## Tests

`tests/test_runner.cpp` is a standalone C++ test runner. It exercises save creation, loading, optional loading, missing slots, deletion, multiple slots, invalid slots, corrupt data, string escaping, collectible round trips, invalid-state rejection, and empty directories.

The tests use temporary relative directories named `test_saves` and `test_saves_empty`, both ignored by Git.
