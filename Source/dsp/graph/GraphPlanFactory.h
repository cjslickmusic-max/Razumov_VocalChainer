#pragma once

#include "GraphPlan.h"
#include <memory>

namespace razumov::graph
{

/** Сборка типовых планов (UI/пресеты позже подставят свои). */
class GraphPlanFactory
{
public:
    /** Пустой граф — прозрачный проход. */
    static std::unique_ptr<GraphPlan> makePassthrough();

    /** Gain(1) + LP почти Nyquist — слышимо как passthrough. */
    static std::unique_ptr<GraphPlan> makeSerialGainAndWideFilter(double sampleRate);

    /** Две ветки с gain 0.5 + суммирование (уровень как у входа). */
    static std::unique_ptr<GraphPlan> makeParallelHalves();

    /** Для тестов: разная задержка в ветках — merge с выравниванием. */
    static std::unique_ptr<GraphPlan> makeParallelMismatchedLatencyForTests();
};

} // namespace razumov::graph
