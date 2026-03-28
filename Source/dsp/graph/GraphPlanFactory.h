#pragma once

#include "FlexGraphDesc.h"
#include "FlexGraphPlan.h"
#include <memory>

namespace razumov::graph
{

/** Сборка описаний и планов (пресеты / тесты). */
class GraphPlanFactory
{
public:
    /** Пустой граф — прозрачный проход. */
    static FlexSegmentDesc makePassthroughDesc();

    /** Описание: Gain(1) + LP почти Nyquist. */
    static FlexSegmentDesc makeSerialGainAndWideFilterDesc(double sampleRate);

    /** Фаза 3: mic -> gain -> LP -> de-esser -> Opto/FET/VCA -> exciter -> spectral. */
    static FlexSegmentDesc makeDefaultVocalChainPhase3Desc(double sampleRate);

    static FlexSegmentDesc makeCompactVocalChainPhase3Desc(double sampleRate);
    static FlexSegmentDesc makeFetForwardVocalChainPhase3Desc(double sampleRate);

    /** Индекс 0 = Full, 1 = Compact, 2 = FET-forward (APVTS chainProfile). */
    static FlexSegmentDesc makeStartupDescForIndex(int index, double sampleRate);

    /** Две ветки gain 0.5 + суммирование (уровень как у входа). */
    static FlexSegmentDesc makeParallelHalvesDesc();

    /** Три ветки gain 1/3 + суммирование (тесты). */
    static FlexSegmentDesc makeParallelThirdsDesc();

    /** Вложенный split: внутри ветки 0 — два gain 0.25, ветка 1 — gain 0.5 (сумма 1). */
    static FlexSegmentDesc makeNestedParallelHalvesDesc();

    /** Разная задержка в ветках — merge с выравниванием (тесты). */
    static FlexSegmentDesc makeParallelMismatchedLatencyDescForTests();

    /** Три ветки с разной задержкой (0 / 32 / 64) и gain 1/3 — PDC на merge (тесты). */
    static FlexSegmentDesc makeParallelThreeWayMismatchedLatencyDescForTests();

    /** Вложенный split: внутри ветки 0 — 32 сэмпла задержки на одной половине (тесты). */
    static FlexSegmentDesc makeNestedParallelMismatchedLatencyDescForTests();

    static std::unique_ptr<FlexGraphPlan> makePlanFromDesc(const FlexSegmentDesc& desc);

    static std::unique_ptr<FlexGraphPlan> makeStartupChainForIndex(int index, double sampleRate);

    /** Новый модуль в палитру редактора (id=0, назначит процессор). */
    static FlexSlotDesc makeModulePaletteSlot(AudioNodeKind kind);

    /** Split с N ветками, в каждой Gain(1) как заглушка прохода. */
    static FlexSlotDesc makeSplitWithUnityBranches(int numBranches);
};

} // namespace razumov::graph
