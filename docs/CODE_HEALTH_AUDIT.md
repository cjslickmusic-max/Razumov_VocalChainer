# Code Health Audit

Журнал **Quick check** (после фич) и **Full pass** (релиз / по запросу). Новые записи добавлять сверху, не удалять историю без необходимости.

---

## Quick check — v0.9.4 (2026-03-28) — Canonical VST3 in Artifacts/ via POST_BUILD

- `CMakeLists.txt`: `POST_BUILD` на `RazumovVocalChain_VST3` -> `Artifacts/Razumov Vocal Chain.vst3`; `README.md`, `Artifacts/README.md`, `agent-build-after-changes.mdc` (сборка `RazumovVocalChain_VST3`).
- Версия **0.9.4**; Release + `RazumovVocalChain_VST3`: OK.

---

## Quick check — v0.9.3 (2026-03-28) — Chain strip layout + UI compact

- `Source/ui/ChainStripLayout.{h,cpp}`: раскладка по `FlexSegmentDesc` (split, ветки рядом, merge, провода fork/join, scale-to-fit).
- `ChainStripComponent`: отрисовка и hit-test по layout; `PluginEditor`: компактнее окно/макросы/панель модуля; версия **0.9.3**.
- `docs/UI_GRAPHICS_ROADMAP.md`: фазы A–D по графике UI.
- ReadLints (изменённые исходники); Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.2 (2026-03-28) — Phase G branch phase align + long regression script

- `FlexSlotDesc` / `FlexSlot`: `branchPhaseAlignSamples`; `FlexGraphSerialization`: свойство `phaseAlign` на `Branch`; `slotLatencySamples` / `GraphEngine::processSplit`: phase pad + PDC pad; пул `phaseAlignPads_`.
- `GraphPlanFactory::makeParallelHalvesPhaseAlignDesc`; `Tests/PhaseAlignTests.cpp`; stress включает phase-align сценарий; `scripts/regression_loop_until_duration.sh`.
- Версия **0.9.2**; ReadLints; Release + оба тестовых таргета: OK.

---

## Quick check — v0.9.1 (2026-03-28) — FlexGraph serialization tests + stress binary

- `Tests/FlexGraphSerializationTests.cpp`: `ValueTree` round-trip (`isEquivalentTo`), пустой сегмент, `assignUniqueSlotIds` при `slotId == 0`.
- `Tests/GraphStressTests.cpp`, `RazumovVocalChainStressTests`: сотни тысяч/миллионы сэмплов (merge 2/3-way, nested, halves, thirds), два `GraphEngine` побитово, чередование `submitPlan`, `MergeDelayPad`, `SpectralCompressorNode`, полная phase3 цепочка; проверки `finite` и пик.
- CMake: `RAZUMOV_GRAPH_DSP_TEST_SOURCES`, `ctest` `GraphStress` (LABELS `long`/`stress`, TIMEOUT 600). `docs/TESTING.md`, `AGENTS.md`.
- ReadLints; Release: `RazumovVocalChainTests` + `RazumovVocalChainStressTests` + `RazumovVocalChain`: OK.

---

## Quick check — v0.9.1 (2026-03-28) — MergePdcTests + Spectral determinism

- `GraphPlanFactory`: `makeParallelThreeWayMismatchedLatencyDescForTests`, `makeNestedParallelMismatchedLatencyDescForTests`.
- `Tests/MergePdcTests.cpp`: 2-way/3-way синус после merge = чистая задержка 64; 3-way импульс и multi-block; вложенный split (пик на 32, DC).
- `DspDeterminismTests`: `SpectralCompressorNode` детерминизм после `reset()`.
- ReadLints (изменённые тесты/фабрика); Release + `RazumovVocalChainTests` + таргет `RazumovVocalChain`: OK.

---

## Quick check — v0.9.1 (2026-03-28) — DSP determinism test suite + TESTING.md

- `Tests/TestMain.cpp`, `DspDeterminismTests.cpp`, `DspTestHelpers.h`; `GraphTests.cpp` -> `runGraphEngineTests()`. CMake: include `Tests/`, `ctest` на тот же бинарник.
- Детерминизм: компрессоры Opto/FET/VCA, Gain, Filter, Deesser, Exciter, Mic, MergeDelayPad, Latency; синус-сдвиг на задержке; DC через Gain; тишина на входе компрессоров.
- `docs/TESTING.md`, `HANDOFF_NEXT_AGENT.md` секция 0; `ARCHITECTURE.md` (тесты).
- Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.1 (2026-03-28) — code critique + GraphEngine split scratch

- Аудит: `docs/CODE_REVIEW_CRITIQUE.md`; журнал `REFACTOR_SUSPICIOUS_LOG.md` (mutex, spectral scratch, fillSlot, walk apply).
- `processSplit`: убрана аллокация `std::vector<int>` на блок — `latPerBranchScratch_` в `ensureBranchPool`.
- ReadLints; Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.1 (2026-03-28) — merge PDC tests + nested split pads

