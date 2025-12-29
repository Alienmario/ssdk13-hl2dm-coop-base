# COOP SSDK13 Server Base for HL2DM

> [!WARNING]
>### This is unfinished and work in progress. You will need to know what your're doing and will need supporting addons. A guide will be added later.
>#### There is no guarantee of things working properly at this point. Contributions are welcome.


#### Based on
```
- Source SDK 2013 Community Edition
- hl2mp.ru coop mod
- SecobMod
```

This project aims to provide a standardized base for creating cooperative servers or maps within HL2DM.

It does so using modern build tools and working CI (thanks to SDK13-CE) and easy to use build scripts.

#### Necessary addons
- **Downloader** - to send required files to clients.
- **Per-map particle manifest creator** - to precache added particles on clients.
- **Entity lump editor** (Stripper:Source, SourceCoop, ..) for fixing up single-player maps or maps imported from standalone mods.
- **Other** - Teleports/checkpoints, regen, map management, equipment management, difficulty scaling, team restrict, scoring, death notice, model select, ...  
This is up to each server to implement! The goal of this project is to provide base fixed up binary to build on, not to seal in every player facing feature!  
Check out [SourceCoop](https://github.com/ampreeT/SourceCoop/) if you're looking for that - it even works on games without open source code.

## Building
#### Don't want to:
Just grab the latest build for your OS from the Actions tab.
#### Windows:
`./mp/src/build_server.bat`
#### Linux:
`./mp/src/build_server`

## ConVars

| ConVar | Description | Default |
|--------|-------------|---------|
| mp_noblock | Disables collisions of players | 1 |
| mp_noblock_entities | Disables entity (projectile) collisions of players on the same team | 1 |
| mp_noblock_use | Prevents other players from blocking \<use\> interactions | 1 |
| sv_use_hl2_models | Replaces all character models with the ones in hl2 subfolder. | 1 |
| sv_edict_overflow_mapchange | Change map when near edict limit | 1 |
| sv_init_default_relationships | Init default AI relationships? | 1 |
| sv_gunship_episodic | Whether gunship uses "fixed patrol logic" | 0 |
| sk_vortigaunt_zap_spread | Spread of vortigaunt's ranged attack | 0.05 |
| sk_vortigaunt_zap_lead_time | Lead aim this much into the future. Vortigaunt specific variation of ai_lead_time | 0.1 |

## Changes

- Footsteps are per surfaceprop and forced serverside (You should not touch sv_footsteps)
- Vehicles are smooth, including entry/exit animations without view snaps
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