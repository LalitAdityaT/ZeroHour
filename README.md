# ZeroHour

ZeroHour is a C++ multiplayer first-person shooter project built with Unreal Engine 5.4. The codebase includes networked combat and match systems, team modes, and a Steam-backed session workflow.

> **Source-only repository:** Unreal Engine content assets are intentionally not included. This repository does not contain the maps, character and weapon assets, Blueprints, or UI assets needed for a complete playable build. See [Content and licensing](#content-and-licensing) before opening the project.

## Features

- **Multiplayer sessions:** A reusable `MultiplayerSessions` plugin provides create, find, join, destroy, and start session operations, with a host/join menu widget.
- **Timed match flow:** Warmup, active match, and cooldown phases; player elimination, scoring, leader tracking, and respawning.
- **Team modes:** Team-based gameplay and a Capture the Flag game mode with flag and capture-zone classes.
- **Replicated combat:** Weapon equipping and swapping, aiming, reloading, ammo, grenades, and networked firing.
- **Weapon classes:** Assault rifles, rocket launchers, pistols, submachine guns, shotguns, sniper rifles, and grenade launchers.
- **Server-side rewind:** Lag compensation includes rewind paths for hitscan, projectile, and shotgun hit confirmation.
- **Player systems:** Health and shield pickups, speed and jump buffs, player state, HUD, and elimination announcements.

These are codebase systems; their related maps, assets, and Blueprint setup are not part of this source-only upload.

## Technology

- Unreal Engine **5.4**
- C++ with Unreal Engine modules including Enhanced Input, Niagara, UMG, and online subsystem modules
- Steam networking configured with SteamSockets and OnlineSubsystemSteam
- Windows project and Visual Studio solution

## Requirements

- Unreal Engine 5.4 with the project-required plugins available
- Visual Studio with the C++ game development workload and a Windows SDK
- The original project content assets, obtained separately and used under their applicable licenses, to restore the full project
- Steam client and appropriate Steam configuration to test Steam-based sessions

## Getting started

1. Clone the repository:

   ```sh
   git clone https://github.com/LalitAdityaT/ZeroHour.git
   ```

2. Install Unreal Engine 5.4 and the C++ build tools listed above.
3. Obtain the project's missing assets separately, making sure you have permission to use them. Restore them to the expected project locations, such as `Content/`.
4. Open `ZeroHour.uproject` in Unreal Editor. Allow Unreal to generate project files and compile the C++ modules when prompted.
5. Once the required assets are restored, open the project maps in the editor and configure any project-specific settings needed for your environment.

Without the missing assets, the editor may report missing maps or resources, and the project cannot be run as a complete game from this repository alone.

## Project layout

| Path | Description |
| --- | --- |
| `Source/ZeroHour/` | Main game module: characters, combat, weapons, game modes, HUD, pickups, player state, and lag compensation |
| `Plugins/MultiplayerSessions/Source/` | C++ online-session subsystem and host/join menu logic |
| `Plugins/MultiplayerSessions/MultiplayerSessions.uplugin` | Multiplayer Sessions plugin definition |
| `Config/` | Unreal project, input, rendering, and online subsystem configuration |
| `ZeroHour.uproject` | Unreal project descriptor; configured for Unreal Engine 5.4 |

## Multiplayer and Steam configuration

The project configures SteamSockets as its game net driver with an IP driver fallback, and enables the Steam online subsystem for sessions. The checked-in configuration uses Steam development App ID `480`; configure an App ID appropriate to your own project before preparing a release. Steam session testing also depends on the required Unreal plugins and a suitable local Steam setup.

The session plugin exposes create, search, join, destroy, and start operations through its subsystem. The menu and session workflow are implemented in C++, but any corresponding widget assets are excluded with the rest of the content.

## Content and licensing

The `Content/` directory and plugin content assets are excluded from this public repository. This includes maps, Blueprints, UI, and other Unreal assets, including third-party or marketplace-style packs. Generated files and packaged builds are excluded as well.

Restore only assets you have the right to use. Do not commit marketplace assets or other restricted content unless their license explicitly permits public redistribution. The `.gitignore` excludes Unreal content and generated output from Git.

The repository includes an MIT `LICENSE` for the project code. That license does not grant rights to any separately obtained Unreal Engine, marketplace, or third-party assets.

## Contributing

Changes to the source code and project configuration are welcome. Before opening a pull request, verify the affected Unreal C++ modules compile in Unreal Engine 5.4 with the required plugins and project assets available.
