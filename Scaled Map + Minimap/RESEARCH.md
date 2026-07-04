# Widescreen UI Scale Research Notes

Scope: first-pass audit of the Widescreen UI Scale hooks against local Ghidra
metadata and the dedicated `HudMinimap2xScale-v1.0` patch. This is not a final
implementation plan; it records expected behavior, current patch strategy, and
candidate improvements worth testing in-game.

## Baseline

`HudMinimap2xScale-v1.0` sets the stronger standard. It does not only resize a
control. It identifies the HUD minimap draw path, scales known minimap GUI
rectangles, updates the viewport basis, zooms the final map-image draw only
while the active viewport is the minimap viewport, and temporarily restores the
fog/grid basis while drawing that overlay.

That means Widescreen UI Scale should not treat "the rect got bigger" as
success. For every scaled UI island, verify:

- the visible control size and position;
- the render viewport size and crop;
- the texture/draw source dimensions;
- hit testing and mouse-over behavior;
- dynamic children created after GUI load;
- overlay rendering that divides by cached width/height fields.

## Hook Inventory

### `scaleGuiExtent` at `0x00409E4D`

Address DB: `CSWGuiExtent::Load` starts at `0x00409DC0`.

Expected behavior: after `LEFT`, `TOP`, `WIDTH`, and `HEIGHT` are read from an
`EXTENT` block, scale 4:3 menu layouts into the largest centered 4:3 area and
scale HUD layouts by height while allowing right-anchored HUD controls to move
to the real screen edge.

Current strategy:

- Hook the generic EXTENT loader.
- Detect root GUI loads by return address `0x0040A713`.
- Detect HUD roots by scanning nearby stack/context pointers for
  `maininterface` or `mi8x6`.
- Keep the HUD minimap texture rect `6,6,512,512` unscaled.
- Treat small menu-map marker rects inside the original map source area as a
  special case.

Risk:

- This hook is broad. Every GUI extent passes through it.
- HUD/menu mode is global state, so a missed or misclassified root can affect
  later child extents.
- Context text scanning is pragmatic but weaker than object/function-local
  evidence.
- Marker detection by rectangle shape can catch unrelated small controls if a
  future GUI has similar coordinates.

Better-patch candidates:

- Keep this as the generic menu/HUD scaler, but reduce special cases by moving
  known UI islands into narrower hooks where practical.
- For HUD minimap, port the exact-known-rect approach from
  `HudMinimap2xScale-v1.0` instead of relying on generic HUD scaling plus the
  `512x512` exemption.
- Investigate a hook around GUI resource/panel construction that exposes the
  resource name directly, replacing stack text scanning for HUD root detection.

### `prepareMenuMapScale` at `0x00693650`

Address DB: `CSWGuiInGameMap::OnPanelAdded`.

Expected behavior: when the map panel is shown, attach the current area map,
initialize map notes/markers, then ensure the map view and overlays match the
active integer scale.

Current strategy:

- Hook at panel-add entry.
- Patch constructor immediates for map backing texture and visible `LBL_Map`
  dimensions.
- Patch area-map coordinate bounds used by marker math.
- Patch map icon material size constants.
- Repair live map controls and dynamic marker controls after they exist.

Risk:

- It writes many code/data immediates at runtime.
- `patchAreaMapIconMaterialSizes` ignores `scale` and writes vanilla sizes,
  which may be intentional after material/marker control scaling but needs
  visual confirmation.
- Live repair writes directly into control extent fields rather than calling
  the control's `SetExtent` virtual method.

Better-patch candidates:

- Prefer calling the control `SetExtent` virtual method for live controls, as
  the minimap patch already does for the arrow button.
- Split the concerns: constructor/static dimension constants, coordinate-space
  bounds, and live marker repair should each have an in-game verification case.
- Confirm whether marker material constants should remain vanilla or scale with
  marker controls. If vanilla is correct, document why.

### Hit-test origin replacements at `0x0069330A` and `0x0069331C`

Address DB: `CSWGuiMapHider::HitCheckMouse` starts at `0x00693300`.

Expected behavior: map note mouse-over should compare mouse coordinates
against the scaled map viewport origin, while preserving the original
origin-relative hit-test logic for the map hider and its child marker controls.

Current strategy:

- Replace `mov edi,[esi+0x04]` and `mov edx,[esi+0x08]` with immediate loads.
- Rewrite those immediates from the patch DLL after computing the scaled
  viewport origin.

Risk:

- Self-modifying the replacement thunk depends on the runtime hook layout.
- If the map control's actual position diverges from the computed origin,
  hit-test behavior will be wrong.

Better-patch candidates:

