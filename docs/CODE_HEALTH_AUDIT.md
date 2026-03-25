# Code Health Audit

Журнал **Quick check** (после фич) и **Full pass** (релиз / по запросу). Новые записи добавлять сверху, не удалять историю без необходимости.

---

## Quick check — v0.3.0 (2026-03-25) — APVTS + Phase 2

- Параметры: `params/ParamIDs`, `ParameterLayout`, APVTS, state XML; UI роторы; `GraphEngine::applyLiveParameters`.
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
