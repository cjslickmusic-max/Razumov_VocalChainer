# Архитектура

## Поток аудио

1. `RazumovVocalChainAudioProcessor::processBlock` собирает `Phase3RealtimeParams` и вызывает `razumov::graph::GraphEngine::process(buffer, params)` на стерео-буфере.
2. `GraphEngine` держит **активный план** (`std::shared_ptr<FlexGraphPlan>`). Новый план приходит через `submitPlan` (message/UI thread), подхватывается в начале `process` **без** изменения топологии внутри DSP-цикла (меняется указатель на план целиком).
3. В начале `process`: **swap** pending-плана → **applyPhase3Parameters** (обход всего дерева) → расчёт динамической задержки → **рекурсивный** `processSegment` по корню.
4. План — **FlexSegment** (`std::vector<FlexSlot>`), слот:
   - **Module:** один `AudioNode`, in-place `process`, при `bypassed` — пропуск.
   - **Split:** `N >= 2` веток (`FlexSegment` каждая). Вход копируется в **N** предвыделенных буферов пула, ветки обрабатываются независимо; к каждой ветке — **MergeDelayPad** до `max(lat_i)`; затем ветки **суммируются** в выходной буфер.

## Задержка (PDC)

- Каждый узел реализует `AudioNode::getLatencySamples()`.
- По дереву: по **serial** — сумма задержек слотов; внутри **Split** — **max** по веткам; рекурсивно. Это же значение — отчётная задержка плагина для хоста.
- `GraphEngine` вызывает `setLatencySamples` через callback при `prepare`, при смене плана и при изменении задержки внутри блока (например spectral bypass).

## Описание vs исполнение

- **`FlexSegmentDesc` / `FlexSlotDesc`** (`FlexGraphDesc.*`): POD-дерево для фабрики, UI и будущей сериализации.
- **`FlexGraphPlan::buildFromDesc`** + **`AudioNodeFactory`**: сборка владеющего дерева `FlexSlot` с узлами.
- **`GraphPlanFactory`**: типовые `FlexSegmentDesc` (в т.ч. `makeStartupDescForIndex` под `chainProfile`).

## Файлы

| Область | Путь |
|---------|------|
| Базовый узел | `Source/dsp/graph/AudioNode.h` |
| Gain / Filter / Latency | `Source/dsp/graph/GainNode.h`, `FilterNode.*`, `LatencyNode.*` |
| Описание графа | `Source/dsp/graph/FlexGraphDesc.*` |
| План исполнения | `Source/dsp/graph/FlexGraphPlan.*` |
| Фабрика узлов | `Source/dsp/graph/AudioNodeFactory.*` |
| Движок | `Source/dsp/graph/GraphEngine.*`, `MergeDelayPad.*` |
| Фабрики описаний | `Source/dsp/graph/GraphPlanFactory.*` |
| Процессор | `Source/PluginProcessor.*` |
| UI: полоса цепочки | `Source/ui/ChainStripComponent.*` |

## UI (редактор)

- Полоса **Signal path** (`ChainStripComponent`): read-only карточки по **`segmentDescToChainStripLabels(graphDesc_)`** (синхронно с последним собранным планом); обновление при смене `chainProfile`.
- Параметр **`micProfile`**: выбор профиля микрофона (список из датасета), пока **без** подгрузки в DSP.
- Редактируемый граф в UI (canvas) — отдельно; сейчас топология задаётся кодом через `FlexSegmentDesc` + `chainProfile`.

## Тесты

Консольная цель `RazumovVocalChainTests`: отчётная задержка при несовпадающих задержках веток и параллель 0.5+0.5.

## Параметры (фаза 2–3)

- `AudioProcessorValueTreeState`: глобальные параметры — `chainProfile`, `micProfile`, четыре макроса (`ParameterLayout.cpp`, `ParamIDs.h`).
- Параметры **модулей по экземпляру** (per `slotId`): `ModuleParamsRuntime` — атомики + дочерний `ValueTree` `ModuleParams` в состоянии плагина; в `processBlock` макросы читаются из APVTS, для каждого слота `GraphEngine` вызывает `fillSlot(slotId, macros, Phase3RealtimeParams)` и применяет к узлу.
- Старые проекты без `ModuleParams`: миграция из legacy `PARAM` в XML (общий снимок на все слоты).

## Пресеты (фаза 4)

- Фабричный банк: `Source/presets/FactoryPresets.*`; глобально — `applyFactoryPresetGlobals` (mic profile + макросы); снимок модулей — `buildFactoryPresetModulePhase3` → `seedAllSlotsWithSameParams` на все слоты.
- Стартовая цепочка: `chainProfile` → `makeStartupDescForIndex` → `graphDesc_` + `makePlanFromDesc` → `submitPlan`; `prepareToPlay` и загрузка состояния синхронизируют описание и план.
- Макросы: смещение в `applyMacroOffsetsToPhase3` после чтения сырых значений слота.
- `MicCorrectionNode` / `SpectralCompressorNode`: см. `docs/ROADMAP.md` и комментарии в узлах.
