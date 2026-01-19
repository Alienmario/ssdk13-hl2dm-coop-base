# COOP SSDK13 Server Base for HL2DM

> [!WARNING]
>### This is unfinished and work in progress. You will need to know what your're doing and will need supporting addons. There is absolutely no guarantee.
>#### A guide will be added later. Contributions are welcome.


#### Based on

- [Source SDK 2013](https://github.com/ValveSoftware/source-sdk-2013) (2025 edition)

With cherry-picked changes from

- [Source SDK 2013 Community Edition](https://github.com/Nbc66/source-sdk-2013-ce)
- [hl2mp.ru coop mod](https://github.com/hl2mp/mp_2012_Win32)
- [SecobMod](https://github.com/whoozzem/SecobMod)


This project aims to provide a standardized base for creating cooperative servers or maps compatible with HL2DM.

It does so using ready-to-use build scripts and working Github CI.

#### Recommended addons
- **Downloader** - to send required files to clients.
- **Per-map particle manifest creator** - to precache added particles on clients.
- **Entity lump editor** (Stripper:Source, SourceCoop, ..) for fixing up single-player maps or maps imported from standalone mods.
- **Other** - Teleports/checkpoints, regen, map management, equipment management, difficulty scaling, team restrict, scoring, death notice, model select, ...  
This is up to each server to implement! The goal of this project is to provide base fixed up binary to build on, not to seal in every player facing feature!  
Check out [SourceCoop](https://github.com/ampreeT/SourceCoop/) if you're looking for that - it even works on games without open source code.

## Building

### Windows prerequisites
- Visual Studio 2022
  - Desktop development with C++
    - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
    - Windows 11 SDK (10.0.22621.0) or Windows 10 SDK (10.0.19041.1)

### Linux prerequisites
`sudo apt-get update && sudo apt-get install podman`

### Build scripts

| Platform      | Instructions |
|---------------|--------------|
| Pre-built     | Grab the latest build from the Actions tab |
| Server - Windows 32    | `./src/build_server_win32.bat` |
| Server - Windows 64    | `./src/build_server_win64.bat` |
| Server - Linux 32      | `./src/build_server_linux32` |
| Server - Linux 64      | `./src/build_server_linux64` |

> You can pass `debug` or `release` (default) as the first parameter.

> (WIN) You can pass `noretry` as the second parameter to disable interactivity.

## ConVars

| ConVar | Description | Default |
|--------|-------------|---------|
| mp_noblock | Disables collisions of players | 1 |
| mp_noblock_entities | Disables entity (projectile) collisions of players on the same team | 1 |
| mp_noblock_use | Prevents other players from blocking \<use\> interactions | 1 |
| sv_use_hl2_models | Replaces all character models with the ones in hl2 subfolder. | 1 |
| sv_init_default_relationships | Init default AI relationships? | 1 |
| sv_gunship_episodic | Whether gunship uses "fixed patrol logic" | 0 |
| sk_vortigaunt_zap_spread | Spread of vortigaunt's ranged attack | 0.05 |
| sk_vortigaunt_zap_lead_time | Lead aim this much into the future. Vortigaunt specific variation of ai_lead_time | 0.1 |

## Changes

- Footsteps are per surfaceprop and forced serverside (You should not touch sv_footsteps)
- Smooth vehicles
- No-rubberband noblock for players and projectiles (optional)
- Fixed various effects (f.e. crossbow reload sparks)
- Reworked NPC death reporting to npc_maker, resolving stuck maps
- Best effort idle and noclip (gmod models) player animation defaults
- Fixed vortigaunt targeting
- Un-hid some convars
- Sped up SLAM
- Reduced temporary edict counts on map load
- Fixed Strider dealing damage through walls
- Reworked ChangeTeam function
- Crash reduction and expected functionality patches

#### Changes from D@Ni1986's hl2mp.ru coop mod
- Fixed NPCs with HL2 model replacements
- Fixed a ton of singleplayer oriented code
- Fixed NPC damage effects
- Added all HL2 NPCs up to episode 2
- Added HL1 NPCs
- Support for Obsidian Conflict and Synergy entities
- Added Portal turrets
- Weapons use skill values for damage and max ammo
- Support for CS:S ladders
- Fixed collision rules and AI relationships
- Removed some unnecessary messages