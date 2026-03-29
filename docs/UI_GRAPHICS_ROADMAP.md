# Roadmap: UI / graphics (VocalChainer)

Roadmap for editor visuals and interaction. Order is approximate; adjust as product priorities change.

## Done (baseline)

| Item | Notes |
|------|--------|
| Signal path from graph tree | `ChainStripLayout` + `ChainStripComponent`: split, parallel branches side by side, merge, fork/join wires; scale-to-fit width |
| Slot selection | Click module or split card; merge is non-interactive |
| Contextual module panel | Existing per-kind parameter strip in viewport |
| Compact default layout | Smaller default window, macros, module knobs |

## Phase A — Polish current strip

| Priority | Task |
|----------|------|
| A1 | Hover state on cards; slightly stronger selected glow (match mockup direction) |
| A2 | Optional grid / subtle background in strip area only |
| A3 | Tooltip: full module name + bypass state on hover |
| A4 | Wire style: subtle curve or rounded corners for fork/join (reduce visual clutter) |

## Phase B — Density and navigation

| Priority | Task |
|----------|------|
| B1 | Horizontal scroll only for strip when graph grows (optional zoom) |
| B2 | Minimap or collapsed overview for very long chains |
| B3 | Keyboard: left/right to move selection along serial order |

## Phase C — Node graph (optional, long-term)

| Priority | Task |
|----------|------|
| C1 | Canvas node graph for advanced users (mockup lower half) — behind feature flag |
| C2 | Node library sidebar (categories, search) |
| C3 | Drag-and-drop wire editing with validation against `FlexGraphDesc` |

## Phase D — Meters and motion

| Priority | Task |
|----------|------|
| D1 | Per-compressor gain reduction meter in module panel when selected |
| D2 | Macro ring animation / value readout refinement |

## Conventions

- UI strings: ASCII-first (see `.cursor/rules/ui-strings-ascii.mdc`).
- Graph topology and latency rules remain defined in DSP/docs; UI must not imply routing that the engine does not perform.

## References

- Implementation: `Source/ui/ChainStripLayout.cpp`, `Source/ui/ChainStripComponent.cpp`
- Graph model: `Source/dsp/graph/FlexGraphDesc.h`
