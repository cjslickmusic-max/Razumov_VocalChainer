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

    /** Разная задержка в ветках — merge с выравниванием (тесты). */
    static FlexSegmentDesc makeParallelMismatchedLatencyDescForTests();

    static std::unique_ptr<FlexGraphPlan> makePlanFromDesc(const FlexSegmentDesc& desc);

    static std::unique_ptr<FlexGraphPlan> makeStartupChainForIndex(int index, double sampleRate);
};

} // namespace razumov::graph
