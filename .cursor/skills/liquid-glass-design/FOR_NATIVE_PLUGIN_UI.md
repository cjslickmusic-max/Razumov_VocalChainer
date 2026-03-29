# Liquid Glass ideas for JUCE / native plugin UI

This repo is **not** SwiftUI. Use `SKILL.md` as a **visual direction** (depth, grouping, highlights, restraint).

## Practical mapping (message thread / paint only)

- **Blur / frosted panels**: `juce::Image` + `ImageConvolutionKernel::createGaussianBlur` or layered semi-transparent fills; avoid heavy blur on every frame if CPU is tight.
- **Glow / rim**: `juce::DropShadow`, `GlowEffect` on components, or multi-pass strokes (see `EditorVisualAssets`).
- **Material contrast**: keep dark base + one accent; avoid mixing unrelated glass styles in one panel (same idea as not mixing glass variants on iOS).

## Project rules

- `.cursor/rules/ui-strings-ascii.mdc`, `dsp-audio-safety.mdc` (no heavy work on audio thread).
