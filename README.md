# Flarial Client Bootstrapper

> [!CAUTION]
> **Only supported on Windows x64!**

Bootstraps functionalities that cannot be implemented via the client's DLL at runtime.

## Features

- Properly fix the game's V-Sync Off implementation or "V-Sync Disabler".

- Ability to force DirectX 11 or "Better Frames".

## FAQ

#### How can I disable V-Sync?

- Open the following file:

    ```cmd
    %APPDATA%\Minecraft Bedrock\Users\*\games\com.mojang\minecraftpe\options.txt
    ```

- Set `gfx_vsync` to `0`, save the file & launch the game.

## Build
1. Install & update [MSYS2](https://www.msys2.org):

    ```bash
    pacman -Syu --noconfirm
    ```

3. Install [GCC](https://gcc.gnu.org) & [MinHook](https://github.com/TsudaKageyu/minhook):

    ```bash
    pacman -Syu mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-MinHook --noconfirm
    ```


3. Start MSYS2's `UCRT64` environment & run `Build.cmd`.
