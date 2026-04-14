# Codex Handoff

## Branch / repo state

- Current branch: `integration/all-features`
- This is a partial selective-integration branch created from `main`.
- Nothing has been committed yet on this branch.
- Working tree status at handoff:
  - modified: `src/common.h`
  - modified: `src/player.c`
  - modified: `src/player.h`
  - modified: `src/ui.h`
  - modified: `src/world.h`
  - untracked: `src/npc.c`
  - untracked: `src/npc.h`
  - untracked: `src/ship_minigame.c`
  - untracked: `src/ship_minigame.h`

## User requirements being implemented

- Selective integration only. Do not merge branches wholesale.
- Use `luca` as the base for dialogue/UI behavior.
- Use the `GTA5` gameplay features that are already merged into `luca`:
  - NPC system
  - detection
  - rock throwing
- Port Shaun’s ship mini-game into the main executable as an in-process game state.
- Travel should only advance after successfully completing the mini-game.
- Failure should retry until the player succeeds.
- Prefer Shaun’s models for the ship mini-game, but do not pull in Shaun’s generated build output / clutter.

## Strategy I was following

1. Keep current `main` as the rendering/world baseline.
   Reason: current `main` has newer rendering, shaders, model loading, and world polish than `luca`.

2. Port Luca/GTA5 systems into current `main` instead of replacing current files wholesale.
   Main files to merge into:
   - `src/main.c`
   - `src/world.c`
   - `src/ui.c`
   - `src/player.c`
   - `src/player.h`
   - `src/world.h`
   - `src/common.h`

3. Add new source files for bounded systems:
   - `src/npc.c`
   - `src/npc.h`
   - `src/ship_minigame.c`
   - `src/ship_minigame.h`

4. After Luca/GTA5 systems compile, wire the ship mini-game into `HandlePortTravel()` flow in `src/main.c`.

## What has already been done

### 1. Shared state was expanded

`src/common.h`
- Added quest entries:
  - quest 3: `Rome Welcome`
  - quest 4: `House Arrest`

`src/player.h`
- Added Luca-derived fields:
  - `wantedStars`
  - `wantedDecayTimer`
  - `preachingNpcIndex`
  - `preachHoldTimer`
  - `preachSuccessTimer`
  - `worldMessage[160]`
  - `worldMessageTimer`
  - `guardDialogueActive`
  - `guardDialogueStep`
  - `guardDialogueLastResult`
  - `guardClearedForShip`
  - `romeBelieversMet[3]`
  - `romeCenturionMet`
- Added declarations:
  - `ResetPlayerMovement`
  - `DamagePlayerSpirit`

`src/world.h`
- Replaced the local bare NPC definition with `#include "npc.h"`
- Added projectile support:
  - `MAX_PROJECTILES`
  - `Projectile` struct
  - `projectiles[MAX_PROJECTILES]`
- Added Luca/GTA5 world state:
  - `playerSeenByPharisee`
  - `watchingPhariseeIndex`
  - `nearbyPreachNpcIndex`
  - `nearbyPreachNpcSeesPlayer`
- Added declaration for:
  - `IsPlayerSeenByPharisee`

`src/ui.h`
- Added declarations for:
  - `UpdateGuardDialogue`
  - `DrawGuardDialogue`

### 2. Player logic was partially merged

`src/player.c`
- Added `ResetPlayerMovement()`
- Added `DamagePlayerSpirit()`
- `InitPlayer()` now initializes the new Luca fields listed above.
- Existing movement / drawing / inventory logic from current `main` was left in place.

This file is in a reasonable intermediate state.

### 3. Luca/GTA5 NPC module was added

Added:
- `src/npc.h`
- `src/npc.c`

These are mostly based on Luca’s branch and include:
- patrol movement
- lerped NPC movement
- Pharisee pursuit logic when wanted level is active
- facing direction tracking
- NPC billboard drawing

This module has not yet been wired into `src/world.c`.

### 4. Shaun mini-game module was added as a first draft

Added:
- `src/ship_minigame.h`
- `src/ship_minigame.c`

Intent:
- An in-process voyage state
- 3-lane obstacle-dodging boat segment
- success advances travel
- failure retries with `R`, `Enter`, or `Space`
- loads boat / sailor GLBs from existing tracked assets:
  - `assets/roman_boat.glb`
  - `assets/roman_character.glb`

Important:
- This is only a draft.
- It is not wired into `src/main.c`.
- It likely does not compile as-is because it currently uses `DrawRing(...)` with a `Vector3`, which is wrong for raylib’s 2D `DrawRing`.
- Replace that with a valid 3D-friendly representation, e.g. `DrawSphere` + `DrawCircle3D`, or another simple effect.

## What is unfinished and still needs to be done

### 1. `src/world.c` is still untouched and is the main blocker

This is the next major task.

It still uses the old/simple NPC representation from current `main`.
It still has:
- empty `UpdateWorld()`
- no projectile updates
- no preach logic
- no pharisee detection state
- no `DrawNPC(...)` usage
- no `IsPlayerSeenByPharisee(...)`
- click detection still uses static NPC positions instead of `lerpPosition`

What needs to be ported into `src/world.c` from `origin/luca` selectively:
- `ClearWorldState()` additions:
  - clear projectiles
  - reset pharisee/preach state