- `MergeDelayPad::setDelaySamples` не сбрасывает линию при том же `delay` (PDC между аудиоблоками). `GraphEngine`: раздельные merge pads по `splitDepth` и индексу ветки; вложенные split не конфликтуют с внешним уровнем. `FlexGraphPlan::computeMaxSplitNestingDepth()`.
- Фабрика: `makeParallelThirdsDesc`, `makeNestedParallelHalvesDesc`. Расширены `Tests/GraphTests.cpp`.
- ReadLints на изменённых файлах; Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.1 (2026-03-28) — macros x8 + parallel strip rows

- APVTS: `macroPunch` / `macroBody` / `macroSmooth` / `macroDensity`; `MacroAudioState` + `applyMacroOffsetsToPhase3`; UI: две строки по четыре макро-ротора; `ChainStripItem::row` + полоса 100px (ветки на нижнем ряду).
- ReadLints (изменённые файлы); сборка Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.0 (2026-03-28) — per-slot module params

- `ModuleParamsRuntime` (atomics per `slotId`, `ModuleParams` in state XML); `GraphEngine::process(buf, macros, moduleParams)`; APVTS: chain / mic profile / macros only; legacy PARAM migration; UI sliders bound to `selectedSlotId_`.
- ReadLints (изменённые файлы); сборка Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.8.3 (2026-03-27) — FlexGraph persist + chain toolbar + slot selection

- `FlexGraphSerialization`, `persistFlexGraphToApvtsState` / sync / ensure после load; `ChainStripComponent` selection + hit test; `PluginEditor` toolbar, Add (modules + Split x2/x3), global vs module viewport; удалён лишний `ignoreUnused` в `refreshModulePanelVisibility`.
- Сборка Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.8.2 (2026-03-27) — Flex graph (serial + N-way parallel, PDC)

- `FlexGraphDesc` / `FlexGraphPlan`, `AudioNodeFactory`, `GraphEngine` (merge pool + `MergeDelayPad` per branch); `GraphPlanFactory` через `FlexSegmentDesc`; Phase3 после swap в `process(buf, params)`; `micProfile` APVTS + UI; полоса цепочки из `graphDesc_`.
- ReadLints (изменённые исходники); сборка Release + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.8.0 (2026-03-25) — Chain strip UI (ShaperX-style)

- `ChainStripComponent`, `chainProfile` listener; сборка: OK.

---

## Quick check — v0.7.1 (2026-03-25) — Roadmap 3.1 slot + spectral plan note

- Таблица ROADMAP, комментарии узлов mic/spectral; сборка: OK.

---

## Quick check — v0.7.0 (2026-03-25) — Macros (4.3) + perf / Windows docs (5.2–5.3)

- Макросы в APVTS + DSP; `docs/PERF_CHECKPOINTS.md`, `docs/BUILD_WINDOWS.md`; сборка + тесты: OK.

---

## Quick check — v0.6.0 (2026-03-25) — Startup chains (phase 4.2)

- `chainProfile` + `makeCompactVocalChainPhase3` / `makeFetForwardVocalChainPhase3` / `makeStartupChainForIndex`; APVTS listener; UI ComboBox «Chain»; сборка + тесты: OK.

---

## Quick check — v0.5.0 (2026-03-25) — Factory presets (phase 4.1)

- `presets/FactoryPresets`, `applyFactoryPreset`, `getNumPrograms` / UI ComboBox; сборка + тесты: OK.

---

## Quick check — v0.4.0 (2026-03-25) — Phase 3 chain + UI

- Параметры: расширенные `ParamIDs` / `ParameterLayout`; `Phase3RealtimeParams` → `GraphEngine::applyPhase3Parameters`; дефолтный план `makeDefaultVocalChainPhase3`.
- Сборка + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.3.0 (2026-03-25) — APVTS + Phase 2

- Параметры: `params/ParamIDs`, `ParameterLayout`, APVTS, state XML; UI роторы; (исторически) привязка gain/LP через `GraphEngine`.
- Сборка + ctest: OK.

---

## Quick check — v0.2.0 (2026-03-25) — Graph engine + tests

- Фаза 1: `dsp/graph/*`, `GraphEngine`, `MergeDelayPad`, интеграция в `PluginProcessor`.
- Сборка + `ctest` (GraphEngine): OK.
- `ReadLints` на изменённых файлах: без замечаний.

---

## Quick check — v0.1.0 (2026-03-25) — Initial CMake + JUCE

- Сборка Release на macOS: OK (`cmake --build build`).
- Затронутые файлы: `CMakeLists.txt`, `Source/*`, новые `docs/*`.
- Примечание: при необходимости — `ReadLints` на `Source/` после открытия в IDE.

---
