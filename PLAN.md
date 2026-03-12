Plan to implement water, ports, and travel

1. Add water regions to the world state as axis-aligned rectangles in grid space.
2. Mark water tiles as blocked in IsTileBlocked to prevent walking on water.
3. Add static port positions along the shoreline with a sequential next-port index.
4. Render water regions and port markers in DrawWorld.
5. Detect when the player is on a port tile, show a "Press T to travel" prompt, and handle travel input.
6. Implement a travel stub that returns win/loss, then move the player to the same or next port based on the result.
7. Leave a clear hook to replace the stub with the colleague's game mode later.
