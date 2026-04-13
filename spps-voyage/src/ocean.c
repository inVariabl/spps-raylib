#include "ocean.h"
#include "game.h"

float WaveHeight(float x, float z, float t)
{
    return sinf(x * 0.08f + t * 2.0f) * 1.2f
         + sinf(z * 0.06f + t * 1.5f) * 0.8f
         + sinf((x + z) * 0.05f + t * 1.0f) * 0.5f;
}

void DrawOcean(float boatX, float t)
{
    float startX = boatX - (WATER_TILES_X / 2.0f) * WATER_TILE_SIZE;
    float startZ =        -(WATER_TILES_Z / 2.0f) * WATER_TILE_SIZE;

    // Base ocean color — all variation stays close to this
    // Crests go slightly lighter, troughs slightly darker
    const float BASE_R =  10.0f;
    const float BASE_G =  80.0f;
    const float BASE_B = 175.0f;

    // How much height influences color — keep this small to avoid patchiness
    const float COLOR_STRENGTH = 2.5f;

    for (int ix = 0; ix < WATER_TILES_X; ix++)
    {
        for (int iz = 0; iz < WATER_TILES_Z; iz++)
        {
            float x = startX + ix * WATER_TILE_SIZE;
            float z = startZ + iz * WATER_TILE_SIZE;

            Vector3 v0 = { x,                  WaveHeight(x,                  z,                  t), z                  };
            Vector3 v1 = { x + WATER_TILE_SIZE, WaveHeight(x + WATER_TILE_SIZE, z,                  t), z                  };
            Vector3 v2 = { x + WATER_TILE_SIZE, WaveHeight(x + WATER_TILE_SIZE, z + WATER_TILE_SIZE, t), z + WATER_TILE_SIZE };
            Vector3 v3 = { x,                  WaveHeight(x,                  z + WATER_TILE_SIZE, t), z + WATER_TILE_SIZE };

            // Use center height for color — averaged over all 4 corners
            float h = (v0.y + v1.y + v2.y + v3.y) * 0.25f;
            float shift = h * COLOR_STRENGTH;

            unsigned char r = (unsigned char)Clamp(BASE_R + shift * 0.3f,   0, 255);
            unsigned char g = (unsigned char)Clamp(BASE_G + shift * 0.8f,  55, 105);
            unsigned char b = (unsigned char)Clamp(BASE_B + shift * 1.0f, 145, 210);
            Color waterColor = { r, g, b, 255 };

            DrawTriangle3D(v0, v2, v1, waterColor);
            DrawTriangle3D(v0, v3, v2, waterColor);

            // Subtle specular highlight on wave crests only — no grid lines
            if (h > 1.2f)
            {
                unsigned char wr = (unsigned char)Clamp(r + 30, 0, 255);
                unsigned char wg = (unsigned char)Clamp(g + 25, 0, 255);
                unsigned char wb = (unsigned char)Clamp(b + 20, 0, 255);
                Color crestColor = { wr, wg, wb, 60 };
                DrawTriangle3D(v0, v2, v1, crestColor);
                DrawTriangle3D(v0, v3, v2, crestColor);
            }
        }
    }
}
