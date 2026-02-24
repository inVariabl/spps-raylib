# RayScape - Paul's Journeys

An interactive RPG built with C and Raylib, following the journeys of St. Paul.

## Features

- **Dual Perspective System:**
  - **F1 (First-Person):** Modern FPS-style controls with smooth WASD movement, mouse-look rotation, and a crosshair for interaction.
  - **F3 (Isometric):** Classic point-and-click movement using BFS pathfinding, with optional WASD/Arrow key support.
- **World & Interaction:**
  - Dynamic tile-based world with robust obstacle detection (buildings, rocks, temples).
  - Interactive NPCs with dialogue trees and combat sessions.
  - Resource collection and crafting (e.g., crafting a tent from logs and canvas).
- **RPG Systems:**
  - **Quest System:** Support for multi-state quests (e.g., "The Antioch Scroll").
  - **Skills & XP:** Levelling system for various skills including Tentmaking and Oratory.
  - **Inventory Management:** Slot-based inventory system for managing items and materials.
- **Dynamic UI:**
  - Responsive HUD with health/spirit bars, skill lists, and inventory, all relative to screen size.
  - Contextual interaction for movement, talking to NPCs, and picking up items.

## Installation & Running

1. Ensure you have `raylib` and its dependencies installed on your system.
2. Run the build script:
   ```bash
   ./build
   ```
3. Alternatively, compile and run manually:
   ```bash
   ./rayscape
   ```

## Controls

- **F1:** Switch to First-Person mode.
- **F3:** Switch to Isometric (3rd Person) mode.
- **WASD / Arrow Keys:** Move player.
- **Mouse (F1):** Look around.
- **Left Click:** Interact with NPCs, items, or move (in F3 mode).
- **ESC:** Exit.
