# Архитектура

## Поток аудио

1. `RazumovVocalChainAudioProcessor::processBlock` читает макросы из APVTS, затем вызывает `razumov::graph::GraphEngine::process(buffer, macros, moduleParams)` на стерео-буфере.
2. `GraphEngine` держит **активный план** (`std::shared_ptr<FlexGraphPlan>`). Новый план приходит через `submitPlan` (message/UI thread), подхватывается в начале `process` **без** изменения топологии внутри DSP-цикла (меняется указатель на план целиком).
3. В начале `process`: **swap** pending-плана → для каждого модуля по `slotId` — `ModuleParamsRuntime::fillSlot` → `applyPhase3ToNode` → расчёт динамической задержки → **рекурсивный** `processSegment` по корню.
4. План — **FlexSegment** (`std::vector<FlexSlot>`), слот:
   - **Module:** один `AudioNode`, in-place `process`, при `bypassed` — пропуск.
   - **Split:** `N >= 2` веток (`FlexSegment` каждая). Вход копируется в **N** предвыделенных буферов пула, ветки обрабатываются независимо; к каждой ветке — **MergeDelayPad** до `max(lat_i)`; затем ветки **суммируются** в выходной буфер. Индекс pad: `splitDepth * maxSplitBreadth + branchIndex` (вложенные split не делят один и тот же экземпляр с внешним). Размер пула: `maxSplitBreadth * max(1, computeMaxSplitNestingDepth())` в `FlexGraphPlan`.

## Задержка (PDC)

- Каждый узел реализует `AudioNode::getLatencySamples()`.
- По дереву: по **serial** — сумма задержек слотов; внутри **Split** — **max** по веткам; рекурсивно. Это же значение — отчётная задержка плагина для хоста.
- `GraphEngine` вызывает `setLatencySamples` через callback при `prepare`, при смене плана и при изменении задержки внутри блока (например spectral bypass).

## Описание vs исполнение

- **`FlexSegmentDesc` / `FlexSlotDesc`** (`FlexGraphDesc.*`): POD-дерево для фабрики, UI и сериализации.
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
| Параметры по слотам | `Source/params/ModuleParamsRuntime.*` |
| Процессор | `Source/PluginProcessor.*` |
| UI: полоса цепочки | `Source/ui/ChainStripComponent.*` |

## UI (редактор)

- Полоса **Signal path** (`ChainStripComponent`): карточки по графу; выбор слота задаёт `selectedSlotId_`, инспектор ниже показывает параметры **этого** слота.
- Параметр **`micProfile`**: выбор из списка (датасет), подгрузка в DSP — отдельная задача.
- Топология: `FlexSegmentDesc` + `chainProfile`, правки через toolbar (Add / Split / …).

## Тесты

Консольная цель `RazumovVocalChainTests`: см. **`docs/TESTING.md`**. Граф (`GraphTests.cpp`): PDC/merge, импульсы, вложенные split. DSP (`DspDeterminismTests.cpp`): детерминизм узлов (компрессоры Opto/FET/VCA, Gain, Filter, De-esser, Exciter, Mic, `MergeDelayPad`, `LatencyNode`), тишина на нуле, сдвиг синуса на задержке, DC через Gain.

## Параметры

- **Глобально (APVTS):** `chainProfile`, `micProfile`, **восемь** макросов — `ParameterLayout.cpp`, `ParamIDs.h`.
- **По экземпляру узла (per `slotId`):** `ModuleParamsRuntime` — атомики и узел `ModuleParams` внутри XML состояния плагина. В `processBlock` для каждого модуля: `fillSlot` → `Phase3RealtimeParams` → узел; макросы применяются в `applyMacroOffsetsToPhase3` после сырых значений слота.
- **Загрузка старых проектов** без `ModuleParams`: разбор legacy `PARAM` в XML и начальное заполнение слотов (общий снимок).

## Сохранение состояния

- `getStateInformation` / `setStateInformation`: FlexGraph + `ModuleParams` в том же `ValueTree`, что и параметры APVTS.
- `MicCorrectionNode` / `SpectralCompressorNode`: см. `docs/ROADMAP.md` и комментарии в узлах.
