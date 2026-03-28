#include "DspTestHelpers.h"

#include <dsp/graph/FlexGraphPlan.h>
#include <dsp/graph/GraphEngine.h>
#include <dsp/graph/GraphPlanFactory.h>
#include <params/ModuleParamsRuntime.h>

#include <cassert>
#include <cmath>

namespace
{

int findMaxAbsSampleIndex(const juce::AudioBuffer<float>& buf, int channel, int numSamples)
{
    int idx = 0;
    float mx = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        const float v = std::abs(buf.getSample(channel, i));
        if (v > mx)
        {
            mx = v;
            idx = i;
        }
    }
    return idx;
}

/** Без phase align импульс на выходе в t=0 (две ветки 0.5). */
void testHalvesImpulseAtZero()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 128, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesDesc());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 128);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(findMaxAbsSampleIndex(buf, 0, 128) == 0);
    assert(razumov::tests::nearAbs(buf.getSample(0, 0), 1.0f, 1.0e-3f));
}

/** Phase align 11 на ветке 0: суммарная задержка до merge = max(11,0)=11, пик на 11. */
void testHalvesPhaseAlign11Peak()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 128, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesPhaseAlignDesc(11, 0));
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 128);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(engine.getReportedLatencySamples() == 11);
    assert(findMaxAbsSampleIndex(buf, 0, 128) == 11);
    assert(razumov::tests::nearAbs(buf.getSample(0, 11), 1.0f, 2.0e-3f));
}

/** Симметричный phase align: max(5,8)=8, обе ветки выровнены к 8. */
void testHalvesPhaseAlignSymmetricLatency()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesPhaseAlignDesc(5, 8));
    assert(plan->computePluginLatencySamples() == 8);
}

/** Синус после merge с phase align совпадает с задержкой max(seg+ph). */
void testHalvesPhaseAlignSineVsDelay()
{
    using namespace razumov::graph;
    constexpr int pa0 = 9;
    constexpr int pa1 = 3;
    constexpr int mx = juce::jmax(pa0, pa1);

    GraphEngine engine;
    engine.prepare(48000.0, 512, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesPhaseAlignDesc(pa0, pa1));
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 512);
    razumov::tests::fillSine(buf, 48000.0, 412.0, 0.18f);
    juce::AudioBuffer<float> ref(1, 512);
    razumov::tests::copyBuffer(buf, ref);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(engine.getReportedLatencySamples() == mx);
    for (int i = mx; i < 512; ++i)
    {
        const float expected = ref.getSample(0, i - mx);
        assert(razumov::tests::nearAbs(buf.getSample(0, i), expected, 2.5e-4f));
    }
}

} // namespace

void runPhaseAlignTests()
{
    testHalvesImpulseAtZero();
    testHalvesPhaseAlign11Peak();
    testHalvesPhaseAlignSymmetricLatency();
    testHalvesPhaseAlignSineVsDelay();
}
