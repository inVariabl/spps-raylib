# Malta Implementation Plan

## 1. Environment & Setup
- [x] **NPCs**: Add "Islander" NPCs to Malta in `LoadWorld` (src/world.c).
- [x] **Snake Visibility**: Ensure the snake decoration is initially hidden or inactive until the fire is lit.
- [x] fire pit should not have the triangle flames when it's "not lit" but should have brown cylindrical logs angled like a teepee. when you click to light it it should changes to the fire animation

## 2. Fire Mechanic
- [x] **Interaction**: Implement interaction with `DECO_FIRE_PIT`.
- [x] **Snake Appearance**: When fire is lit, set snake decoration to active/visible and display message.

## 3. Snake Interaction
- [x] **Shake Off**: Interact with the snake (click) to shake it off.
  - Updates state and hides snake.
	- i don't think this is working. the snake is not attached to the player. i
		think just when the fire is lit, the snake appears (like it is rn) and then
		the dialogue should change. i don't think you should be able to shake off a
		snake.

## 4. NPC Dialogue & Reaction
- [x] **Initial State**: Islanders call Paul a murderer.
- [x] **Reaction State**: After shaking off snake, Islanders call Paul a god.

## 5. Progression
- [x] **Unlock Boat**: 
  - Boat is locked until the snake event is completed.
  - Captain/Message warns against sailing until winter passes (event complete).

## Refactoring
- [ ] Move interaction logic to `UpdateWorld` or a dedicated `Interact` function to keep `main.c` clean (Optional/Future).
