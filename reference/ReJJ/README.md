# ReJJ / ReEQ — reference sources (REAPER JSFX)

**Upstream:** [Justin-Johnson/ReJJ](https://github.com/Justin-Johnson/ReJJ)  
**ReaPack index:** `https://raw.githubusercontent.com/Justin-Johnson/ReJJ/master/index.xml`

## License

`ReEQ/ReEQ.jsfx` header states: **`license: MIT`** (author Justin Johnson). Keep this file and attribution if DSP/UI ideas are ported into Razumov VocalChainer.

## What was copied here

| Path | Role |
|------|------|
| `ReEQ/ReEQ.jsfx` | Main effect: sliders, `@sample` DSP, `@gfx` UI, node graph |
| `ReEQ/Dependencies/spectrum.jsfx-inc` | FFT spectrum: log freq axis, dB mapping, tilt |
| `ReEQ/Dependencies/svf_filter.jsfx-inc` | SVF filters (Cytomic SVF paper), cascaded slopes, Butterworth |
| `ReEQ/Dependencies/firhalfband.jsfx-inc` | Half-band FIR (HQ path) |

These files are **not compiled** by our CMake project. They are **design and math reference** for porting.

## Why not “drop in” to the VST

- **JSFX / EEL2** runs inside REAPER only. A VST3/AU plugin cannot execute `.jsfx` natively.
- A faithful “as-is” behavior requires **reimplementing** the same algorithms in **C++** (JUCE `dsp` or custom), and rebuilding the **interactive EQ UI** in JUCE (`Component`, paths, drag nodes).

## Relation to our code

- Current: `Source/dsp/graph/ParametricEqNode` — **4 peaking bands**, IIR peak, spectrum tap **before** EQ.
- ReEQ: **many nodes**, types (low/high cut, shelves, notch, tilt, …), **M/S & L/R**, SVF + FIR paths, rich spectrum + curve editor in `@gfx`.

See **`docs/REEQ_PORT_REFERENCE.md`** for a port checklist and mapping.
