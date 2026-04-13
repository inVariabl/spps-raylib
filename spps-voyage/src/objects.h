#ifndef OBJECTS_H
#define OBJECTS_H

#include "game.h"

typedef struct {
    Model       model;
    Vector3     position;
    BoundingBox bounds;
    bool        active;
} GameObject;

GameObject CreateGameObject(const char* modelPath, Vector3 pos);
void       UnloadGameObject(GameObject* obj);

#endif // OBJECTS_H
