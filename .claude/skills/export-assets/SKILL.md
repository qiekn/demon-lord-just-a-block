---
name: export-assets
description: Use when working with assets from `./export/` — the Unity asset dump for the original "卡牌魔王，只剩个头". Activates when the user mentions "export", "原版资源", "TextAsset", "Sprite", "Texture2D", "MonoBehaviour", "Room layout", "原版配置", or asks to port/find/convert game data (skills, souls, rooms, enemies, dialogue, achievements). Tells you which files are worth reading, which to skip, and the CSV conventions used throughout.
---

# `export/` asset dump — reading guide

`export/` is a local-only (gitignored) UnityRipper-style dump of the original Steam build. Use it as the source of truth for game data when implementing systems; copy/convert into `assets/` when the binary needs it.

## TL;DR — read order when porting a system

| If you need… | Open these (in order) | Skip |
|---|---|---|
| Room/map layout | `TextAsset/RoomConfigs.txt` → individual `Room_<id>.txt` | MonoBehaviour |
| Ability/card list | `TextAsset/SoulConfigs.txt` + `TextAsset/SkillConfigs.txt` | — |
| Enemy types | `TextAsset/UnitDescribeConfigs.txt` (book) + `TextAsset/EnemyConfigs.txt` (spawns) | — |
| Enemy AI | `TextAsset/StateConfigs.txt` | — |
| Dialogue | `TextAsset/DialogueShortConfig.txt` (short barks) → `TextAsset/DialogueConfigs.txt` (story; 121 KB, sample only) | LanguageConfig_Dialogue.txt unless localizing |
| Achievements | `TextAsset/AchievementConfigs.txt` | — |
| Difficulty modifiers | `TextAsset/DifficultConfigs.txt` | — |
| Sprite art | `Sprite/<name>.png` | `Texture2D/` (mostly a duplicate of Sprite/) |
| i18n | `TextAsset/LanguageConfig_*.txt` (huge — grep, don't read whole) | — |

## What's in each top-level dir

```
export/
├── TextAsset/      164 .txt    ← gameplay configs. The gold mine.
├── Sprite/         2330 .png   ← UI + character + tile art
├── Texture2D/      2140 .png   ← ~95% the same files as Sprite/ (Unity exports both)
├── MonoBehaviour/  3429 .json  ← almost worthless: 168–198 B each, script ref stubs only, no data
├── AudioClip/      155 .wav    ← SFX + music
├── Animator/       0 files     ← directory exists but empty
├── Mesh/           13 .obj     ← Unity primitives, not game art
├── Font/           6 .ttf      ← original-game fonts (we use NotoSansSC instead)
└── VideoClip/      2 .mp4      ← doggeTeach.mp4 / parryTeach.mp4 tutorials
```

**MonoBehaviour/ is a trap.** Each file is ~170 B and looks like `{m_GameObject, m_Enabled, m_Script:{m_FileID, m_PathID}, m_Name}` — just a script-asset reference, no serialized field data. Don't crawl it. Game data lives in `TextAsset/`.

**Sprite/ vs Texture2D/.** Unity exports the same image twice when a Texture2D backs a Sprite. Prefer `Sprite/` (the artist-named version) over `Texture2D/` to avoid duplicates. Filenames are descriptive: `abilityPannel.png`, `ach000.png` (achievement icon for id 0), etc.

## TextAsset CSV conventions

All `*Configs.txt` files are CSV with a header row. Watch for:

- **Header row first.** Inspect line 1 to see column names; the rest of the work flows from there.
- **Pipe `|` = array.** Multi-level values use `|` as inner separator, e.g. `10|15|20|25|30` in `SoulConfigs.cost` means levels 1..5 cost 10/15/20/25/30.
- **Empty fields are common** — multiple consecutive commas mean "no value". Don't treat them as errors.
- **Empty rows are section separators.** `EnemyConfigs.txt` and `SoulConfigs.txt` group rows by chapter using blank rows. Preserve that when parsing.
- **Trailing commas pad to header width.** Some columns at the right edge are unused (often `,,,,,` at line end).
- **Quoted commas inside fields.** Localization files quote `"foo, bar"` properly — use a CSV library, not naïve `split(',')`.
- **Booleans are `TRUE`/`FALSE` uppercase, or 1/0.** Both appear.
- **CJK + asterisk markup.** `*foo*` in dialogue/UI strings marks emphasis (the game highlights it).

### Room file format

`Room_<id>.txt` is a 2D tile grid: each line is one row, each comma-separated cell is a tile ID. Row width can vary per file — read from `RoomConfigs.txt` (`layoutPath` column) to find which file backs a roomID, plus `playerX,playerY` spawn. Empty cell = empty tile. Tile ID semantics (walls vs floor vs door) need to be back-derived from the game; `801`/`99` look like walls/void in samples, `999` looks like a door.

### LanguageConfig

`LanguageConfig*.txt` columns: `id, zh, en, CNTraditional, Japan, Korean, ru, es, ptbr, de, fr`. ID prefixes hint at scope: `dlg_*` = dialogue, etc. These files are 75–475 KB; never read whole — grep for an ID first.

## Safe exploration recipes

```powershell
# List TextAssets by size (smallest first) — start small
Get-ChildItem .\export\TextAsset -File | Sort-Object Length | Select Name,Length

# Show only the header row of a config to learn its schema
Get-Content .\export\TextAsset\SkillConfigs.txt -TotalCount 1

# Find a specific enemy ID across all configs
Select-String -Path .\export\TextAsset\*.txt -Pattern '\b104\b'
```

For Claude tools: use `Read` with `limit: 10–20` on big CSVs (Language*, Skill, Dialogue) to grab the header + a handful of rows. Don't read full 475 KB Language files into context.

## When porting an asset into the project

1. Keep `export/` read-only.
2. Copy the specific file(s) you need into `assets/` (committed) — converting format if needed (e.g. CSV → a runtime-friendly format).
3. Reference from code via `CK_ASSET("rel/path")` from `src/assets.hpp`.
4. Don't bulk-copy whole subdirs of `export/` into the repo.
