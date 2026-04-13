#include "objects.h"

GameObject CreateGameObject(const char* modelPath, Vector3 pos)
{
    GameObject obj = { 0 };
    obj.model = LoadModel(modelPath);

    if (!IsModelValid(obj.model))
    {
        TraceLog(LOG_WARNING, "GAMEOBJECT: Failed to load model: %s", modelPath);
        obj.active = false;
        return obj;
    }

    obj.position = pos;
    obj.bounds   = GetModelBoundingBox(obj.model);
    obj.active   = true;
    return obj;
}

void UnloadGameObject(GameObject* obj)
{
    if (obj->active || IsModelValid(obj->model))
    {
        UnloadModel(obj->model);
        obj->active = false;
    }
}
