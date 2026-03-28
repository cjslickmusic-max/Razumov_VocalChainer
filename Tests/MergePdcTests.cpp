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

/** После merge с PDC выход совпадает с чистой задержкой D на том же входе (синус / импульс). */
void testTwoWayMismatchedSineEqualsPureDelay()
{
    using namespace razumov::graph;
    constexpr int D = 64;
    GraphEngine engine;
    engine.prepare(48000.0, 512, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 512);
    razumov::tests::fillSine(buf, 48000.0, 333.0, 0.2f);

    juce::AudioBuffer<float> ref(1, 512);
    razumov::tests::copyBuffer(buf, ref);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(engine.getReportedLatencySamples() == D);
    for (int i = D; i < 512; ++i)
    {
        const float expected = ref.getSample(0, i - D);
        assert(razumov::tests::nearAbs(buf.getSample(0, i), expected, 2.0e-4f));
    }
}

void testThreeWayChainLatencyIs64()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests());
    assert(plan->computePluginLatencySamples() == 64);
}

void testThreeWayImpulsePeakAligned()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 256, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 256);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    const int peakIdx = findMaxAbsSampleIndex(buf, 0, 256);
    assert(peakIdx == 64);
    assert(razumov::tests::nearAbs(buf.getSample(0, 64), 1.0f, 2.0e-3f));
}

void testThreeWaySineEqualsPureDelay64()
{
    using namespace razumov::graph;
    constexpr int D = 64;
    GraphEngine engine;
    engine.prepare(48000.0, 512, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 512);
    razumov::tests::fillSine(buf, 48000.0, 777.0, 0.17f);
    juce::AudioBuffer<float> ref(1, 512);
    razumov::tests::copyBuffer(buf, ref);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(engine.getReportedLatencySamples() == D);
    for (int i = D; i < 512; ++i)
    {
        const float expected = ref.getSample(0, i - D);
        assert(razumov::tests::nearAbs(buf.getSample(0, i), expected, 2.5e-4f));
    }
}

void testThreeWayImpulseAcrossBlocks()
{
    using namespace razumov::graph;
    constexpr int block = 48;
    GraphEngine engine;
    engine.prepare(48000.0, block, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;

    juce::AudioBuffer<float> a(1, block);
    a.clear();
    a.setSample(0, 0, 1.0f);
    engine.process(a, macros, mod);

    juce::AudioBuffer<float> b(1, block);
    b.clear();
    engine.process(b, macros, mod);

    assert(razumov::tests::nearAbs(a.getSample(0, 0), 0.0f, 1.0e-3f));
    assert(razumov::tests::nearAbs(b.getSample(0, 16), 1.0f, 2.0e-3f));
}

void testNestedMismatchLatencyIs32()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests());
    assert(plan->computePluginLatencySamples() == 32);
    assert(plan->computeMaxSplitNestingDepth() == 2);
}

void testNestedMismatchImpulseUnityAt32()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 128, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 128);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    const int peakIdx = findMaxAbsSampleIndex(buf, 0, 128);
    assert(peakIdx == 32);
    assert(razumov::tests::nearAbs(buf.getSample(0, 32), 1.0f, 2.0e-3f));
}

void testNestedMismatchPreservesDc()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 64, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 64);
    for (int i = 0; i < 64; ++i)
        buf.setSample(0, i, -0.0625f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    for (int i = 0; i < 64; ++i)
        assert(razumov::tests::nearAbs(buf.getSample(0, i), -0.0625f, 1.0e-4f));
}

} // namespace

void runMergePdcTests()
{
    testTwoWayMismatchedSineEqualsPureDelay();
    testThreeWayChainLatencyIs64();
    testThreeWayImpulsePeakAligned();
    testThreeWaySineEqualsPureDelay64();
    testThreeWayImpulseAcrossBlocks();
    testNestedMismatchLatencyIs32();
    testNestedMismatchImpulseUnityAt32();
    testNestedMismatchPreservesDc();
}
