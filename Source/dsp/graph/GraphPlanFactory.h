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

    /**
     * Цепочка фазы 3: mic slot (пока прозрачно) -> gain -> LP -> de-esser -> Opto/FET/VCA -> exciter -> spectral stub.
     */
    static std::unique_ptr<GraphPlan> makeDefaultVocalChainPhase3(double sampleRate);

    /** Короче: без de-esser и без FET/VCA — один Opto, дальше exciter + spectral. */
    static std::unique_ptr<GraphPlan> makeCompactVocalChainPhase3(double sampleRate);

    /** Тот же набор узлов, порядок компрессоров: FET → Opto → VCA (другой «характер» цепи). */
    static std::unique_ptr<GraphPlan> makeFetForwardVocalChainPhase3(double sampleRate);

    /** Индекс 0 = Full, 1 = Compact, 2 = FET-forward (см. APVTS chainProfile). */
    static std::unique_ptr<GraphPlan> makeStartupChainForIndex(int index, double sampleRate);

    /** Две ветки с gain 0.5 + суммирование (уровень как у входа). */
    static std::unique_ptr<GraphPlan> makeParallelHalves();

    /** Для тестов: разная задержка в ветках — merge с выравниванием. */
    static std::unique_ptr<GraphPlan> makeParallelMismatchedLatencyForTests();
};

} // namespace razumov::graph
