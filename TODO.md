- [x] game looks better with the shaders off. what i really wanted was shadows.
	  shadows from the palm trees on the ground / sand, and shadows from the
	  temple and other buildings + players.
	- [x] yeah thats as good as it is gonna get
- [x] there needs to be water on the jerusalem level that goes up to the portal
  / teleport / transport point, so that the ship is in the water and it makes
  logical sense.
	- [x] can the water extend out "infinitely" so it looks more like a shoreline?
	- [x] water move a little bit? shoreline go in and out a tad bit, sorta
		  like waves? maybe some foam or something? idk it's kinda lack luster
		  rn. maybe some beach?
- [x] swap out the cotton perhaps with a scroll or i think a mail enveloped, instead of this "cotton" block. 
	- [x] no i want a flat, floating and dancing letter sprite that you click on instead of a boring scroll block

- [ ] try and add in glb assets
	- [ ] snake first probably


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
- [x] Move interaction logic to `UpdateWorld` or a dedicated `Interact` function to keep `main.c` clean (Optional/Future).
