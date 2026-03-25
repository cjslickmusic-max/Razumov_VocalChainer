# Архитектура

## Поток аудио

1. `RazumovVocalChainAudioProcessor::processBlock` вызывает `razumov::graph::GraphEngine::process` на стерео-буфере.
2. `GraphEngine` держит **активный план** (`std::shared_ptr<GraphPlan>`). Новый план приходит через `submitPlan` (ожидаемо с message/UI thread), подхватывается в начале `process` **без** изменения векторов узлов внутри самого DSP-цикла (указатель на план меняется целиком).
3. План состоит из шагов `std::variant<SerialStep, ParallelStep>`:
   - **Serial:** узлы обрабатываются по очереди in-place.
   - **Parallel:** вход копируется в `workL_` / `workR_`, ветки обрабатываются независимо, затем к каждой ветке применяется **дополнительная задержка** (`MergeDelayPad`), чтобы суммарная задержка веток совпала с `max(latLeft, latRight)`, после чего буферы **суммируются** в выход.

## Задержка (PDC)

- Каждый узел реализует `AudioNode::getLatencySamples()`.
- `GraphPlan::computePluginLatencySamples()` суммирует задержки serial-шагов; для parallel-шага добавляет **max** по веткам (после выравнивания merge это отчётная задержка секции).
- `GraphEngine` вызывает `setLatencySamples` через callback при `prepare` и при смене плана (см. `PluginProcessor`).

## Файлы

| Область | Путь |
|---------|------|
| Базовый узел | `Source/dsp/graph/AudioNode.h` |
| Gain / Filter / Latency | `Source/dsp/graph/GainNode.h`, `FilterNode.*`, `LatencyNode.*` |
| План | `Source/dsp/graph/GraphPlan.*` |
| Движок | `Source/dsp/graph/GraphEngine.*`, `MergeDelayPad.*` |
| Фабрики планов | `Source/dsp/graph/GraphPlanFactory.*` |
| Процессор | `Source/PluginProcessor.*` |

## Тесты

Консольная цель `RazumovVocalChainTests` (см. `CMakeLists.txt`): проверка отчётной задержки и параллели 0.5+0.5.

## Параметры (фаза 2)

- `AudioProcessorValueTreeState` в `PluginProcessor`: идентификаторы в `Source/params/ParamIDs.h`, раскладка в `Source/params/ParameterLayout.cpp`.
- В `processBlock` значения читаются из APVTS и передаются в `GraphEngine::applyLiveParameters` (линейный gain, Hz для LP).
- Узлы в активном плане находятся через `AudioNode::asGain()` / `asFilter()` и кэш `gainBinding_` / `filterBinding_` после смены/подготовки плана.
