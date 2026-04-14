#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ----------------------------------------------------------------
// Window
// ----------------------------------------------------------------
#define WIDTH   1280
#define HEIGHT   720

// ----------------------------------------------------------------
// Object pool sizes
// ----------------------------------------------------------------
#define MAX_ROCKS  30
#define MAX_GOLD   30

// ----------------------------------------------------------------
// How far away Malta is
// At 20 units/sec:  -500 ~ 25s  |  -1500 ~ 75s  |  -3000 ~ 2.5min
// ----------------------------------------------------------------
#define FINISH_LINE_X  -2500.0f

// ----------------------------------------------------------------
// Horizontal (Z-axis) movement bounds for the boat
// ----------------------------------------------------------------
#define BOAT_Z_MIN  -130.0f
#define BOAT_Z_MAX   130.0f

// ----------------------------------------------------------------
// Malta island placement
// ----------------------------------------------------------------
#define ISLAND_OFFSET  120.0f

// ----------------------------------------------------------------
// Water grid settings
// ----------------------------------------------------------------
#define WATER_TILES_X    40
#define WATER_TILES_Z    40
#define WATER_TILE_SIZE  20.0f

// ----------------------------------------------------------------
// Crew / morale settings
// ----------------------------------------------------------------
#define MAX_CREW              5
#define CREW_MORALE_MAX     100.0f
#define CREW_MORALE_DRAIN     2.5f   // points per second per crew member
#define CREW_MORALE_BOOST    30.0f   // points restored by pressing E
#define CREW_INTERACT_DIST    2.5f   // how close Paul must be to interact
#define MORALE_GAMEOVER_VAL   0.0f

// ----------------------------------------------------------------
// Interior room dimensions (world units inside interior scene)
// ----------------------------------------------------------------
#define INTERIOR_ROOM_W   10.0f
#define INTERIOR_ROOM_H    3.5f
#define INTERIOR_ROOM_D   28.0f

// ----------------------------------------------------------------
// Mini-window dimensions (pixels)
// ----------------------------------------------------------------
#define MINI_W  320
#define MINI_H  220

#endif // GAME_H
