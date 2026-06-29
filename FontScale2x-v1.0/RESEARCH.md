# Font Scale 2x Research

## Existing Research Reference

- `research.lnk` resolves to `K:\kotor_dev\WidescreenPatch\research`.
- The GUI JSON research under `k1hrm-1.5` shows game controls reference named font textures through `FONT` fields such as `dialogfont16x16`, `fnt_console`, and `fnt_d16x16`.

## Address Database Reference

From `AddressDatabases/kotor1_0_3.db`:

- `CAurFont::TextOutA` at `0x004A1770`
- `CAurFontInfo::ParseField` at `0x00422210`
- `CAurFontInfo` offsets:
  - `0x04` `fontheight`
  - `0x08` `baselineheight`
  - `0x0C` `texturewidth`
  - `0x10` `spacingR`
  - `0x14` `spacingB`
  - `0x18` `upperleftcoords`
  - `0x24` `lowerrightcoords`

## Ghidra Findings

`CAurFontInfo::ParseField` loads TXI fields named `fontheight`, `baselineheight`, `texturewidth`, `spacingR`, `spacingB`, `upperleftcoords`, and `lowerrightcoords` into the offsets above.

`CAurFont::TextOutA` reads the font info pointer from `CAurFont + 0x18`. It uses:

- `fontheight` and `baselineheight` for glyph quad vertical extents.
- `texturewidth` and `spacingR` for glyph advance/width.
- `spacingB` for line/vertical spacing.
- `upperleftcoords` and `lowerrightcoords` as texture UV coordinate arrays.

The patch scales only the pixel metrics by `2.0f` and leaves UV arrays unchanged.

Ghidra xrefs to `0x004A1770` show calls from `0x0044CE0D` and recursive markup handling inside `TextOutA` itself at `0x004A1B27`, so one entry hook covers the shared renderer path instead of individual GUI controls.

After initial testing, the visible GUI font size did not change. The likely reason is that labels/buttons/listbox rows use `CAurGUIStringInternal`, not the `CAurFont::TextOutA` path above.

Additional findings:

- `CAurGUIStringInternal::Draw` is at `0x0045A850`.
- `CAurGUIStringInternal::GetFontPixelHeight` at `0x00459610` reads `CAurGUIStringInternal + 0x18`, calls the safe-pointer vtable method at `+0x38`, and then reads `CAurFontInfo + 0x04`.
- The second hook therefore scales the same `CAurFontInfo` metrics from the GUI string draw path.

## Hook Bytes

The local `swkotor.exe` SHA-256 is `9C10E0450A6EECA417E036E3CDE7474FED1F0A92AAB018446D156944DEA91435`.

Bytes at `0x004A1770`:

`6A FF 68 5C 7E 71 00 64 A1 00 00 00 00`
