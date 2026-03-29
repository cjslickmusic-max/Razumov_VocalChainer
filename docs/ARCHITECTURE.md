# Архитектура

## Поток аудио

1. `RazumovVocalChainAudioProcessor::processBlock` читает макросы из APVTS, затем вызывает `razumov::graph::GraphEngine::process(buffer, macros, moduleParams)` на стерео-буфере.
2. `GraphEngine` держит **активный план** (`std::shared_ptr<FlexGraphPlan>`). Новый план приходит через `submitPlan` (message/UI thread), подхватывается в начале `process` **без** изменения топологии внутри DSP-цикла (меняется указатель на план целиком).
3. В начале `process`: **swap** pending-плана → для каждого модуля по `slotId` — `ModuleParamsRuntime::fillSlot` → `applyPhase3ToNode` → расчёт динамической задержки → **рекурсивный** `processSegment` по корню.
4. План — **FlexSegment** (`std::vector<FlexSlot>`), слот:
   - **Module:** один `AudioNode`, in-place `process`, при `bypassed` — пропуск.
   - **Split:** `N >= 2` веток (`FlexSegment` каждая). Вход копируется в **N** предвыделенных буферов пула, ветки обрабатываются независимо; к каждой ветке — **два** `MergeDelayPad` подряд: (1) **phase align** — целочисленная задержка `branchPhaseAlignSamples[i]` (этап G, подгонка фазы между ветками поверх PDC); (2) **PDC merge** — до `max(lat_i + phaseAlign_i)` по веткам; затем ветки **суммируются** в выходной буфер. Индекс pad: `splitDepth * maxSplitBreadth + branchIndex` (вложенные split не делят один и тот же экземпляр с внешним). Размер пула **на тип** (phase + PDC): `2 * maxSplitBreadth * max(1, computeMaxSplitNestingDepth())` в `GraphEngine::ensureBranchPool`.

## Задержка (PDC)

- Каждый узел реализует `AudioNode::getLatencySamples()`.
- По дереву: по **serial** — сумма задержек слотов; внутри **Split** — **max** по веткам **с учётом** `branchPhaseAlignSamples` на ветку; рекурсивно. Это же значение — отчётная задержка плагина для хоста.
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
- Топология: `FlexSegmentDesc` + `chainProfile`; правки графа из контекстного меню и `+` на полосе.
- Параллель: **`+` под узлом** открывает палитру модуля и вставляет **split из двух веток** (сухая 0.5 + ветка 0.5 -> выбранный модуль), без отдельного меню «Split x2» / «Split x3».

## Профиль микрофона / комнаты vs фабричные пресеты

**Цель продукта:** коррекция под конкретный микрофон (и при необходимости комнату) — это **не** часть «косметического» пресета цепочки, а **глобальный слой** поверх него.

Рекомендуемая модель:

1. **Сохранение:** `micProfile` и параметры комнаты (когда появятся в DSP) хранить в **APVTS / общем состоянии плагина** так, чтобы смена **factory preset** (макросы + граф по умолчанию) **не затирала** профиль, если явно не сказано обратное. Пресет = цепочка + макросы + опциональные дефолты слотов; профиль = отдельный блок в `ValueTree` или отдельные параметры верхнего уровня.
2. **UI:** не прятать микрофон «вглубь» навсегда: пользователь должен **видеть**, что плагин «привязан» к профилю (компактная строка в шапке, чип «Mic: …», или первый запуск мастера). Полноэкранный wizard при каждом открытии обычно мешает; **одноразовый onboarding** + постоянный доступ из шапки — разумный компромисс.
3. **Узел Mic correction** в графе остаётся местом **amount/bypass** для этого слота; выбор **какого** профиля применять — глобально (`micProfile`), чтобы любой пресет звучал предсказуемо лучше с уже настроенным микрофоном.

## Тесты

Консольная цель `RazumovVocalChainTests`: см. **`docs/TESTING.md`**. Граф (`GraphTests.cpp`, `MergePdcTests.cpp`, `PhaseAlignTests.cpp`): PDC/merge, phase align на ветках, импульсы, вложенные split. DSP (`DspDeterminismTests.cpp`): детерминизм узлов (компрессоры Opto/FET/VCA, Gain, Filter, De-esser, Exciter, Mic, `MergeDelayPad`, `LatencyNode`), тишина на нуле, сдвиг синуса на задержке, DC через Gain.

## Параметры

- **Глобально (APVTS):** `chainProfile`, `micProfile`, **восемь** макросов — `ParameterLayout.cpp`, `ParamIDs.h`.
- **По экземпляру узла (per `slotId`):** `ModuleParamsRuntime` — атомики и узел `ModuleParams` внутри XML состояния плагина. В `processBlock` для каждого модуля: `fillSlot` → `Phase3RealtimeParams` → узел; макросы применяются в `applyMacroOffsetsToPhase3` после сырых значений слота.
- **Загрузка старых проектов** без `ModuleParams`: разбор legacy `PARAM` в XML и начальное заполнение слотов (общий снимок).

## Сохранение состояния

- `getStateInformation` / `setStateInformation`: FlexGraph + `ModuleParams` в том же `ValueTree`, что и параметры APVTS.
- `MicCorrectionNode` / `SpectralCompressorNode`: см. `docs/ROADMAP.md` и комментарии в узлах.
