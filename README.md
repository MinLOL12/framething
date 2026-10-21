# FrameBack

A Geode mod for Geometry Dash 2.208. When the game is about to destroy a player after a fatal collision, FrameBack restores that player to the position, rotation, vertical velocity, and grounded state saved immediately before the current gameplay update, then skips the death call. It is enabled by default and can be switched off in the mod's Geode settings.

## Build

1. Install the Geode SDK for the Geometry Dash 2.208 target and the platform toolchain.
2. Set `GEODE_SDK` to the SDK directory.
3. Configure and build with CMake using the normal Geode mod workflow, for example:

   ```sh
   cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build build --config RelWithDebInfo
   ```

The generated mod package can then be installed with Geode's mod installer.

## Notes

This is a lightweight player-state rollback, not a full rewind of the entire game simulation. Triggers and other world state processed earlier in the same update are not undone. Depending on the collision and movement mode, rolling back can leave the player at the edge of an obstacle or repeat the attempted collision on a later update. Test it in practice mode first and disable it if a level behaves unexpectedly.