- helper functions:
  - `IsSightLineBlocked`
  - `CanPhariseeSeePlayer`
  - `CanPreachTargetSeePlayer`
  - `SpawnRockProjectile`
  - `UpdateProjectiles`
  - `UpdatePreaching`
- `UpdateWorld()` implementation
- `IsPlayerSeenByPharisee()`
- `GetClickedNPC()` should use `npc.lerpPosition`
- projectile drawing in `DrawWorld()`
- use `DrawNPC(...)` for billboards unless a special model path is intended

For `LoadWorld()`:
- Keep current `main` world layout / bounds / rendering baseline.
- Do not import Luca’s extra intermediate worlds unless you intentionally decide to.
- Add Luca/GTA5 NPC content into the existing 3-world structure:
  - Judea:
    - keep current quest flow
    - add several Pharisees for detection / rock throwing
    - add Guard for Luca dialogue gate
    - optionally add civilian NPCs like Merchant / Elder / Pilgrim
  - Malta:
    - use `InitNPC()` for islanders
  - Puteoli:
    - add Roman believers + centurion from Luca

Important design note:
- I was planning to keep the existing `main` 3-world structure:
  - `WORLD_JUDEA`
  - `WORLD_MALTA`
  - `WORLD_PUTEOLI`
- The ship mini-game would sit between those transitions instead of adding Luca’s extra travel stop worlds.
- That seemed most aligned with the user request.

### 2. `src/ui.c` still needs a selective merge

Current state:
- `src/ui.h` was updated, but `src/ui.c` was not.

Need to selectively port from `origin/luca:src/ui.c`:
- `UpdateGuardDialogue()`
- `DrawGuardDialogue()`
- wanted stars UI
- “WATCHED: PHARISEE / CLEAR”
- preaching prompt / progress bar
- `worldMessage` banner
- Luca quest log behavior

But preserve current `main` UI improvements where they are better:
- current letter / item inventory icons
- shader debug UI already present in current `main`
- current HUD layout where possible

User explicitly said:
- defer to Luca for dialogue and UI

So if there is a conflict, Luca behavior should win unless it breaks current systems.

### 3. `src/main.c` still needs the core wiring

This file has not been edited yet.

It needs:
- include `ship_minigame.h`
- add a `ShipMinigame` instance and pending travel state
- add a helper like `ShowWorldMessage(...)`
- port Luca helper logic:
  - `CountRomeBelieversMet`
  - `HandleRomeNpcInteraction`
- integrate Luca guard dialogue flow:
  - call `UpdateGuardDialogue(&player, &world)`
  - gate mouse / travel / other actions while guard dialogue is active
- integrate GTA5/Luca interactions:
  - Sadducee / Pharisee / Ananias / Islander / Roman believers / Centurion handling
- keep current Malta fire pit + snake event logic
- replace direct port travel with:
  - start ship mini-game
  - when mini-game succeeds:
    - `LoadWorld(nextWorldId)`
    - move player to arrival port
  - when it fails:
    - stay in the mini-game retry loop
- for Judea departure:
  - preserve Luca’s `guardClearedForShip` gate before allowing voyage

Recommended shape:
- when `shipGame.active`:
  - pause overworld input / combat / normal drawing
  - update and draw only the mini-game
- otherwise:
  - run current main overworld loop

### 4. Compile / verification still needs to happen

I did not reach a compile/test step.

Potential known compile issues:
- `src/ship_minigame.c`: invalid `DrawRing(...)` usage with `Vector3`
- `src/world.c` is not yet updated to match `src/world.h` / `src/npc.h`
- `src/ui.c` is not yet updated to match `src/ui.h`
- `src/main.c` does not include or use the new mini-game / UI functions

## Recommended next steps for the next Codex instance

1. Fix `src/ship_minigame.c`
   - remove / replace invalid `DrawRing(...)`
   - keep it self-contained and simple

2. Update `src/world.c`
   - this is the biggest blocker
   - port Luca/GTA5 world update logic into current `main` world layout

3. Update `src/ui.c`
   - add guard dialogue + wanted / watched / preaching / world message UI

4. Update `src/main.c`
   - wire in:
     - guard dialogue
     - world messages
     - Rome NPC interactions
     - ship mini-game travel flow

5. Run build / fix compile errors

6. Only after it is compiling, do gameplay polish / conflict cleanup

## Useful branch references

These were the source branches I was using:
- `origin/luca`
- `origin/GTA5`
- `origin/shaun`

Important graph note:
- `origin/luca` already contains a merge of `origin/GTA5`
- so treat `luca` as the source of Luca + GTA5 gameplay logic

## Notes on assets

- I intentionally did not try to import Shaun’s raw `models/` directory from his branch.
- This repo already contains tracked GLBs in `assets/` that appear sufficient for the mini-game:
  - `assets/roman_boat.glb`
  - `assets/roman_character.glb`
- Using these seemed safer than dragging in Shaun’s branch artifacts.

## Final status at handoff

- Partial header / module groundwork is in place.
- The branch is not in a buildable state yet.
- The next Codex should resume by editing `src/world.c`, `src/ui.c`, and `src/main.c`, then compiling and fixing errors.
