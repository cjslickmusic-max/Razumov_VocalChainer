# UI assets (SVG + PNG + glow)

## Embedded resources (BinaryData)

| File | Size (px) | Role |
|------|-----------|------|
| `Resources/ui/brushed_panel_placeholder.png` | **512 x 256** | Brushed metal placeholder; tiled in editor at ~15% opacity over base fill. |
| `Resources/ui/editor_corner_accent.svg` | **72 x 72** viewBox | Top-left accent (stroke + dot). |
| `Resources/ui/editor_corner_accent_r.svg` | **72 x 72** viewBox | Top-right mirror accent. |

Regenerate PNG (no third-party deps):

```bash
python3 Resources/ui/generate_placeholder_texture.py
```

## Code

- `Source/ui/EditorVisualAssets.h` / `EditorVisualAssets.cpp` — load PNG/SVG from `BinaryData`, draw background, corners, `drawRoundedRectGlow` for chain selection.
- `PluginEditor::paint` — base + tiled texture + corner SVGs.
- `ChainStripComponent::paint` — multi-pass glow under selected slot before cards.

## Future

- Replace `brushed_panel_placeholder.png` with a higher-res seam-tile PNG (powers of two, e.g. 1024x512) and adjust tile opacity.
- Optional SVG filters: JUCE `Drawable` does not render SVG `<filter>`; heavy glow stays in code or rasterized PNG.
- Mic profile: reserve a **square** region (e.g. 96x96 or 128x128) for a photo-style PNG when profiles ship.
