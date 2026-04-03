# ReEQ (ReJJ) — port reference for VocalChainer

## Sources in repo

- `reference/ReJJ/ReEQ/` — MIT-licensed JSFX from [Justin-Johnson/ReJJ](https://github.com/Justin-Johnson/ReJJ) (see `reference/ReJJ/README.md`).

## Honest scope

**“Port as-is”** in a VST means:

1. **DSP:** replicate filter types, topology (SVF from `svf_filter.jsfx-inc`, cascades, Butterworth options), stereo/M-S routing, optional FIR path (`firhalfband.jsfx-inc`) — in C++, real-time safe.
2. **Analysis:** spectrum pipeline from `spectrum.jsfx-inc` (FFT size, windows, log frequency, floor/ceiling) — align with `.cursor/skills/fft-spectrum-engineering/SKILL.md`.
3. **UI:** ReEQ’s interaction (nodes on curve, piano, peaks, pre/post EQ overlay) — new JUCE components; not a file copy.

## Current VocalChainer baseline

| Area | Now | ReEQ |
|------|-----|------|
| Bands | 4 peaking IIR | Multiple nodes, many filter types |
| Spectrum | Tap pre-EQ, `SpectrumPanel` | Full analyser + curve + options |
| UI | Rotary sliders | Graph-first |

## Suggested phases (for implementation tickets)

1. **Reference only** (done): vendor JSFX under `reference/ReJJ/` + this doc.
2. **Spectrum UX:** log axis, shared FFT policy, response curve overlay (math from ReEQ + JUCE `dsp::FFT`).
3. **DSP parity (incremental):** extend `ParametricEqNode` or new node: SVF peaking + shelves + LPF/HPF with slope; then M/S if needed.
4. **Graph UI:** draggable nodes; parameter sync with `ModuleParamsRuntime` / APVTS pattern.

## Attribution

When reusing filter math or structure derived from ReJJ, keep **MIT** obligations (copyright notice in third-party or docs).
