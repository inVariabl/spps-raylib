#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

// --- Configuration ---
#define INVENTORY_SIZE 28
#define MAX_GROUND_ITEMS 10
#define TILE_SIZE 1.0f

// --- Structs ---
typedef struct {
  int id;
  const char *name;
  Color color; // Simple color to represent the item for now
} ItemData;

typedef struct {
  int itemId; // 0 = Empty
  int quantity;
} Slot;

typedef struct {
  Vector3 position;
  int itemId;
  bool active;
} GroundItem;

// --- Global Data ---
ItemData itemDatabase[] = {{0, "Empty", BLANK},
                           {1, "Bronze Sword", DARKBLUE},
                           {2, "Shrimp", ORANGE},
                           {3, "Logs", BROWN}};

// --- Helper Functions ---
bool AddToInventory(Slot inv[], int id) {
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (inv[i].itemId == 0) {
      inv[i].itemId = id;
      inv[i].quantity = 1;
      return true;
    }
  }
  return false;
}

int main() {
  // 1. Initialization
  const int screenWidth = 1280;
  const int screenHeight = 720;
  InitWindow(screenWidth, screenHeight, "St. Paul's Postal Service");

  // Camera setup
  Camera3D camera = {0};
  camera.position = (Vector3){8.0f, 8.0f, 8.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  // Player State
  Vector3 playerPos = {0.0f, 0.0f, 0.0f};
  Vector3 targetPos = playerPos;
  Slot inventory[INVENTORY_SIZE] = {0};

  // World State: Drop a few items on the ground
  GroundItem worldItems[MAX_GROUND_ITEMS] = {0};
  worldItems[0] = (GroundItem){(Vector3){3, 0, 2}, 1, true};  // Sword
  worldItems[1] = (GroundItem){(Vector3){-2, 0, 4}, 2, true}; // Shrimp
  worldItems[2] = (GroundItem){(Vector3){5, 0, -1}, 3, true}; // Logs

  SetTargetFPS(60);

  // --- Main Loop ---
  while (!WindowShouldClose()) {

    // 2. Logic: Mouse Interaction
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Ray ray = GetMouseRay(GetMousePosition(), camera);
      bool clickedItem = false;

      // Check if we clicked a ground item first
      for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (!worldItems[i].active)
          continue;

        BoundingBox box = {(Vector3){worldItems[i].position.x - 0.4f, 0,
                                     worldItems[i].position.z - 0.4f},
                           (Vector3){worldItems[i].position.x + 0.4f, 0.5f,
                                     worldItems[i].position.z + 0.4f}};

        if (GetRayCollisionBox(ray, box).hit) {
          // Try to pick up
          if (AddToInventory(inventory, worldItems[i].itemId)) {
            worldItems[i].active = false;
            clickedItem = true;
            break;
          }
        }
      }

      // If we didn't click an item, move the player
      if (!clickedItem) {
        RayCollision groundHit = GetRayCollisionQuad(
            ray, (Vector3){-50, 0, -50}, (Vector3){-50, 0, 50},
            (Vector3){50, 0, 50}, (Vector3){50, 0, -50});

        if (groundHit.hit) {
          targetPos.x = roundf(groundHit.point.x);
          targetPos.z = roundf(groundHit.point.z);
        }
      }
    }

    // Smooth Movement (Lerp)
    playerPos.x = Lerp(playerPos.x, targetPos.x, 0.15f);
    playerPos.z = Lerp(playerPos.z, targetPos.z, 0.15f);
    camera.target = playerPos;
    UpdateCamera(&camera, CAMERA_THIRD_PERSON);

    // 3. Rendering
    BeginDrawing();
    ClearBackground(SKYBLUE);

    BeginMode3D(camera);
    // Draw World
    DrawPlane((Vector3){0, -0.01f, 0}, (Vector2){100, 100}, DARKGREEN); // Grass
    DrawGrid(20, TILE_SIZE);

    // Draw Ground Items
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
      if (worldItems[i].active) {
        DrawCube(worldItems[i].position, 0.3f, 0.3f, 0.3f,
                 itemDatabase[worldItems[i].itemId].color);
      }
    }

    // Added an extra '8' for the rings parameter
    DrawCapsule((Vector3){playerPos.x, 0, playerPos.z},
                (Vector3){playerPos.x, 1.2f, playerPos.z}, 0.4f, 8, 8, BLUE);

    // Draw Destination Marker
    if (Vector3Distance(playerPos, targetPos) > 0.1f) {
      DrawCircle3D(targetPos, 0.5f, (Vector3){1, 0, 0}, 90.0f,
                   Fade(YELLOW, 0.5f));
    }
    EndMode3D();

    // --- 2.5D UI Layer ---
    int invStartX = screenWidth - 220;
    int invStartY = screenHeight - 350;

    // Inventory Panel
    DrawRectangle(invStartX, invStartY, 200, 330, Fade(BROWN, 0.9f));
    DrawRectangleLines(invStartX, invStartY, 200, 330, GOLD);
    DrawText("INVENTORY", invStartX + 50, invStartY + 10, 18, GOLD);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
      int slotX = invStartX + 10 + (i % 4) * 46;
      int slotY = invStartY + 40 + (i / 4) * 40;

      DrawRectangle(slotX, slotY, 40, 35, BLACK); // Slot background

      if (inventory[i].itemId != 0) {
        Color itemCol = itemDatabase[inventory[i].itemId].color;
        DrawRectangle(slotX + 5, slotY + 5, 30, 25, itemCol);
      }
    }

    DrawFPS(10, 10);
    DrawText("L-Click: Move or Pick Up Items", 10, 40, 20, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
