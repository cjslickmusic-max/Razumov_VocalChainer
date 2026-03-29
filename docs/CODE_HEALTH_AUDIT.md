# Code Health Audit

Журнал **Quick check** (после фич) и **Full pass** (релиз / по запросу). Новые записи добавлять сверху, не удалять историю без необходимости.

---

## Quick check — v0.9.19 (2026-03-29) — Parallel + opens module palette (not Split x2/x3)

- `GraphPlanFactory::makeSplitDryBranchAndParallelModule`: 2-way split, dry 0.5 + (0.5 -> chosen module); `insertParallelModuleAfterSlot`.
- `PluginEditor`: `showParallelModuleMenuForSlot` replaces split count menu; context labels "Parallel module...".
- `docs/ARCHITECTURE.md`: parallel UX note.
- ReadLints scoped; `cmake --build` VST3 + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.18 (2026-03-29) — Warm neutral theme + atmosphere gradient + mic/room architecture note

- `DesignTokens.h`: warm stone backgrounds, borders, rotary ambient; accentSignal slightly warmer; `backgroundAtmosphereBottom` vignette.
- `PluginEditor::paint`: subtle bottom gradient after base fill; brushed texture opacity 0.055.
- `docs/ARCHITECTURE.md`: mic/room profile vs factory presets (global layer, onboarding vs header).
- ReadLints scoped; `cmake --build` VST3 + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.17 (2026-03-29) — Knob shadow tail + value box contrast + adaptive chain height

- `EditorVisualAssets::drawKnobSoftShadowStack`: contact base slightly weaker; long soft tail (ambient + wide contact, low alpha, +18/+28 px).
- `styleRotary`: `textBoxTextColourId` / `rotaryValueBoxFill` / outline; Mic/Spectral bypass toggles `textColourId` + `tickColourId`.
- `graphContainsAnySplit` + `PluginEditor::resized`: strip height 328 scaled when serial-only, 248 when any Split (parallel).
- `Tests/FlexGraphSerializationTests`: `testGraphContainsAnySplit`.
- ReadLints scoped; `cmake --build` VST3 + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.16 (2026-03-29) — Chain toolbar -> RMB menu + knob drop shadow + duplicate

