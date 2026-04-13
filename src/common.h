#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include "raymath.h"

// --- Configuration ---
#define INVENTORY_SIZE 28
#define MAX_GROUND_ITEMS 50
#define TILE_SIZE 1.0f
#define RENDER_DISTANCE 30.0f // How many tiles around the player to draw

// --- Types ---
typedef struct {
    int x, y, z;
} Vector3Int;

typedef enum {
    SKILL_ORATORY,
    SKILL_TENTMAKING,
    SKILL_ENDURANCE,
    SKILL_COUNT
} SkillType;

typedef struct {
    int level;
    int currentXp;
} Skill;

typedef struct {
    int id;
    const char *name;
    Color color;
} ItemData;

typedef struct {
    int itemId; // 0 = Empty
    int quantity;
} Slot;

typedef struct {
    Vector3Int position;
    int itemId;
    bool active;
} GroundItem;

typedef enum {
    QUEST_NONE,
    QUEST_NOT_STARTED,
    QUEST_ACTIVE,
    QUEST_COMPLETED
} QuestState;

typedef struct {
    int id;
    const char *title;
    const char *description;
    int requiredItemId;
    int targetChunkX;
    int targetChunkZ;
    int rewardXp;
} Quest;

static const Quest questDatabase[] = {
    {0, "None", "", 0, 0, 0, 0},
    {1, "The Antioch Scroll", "Deliver the Jerusalem Scroll to Barnabas in Antioch.", 5, 5, 15, 500},
    {2, "Tent for Damascus", "Bring a crafted Tent to Ananias in Damascus.", 6, 0, 10, 300},
    {3, "Rome Welcome", "Speak with all three Roman believers.", 0, 0, 0, 0},
    {4, "House Arrest", "Report to the Roman centurion.", 0, 0, 0, 0}
};

typedef enum {
    DECO_NONE,
    DECO_PALM_TREE,
    DECO_ROCK,
    DECO_HOUSE,
    DECO_SYNAGOGUE,
    DECO_TEMPLE,
    DECO_SHIP,
    DECO_COLUMN,
    DECO_FORUM_ARCH,
    DECO_FIRE_PIT,
    DECO_FIRE_PIT_UNLIT,
    DECO_SNAKE,
    DECO_MARKET,
    DECO_COUNT
} DecorationType;

typedef enum {
    SPRITE_PAUL,
    SPRITE_SADDUCEE,
    SPRITE_ANANIAS,
    SPRITE_LETTER,
    SPRITE_COUNT
} SpriteType;

extern Texture2D spriteDatabase[SPRITE_COUNT];
extern Model snakeModel;
extern bool snakeModelLoaded;
extern Vector3 snakeModelScale;
extern Vector3 snakeModelOffset;
extern Model columnModel;
extern bool columnModelLoaded;
extern Vector3 columnModelScale;
extern Vector3 columnModelOffset;
extern Model templeModel;
extern bool templeModelLoaded;
extern Vector3 templeModelScale;
extern Vector3 templeModelOffset;
extern Model desertHouseModel;
extern bool desertHouseModelLoaded;
extern Vector3 desertHouseModelScale;
extern Vector3 desertHouseModelOffset;
extern Model sadduceeModel;
extern bool sadduceeModelLoaded;
extern Vector3 sadduceeModelScale;
extern Vector3 sadduceeModelOffset;
extern Model boatModel;
extern bool boatModelLoaded;
extern Vector3 boatModelScale;
extern Vector3 boatModelOffset;
extern Model islanderModel;
extern bool islanderModelLoaded;
extern Vector3 islanderModelScale;
extern Vector3 islanderModelOffset;
extern Model paulModel;
extern bool paulModelLoaded;
extern Vector3 paulModelScale;
extern Vector3 paulModelOffset;
extern Model romanCharacterModel;
extern bool romanCharacterModelLoaded;
extern Vector3 romanCharacterModelScale;
extern Vector3 romanCharacterModelOffset;
extern Model romanSoldierModel;
extern bool romanSoldierModelLoaded;
extern Vector3 romanSoldierModelScale;
extern Vector3 romanSoldierModelOffset;

typedef struct {
    Vector3 lightDir;
    Color lightColor;
    float ambient;
    float shadowBias;
    bool showDebugUI;
} ShaderSettings;

// --- Global Constants ---
static const ItemData itemDatabase[] = {
    {0, "Empty", BLANK},
    {1, "Bronze Sword", DARKBLUE},
    {2, "Shrimp", ORANGE},
    {3, "Logs", BROWN},
    {4, "Tent Canvas", BEIGE},
    {5, "Scroll", WHITE},
    {6, "Tent", DARKGRAY}
};

#endif // COMMON_H
