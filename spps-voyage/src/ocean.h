#ifndef OCEAN_H
#define OCEAN_H

// Returns wave height at a given world position and time.
float WaveHeight(float x, float z, float t);

// Draws the animated ocean grid centred on boatX.
void DrawOcean(float boatX, float t);

#endif // OCEAN_H