- `PluginEditor`: removed Bypass/Remove/Left/Right/Add toolbar row; graph actions on right-click (`showChainContextMenu`: Bypass, Remove, Move, Duplicate, Add after, Parallel split); `+` RMB: Add / Parallel.
- `RazumovVocalChainAudioProcessor`: `duplicateRootModuleAfter`, `canDuplicateRootModuleSlot`; `ModuleParamsRuntime::copySlotParamsFromTo`.
- `EditorVisualAssets::drawKnobSoftShadowStack`: extra dark contact layer shifted down (shadow toward value box).
- ReadLints scoped; `cmake --build` VST3 + `RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.15 (2026-03-29) — Chain UI polish + swap test + button shadow rect

- `PluginEditor` `VocalChainerLookAndFeel::drawButtonBackground`: тень снизу рисуется с копией `fullRect`, верхний highlight по полной ширине кнопки (не после `removeFromBottom`).
- `Tests/FlexGraphSerializationTests.cpp`: `trySwapDirectRootModuleSlots` — успех для слотов 2/3, отказ для Mic+Gain.
- ReadLints: OK; `cmake --build` `RazumovVocalChain_VST3` + `RazumovVocalChainTests`; `./build/RazumovVocalChainTests`: OK.

---

## Quick check — v0.9.14 (2026-03-29) — Gaussian-blurred knob shadows (cached rasters)

- `EditorVisualAssets::drawKnobSoftShadowStack`: два кэшируемых ARGB-слоя (ImageConvolutionKernel Gaussian 9.5 / 4.2), тинт через токены `shadowRotaryAmbient` / `shadowRotaryContact`, `highResamplingQuality` при масштабе.
- `VocalChainerLookAndFeel::drawRotarySlider`: вместо трёх эллипсов — вызов стека теней.
- `cmake --build` `RazumovVocalChain_VST3`: OK.

---

## Quick check — v0.9.13 (2026-03-29) — Preset header strip + rotary hover + ComboBox chrome

- Шапка: градиент + тень под полосой пресета; заголовок слева / версия справа / Combo по центру; `paint`/`resized` с `scaled(16)`.
- `VocalChainerLookAndFeel`: ручка — hover x1.4 на активной дуге, disabled alpha; ComboBox — тень + hover border; кнопки — hover border accent.
- ReadLints: OK; `cmake --build` `RazumovVocalChain_VST3`: по плану.

---

## Quick check — v0.9.12 (2026-03-29) — Rotary soft-UI shadows + inactive track

- `DesignTokens.h`: `shadowRotaryAmbient`, `shadowRotaryMid`, `shadowRotaryContact`, `rotaryTrackInactive`.
- `VocalChainerLookAndFeel::drawRotarySlider`: многослойные тени, радиальный градиент «шайбы», тёмный полный трек, акцентная дуга + маркер.
- ReadLints: OK; `cmake --build` `RazumovVocalChain_VST3`: OK.

---

## Quick check — v0.9.9 (2026-03-29) — Product name VocalChainer + VERSIONING.md

- Отображаемое имя плагина: **Razumov VocalChainer** (`PRODUCT_NAME`, заголовок UI); артефакт VST3: `Artifacts/Razumov VocalChainer.vst3`; `BUNDLE_ID` `com.razumov.vocalchainer`; `VocalChainerLookAndFeel`.
- `docs/VERSIONING.md`: линия 0.9.x до **1.0.0**; документы и правила обновлены под новое имя бандла.
- `cmake --build` `RazumovVocalChain_VST3`: по плану после правок.

---

## Quick check — v0.9.8 (2026-03-29) — PluginEditor full token rollout

- `tokens.json` / `tokens.css`: surface, border, text, control, macro, knob; `DesignTokens.h`: `argb`, `macro`, `knob`; `PluginEditor.cpp` без цветовых литералов `0xff…`.
- `docs/design/DESIGN_SYSTEM.md`: таблица расширена; версия **0.9.8**.
- `cmake --build` `RazumovVocalChain_VST3`: OK.

---

## Quick check — v0.9.7 (2026-03-29) — Design tokens + DESIGN_SYSTEM.md

- `resources/design/tokens/tokens.json`, `tokens.css`; `docs/design/DESIGN_SYSTEM.md`; `Source/ui/DesignTokens.h`; `chain-graph-prototype.html` подключает `tokens.css`.
- `ChainStripComponent` / `PluginEditor::paint` используют `tokens::argb`; `docs/design/UI_ASSETS.md` — ссылка на дизайн-систему.
- ReadLints (изменённые исходники); `cmake --build` `RazumovVocalChain_VST3`: по плану после правок.

---

## Quick check — v0.9.5 (2026-03-28) — COPY_PLUGIN_AFTER_BUILD OFF (no ~/Library copy)

- `CMakeLists.txt`: `COPY_PLUGIN_AFTER_BUILD FALSE`; `README.md`, `Artifacts/README.md`, `HANDOFF_NEXT_AGENT.md`, `plugin-artifacts-only.mdc`.
- Версия **0.9.5**; `cmake -S . -B build` + `RazumovVocalChain_VST3`: по логу сборки без install в Library.

---

## Quick check — v0.9.4 (2026-03-28) — Canonical VST3 in Artifacts/ via POST_BUILD

- `CMakeLists.txt`: `POST_BUILD` на `RazumovVocalChain_VST3` -> `Artifacts/Razumov VocalChainer.vst3`; `README.md`, `Artifacts/README.md`, `agent-build-after-changes.mdc` (сборка `RazumovVocalChain_VST3`).
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
