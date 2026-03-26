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
| UI: полоса цепочки | `Source/ui/ChainStripComponent.*` |

## UI (редактор)

- Полоса **Signal path** (`ChainStripComponent`): read-only карточки и стрелки по текущему `chainProfile`, визуально в духе **normal** chain view из Razumov ShaperX (без drag/add/remove и без отдельного движка графа в UI).
- Редактируемый граф по-прежнему задаётся кодом (`GraphPlanFactory`) + параметр `chainProfile`; свободная расстановка узлов (как Advanced canvas в ShaperX) в планах — отдельно, без привязки к текущему DSP.

## Тесты

Консольная цель `RazumovVocalChainTests` (см. `CMakeLists.txt`): проверка отчётной задержки и параллели 0.5+0.5.

## Параметры (фаза 2–3)

- `AudioProcessorValueTreeState` в `PluginProcessor`: идентификаторы в `Source/params/ParamIDs.h`, раскладка в `Source/params/ParameterLayout.cpp`.
- В `processBlock` значения читаются из APVTS, собираются в `razumov::params::Phase3RealtimeParams` и передаются в `GraphEngine::applyPhase3Parameters`.
- Узлы в активном плане сопоставляются по `AudioNode::getKind()` и `GraphNodeBindings` после `refreshParameterBindings` при подготовке плана.

## Пресеты (фаза 4)

- Фабричный банк: `Source/presets/FactoryPresets.*`; `applyFactoryPreset` задаёт APVTS; индекс программы хоста (`getNumPrograms` / `setCurrentProgram`) совпадает с выбором в UI.
- Стартовая цепочка: `chainProfile` в APVTS → `GraphPlanFactory::makeStartupChainForIndex` → `GraphEngine::submitPlan` (смена при автоматизации/загрузке проекта; `prepareToPlay` подаёт план по текущему индексу).
- Макросы (`macroGlue` … `macroPresence`, 0…1, нейтраль 0.5): смещение групп параметров в `buildPhase3RealtimeParams` поверх значений ручек APVTS.
- `MicCorrectionNode`: pass-through, `getLatencySamples() == 0` (профиль mic — по `docs/ROADMAP.md`). `SpectralCompressorNode`: STFT 1024 / hop 512, динамика по модулю с сохранением фазы; при активном spectral — `getLatencySamples() == 1024`, dry задержан и смешивается с wet; bypass — 0.