- A detour around `CSWGuiMapHider::HitCheckMouse` could pass the map hider
  object and mouse coordinates to patch code, subtract the live scaled origin,
  and avoid rewriting immediates.
- If the immediate replacement stays, derive origin from the live map/control
  extent where possible rather than recomputing from `g_scale`.

### `prepareMenuMapDraw` at `0x006928D3`

Address DB: `CSWGuiInGameMap::Draw` starts at `0x00692810`.

Expected behavior: draw the area map inside the scaled visible map viewport,
crop to `440 * scale` width instead of leaking the full backing texture width,
and repair dynamic controls after refresh has populated them.

Current strategy:

- Hook immediately before width is loaded for the draw call.
- Reapply engine dimensions and live control repair.
- Write `440 * scale` into the stack width used by the original draw code.

Risk:

- The hook is partly a draw-time repair loop; if it hides an earlier lifecycle
  issue, it may do unnecessary repeated memory writes.
- The current centering constants are also patched globally from this patch,
  including map draw constants.

Better-patch candidates:

- Keep the crop hook unless a cleaner argument-level hook into the image draw
  call is found; this is a narrow and behavior-specific patch point.
- Consider moving repeated live-control repair to panel-add/update only after
  verifying dynamic markers are not created later than `OnPanelAdded`.

### `prepareHudMinimapMapControl` at `0x0068ABB0`

Address DB: `CSWGuiMainInterface::DrawMap` starts at `0x0068AB10`.

Expected behavior: scale the HUD minimap viewport/control while keeping map,
arrow, and fog/grid rendering centered and visually correct.

Current strategy:

- Reuses `menuMapIntegerScale`.
- Writes `hud+0x6088/0x608C` viewport basis to `120 * scale`.
- Writes `hud+0x6090/0x6094` map basis to `512 * scale`.
- Resizes the `hud+0x5E00` map label control to `512 * scale`.
- Writes stack rect width/height to `512 * scale`.

Risk:

- This is weaker than `HudMinimap2xScale-v1.0`.
- It scales the map control, but does not zoom the final map-image draw by
  active viewport, and does not handle fog/grid overlay basis restoration.
- It does not recenter or resize the minimap arrow via `SetExtent`.
- Reusing `menuMapIntegerScale` couples HUD minimap behavior to the area-map
  panel fit calculation, not to HUD minimap-specific design.
- It likely conflicts conceptually with `HudMinimapMapSizeFix-v1.0`, which
  forces logical `512x256` stack dimensions later in the same draw function.

Better-patch candidates:

- Port the dedicated minimap behavior into Widescreen UI Scale instead of this
  simplified map-control-only hook:
  - scale only known HUD minimap GUI extents;
  - set `0x6088/0x608C` viewport basis to the chosen viewport size;
  - use a viewport-active image draw hook for centered zoom;
  - bracket `CSWGuiMainInterface::UpdateAndDrawFogOfWar` with basis restore
    hooks;
  - update the arrow button through its `SetExtent` virtual method.
- Decide whether Widescreen UI Scale wants minimap viewport scale equal to the
  menu map integer scale or a fixed/derived HUD-specific scale. The dedicated
  patch proves fixed 2x; Widescreen UI Scale currently derives 1x/2x/etc from
  menu map fit.

## Suggested Next Order

1. Replace the Widescreen UI Scale HUD minimap logic with the proven
   `HudMinimap2xScale` behavior adapted to the patch's desired scale policy.
2. Audit menu-map hit testing with a detour prototype that uses live extents
   instead of immediate rewriting.
3. Convert live control extent writes to the virtual `SetExtent` path where the
   object layout is known.
4. Verify marker material constants visually and document whether they should
   stay vanilla or scale.
5. Only after the above, revisit the broad `CSWGuiExtent::Load` hook and remove
   special cases that have been moved to narrower hooks.

## Verification Matrix

Test at minimum:

- 640x480 and 800x600: patch should be a no-op or visually vanilla.
- 1280x720: common 16:9 with integer map scale likely 1x for menu map.
- 1920x1080: common 16:9, likely 2x menu-map/minimap behavior.
- 2560x1440 or 3840x2160: catches over-scaling and integer-scale thresholds.

For each resolution:

- main HUD root fills the screen and right-anchored HUD controls stay aligned;
- HUD minimap border, map, arrow, fog/grid overlay, and map texture are centered;
- area map `LBL_Map` crops correctly and does not show the unused atlas strip;
- map note markers render at expected size and location;
- map note mouse-over/click hit tests match visible marker positions;
- opening/closing the map repeatedly does not drift control positions or sizes.
