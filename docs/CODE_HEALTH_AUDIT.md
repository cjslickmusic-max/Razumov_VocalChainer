# Code Health Audit

Журнал **Quick check** (после фич) и **Full pass** (релиз / по запросу). Новые записи добавлять сверху, не удалять историю без необходимости.

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
