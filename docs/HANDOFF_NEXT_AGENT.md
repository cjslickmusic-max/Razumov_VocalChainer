# Handoff для следующего агента — Razumov Vocal Chain

**Дата:** 2026-03-28  
**Состояние репозитория:** `main`, версия продукта **0.9.1** (`CMakeLists.txt`, строка версии в `PluginEditor::paint`).

---

## 0. Система тестов (для следующего агента)

- **Документ:** `docs/TESTING.md` — структура таргета, политика детерминизма / DC / фаза на задержке.
- **Точка входа:** `Tests/TestMain.cpp` вызывает `runGraphEngineTests()` (`GraphTests.cpp`), `runMergePdcTests()` (`MergePdcTests.cpp`), `runDspDeterminismTests()` (`DspDeterminismTests.cpp`).
- **Хелперы:** `Tests/DspTestHelpers.h` (`fillSine`, сравнение буферов).
- **Идея:** на одних и тех же входах узел даёт тот же выход после `reset()`; тишина на входе компрессоров → тишина на выходе; `LatencyNode` сохраняет сдвиг синуса; расширять новыми `run*Tests()` и вызовом из `TestMain`.

---

## 1. Что сделано непосредственно перед этим handoff

- **Макросы ×8 (этап H по продуктовой карте):** в APVTS добавлены `macroPunch`, `macroBody`, `macroSmooth`, `macroDensity` (плюс прежние Glue / Air / Sibilance / Presence). Состояние: `MacroAudioState` в `Source/params/ModuleParamsRuntime.h`, смещения в `applyMacroOffsetsToPhase3` (`ModuleParamsRuntime.cpp`), чтение из APVTS в `buildMacroStateFromApvts` (`PluginProcessor.cpp`). Фабричные пресеты сбрасывают все восемь в **0.5** (`presets/FactoryPresets.cpp`).
- **Полоса цепочки и параллели:** у `ChainStripItem` есть поле **`row`** (глубина после split); обход в `FlexGraphDesc.cpp` (`walkSegmentForStrip(..., depth)`). `ChainStripComponent` рисует карточки с `row > 0` на нижнем ряду; высота полосы в редакторе **100px**.
- **Документы:** обновлены `docs/ROADMAP.md` (журнал вех), `docs/CODE_HEALTH_AUDIT.md`, `AGENTS.md`. Коммит с сообщением вида `[WIP] v0.9.1 macros x8 parallel strip rows`.

---

## 2. Куда смотреть в первую очередь

| Задача | Файлы / места |
|--------|----------------|
| Очередь работ и критерии | `docs/ROADMAP.md` |
| Приоритеты продукта (цепочка, макросы, компрессоры, **фаза на параллелях**, микрофон/комната — в конце) | `docs/PRODUCT_ARCHITECTURE_ROADMAP.md` |
| Правила агента (язык ответов RU, DSP safety, git WIP-коммиты, UI ASCII) | `AGENTS.md`, `.cursor/rules/*.mdc` |
| Граф, PDC, merge | `Source/dsp/graph/GraphEngine.*`, `MergeDelayPad`, `FlexGraphPlan` |
| Макросы и per-slot модули | `ModuleParamsRuntime.*`, `PluginProcessor::processBlock` / `graphEngine_.process` |

---

## 3. Логичные следующие крупные блоки (не выбирай всё сразу — согласуй с постановщиком)

1. **Фаза / выравнивание между параллельными ветками (этап G в продуктовом документе):** PDC по задержке уже есть; дальше — опции вроде «Phase align» (all-pass / микрозадержка / UX), если это следующий продуктовый приоритет.
2. **Качество компрессоров (этап E):** отдельные узлы уже есть; улучшение моделей/тайминга — долгая итерация.
3. **Фаза 1, хвост:** расширенные DSP-тесты импульса/фазы на merge (см. `docs/ROADMAP.md`, блок после таблицы фазы 1).
4. **Advanced UI (canvas, кабели)** — после стабильного Main, см. `PRODUCT_ARCHITECTURE_ROADMAP.md`.
5. **Не ставить в ближайшую очередь без явного запроса:** DSP профиля микрофона по замерам, коррекция комнаты — см. раздел «Порядок приоритетов» в продуктовом документе.

---

## 4. Сборка и тесты

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/RazumovVocalChainTests
```

Цель CMake: **`RazumovVocalChain`**. JUCE **8.0.12** через FetchContent.

---

## 5. Риски / неочевидное

- **Макросы** суммируются в `applyMacroOffsetsToPhase3` последовательно; пересечения по одним и тем же параметрам (например LP у Air и Body) — осознанный компромисс, при тюнинге проверять на ухо и лимиты.
- **Состояние проекта:** APVTS + вложенный граф (`FlexGraphSerialization`) + `ModuleParams` в state; при новых параметрах старые сессии обычно подхватывают дефолты JUCE, но стоит проверить загрузку старых пресетов/проектов.
- После правок DSP — **Release** и быстрая проверка на щелчки (правило `dsp-audio-safety.mdc`).

---

## 6. Версионирование

Политика **0.8.x / 0.9.x** — см. `docs/ROADMAP.md` (раздел «Версионирование до 1.0.0»). После значимых правок DSP/UI — поднять `project(VERSION ...)` в `CMakeLists.txt` и синхронно строку в UI при ручном отображении версии.

---

*Этот файл можно обновлять или удалять после принятия задачи следующим агентом.*
