#include <dsp/graph/FlexGraphPlan.h>
#include <dsp/graph/GraphEngine.h>
#include <dsp/graph/GraphPlanFactory.h>
#include <dsp/graph/MergeDelayPad.h>
#include <params/ModuleParamsRuntime.h>

#include <cassert>
#include <cmath>

namespace
{

bool nearAbs(float a, float b, float eps)
{
    return std::abs(a - b) <= eps;
}

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

} // namespace

static void testChainLatencySum()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    assert(plan != nullptr);
    const int lat = plan->computePluginLatencySamples();
    assert(lat == 64);
}

static void testParallelHalvesIsUnityGain()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 512, 2);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesDesc());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(2, 512);
    buf.clear();
    buf.setSample(0, 0, 1.0f);
    buf.setSample(1, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    const float l = buf.getSample(0, 0);
    const float r = buf.getSample(1, 0);
    assert(nearAbs(l, 1.0f, 1.0e-3f));
    assert(nearAbs(r, 1.0f, 1.0e-3f));
}

static void testMergeDelayPadImpulse()
{
    using namespace razumov::graph;
    MergeDelayPad pad;
    pad.prepare(1, 256, 256);
    pad.setDelaySamples(17);

    juce::AudioBuffer<float> buf(1, 64);
    buf.clear();
    buf.setSample(0, 0, 1.0f);
    pad.process(buf);

    assert(nearAbs(buf.getSample(0, 0), 0.0f, 1.0e-6f));
    assert(nearAbs(buf.getSample(0, 16), 0.0f, 1.0e-6f));
    assert(nearAbs(buf.getSample(0, 17), 1.0f, 1.0e-4f));
}

static void testMismatchedParallelImpulsePeakAligned()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 256, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 256);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    const int peakIdx = findMaxAbsSampleIndex(buf, 0, 256);
    assert(peakIdx == 64);
    assert(nearAbs(buf.getSample(0, 64), 1.0f, 2.0e-3f));
}

static void testMismatchedParallelImpulseAcrossSmallBlocks()
{
    using namespace razumov::graph;
    GraphEngine engine;
    const int block = 32;
    engine.prepare(48000.0, block, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
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

    juce::AudioBuffer<float> c(1, block);
    c.clear();
    engine.process(c, macros, mod);

    assert(nearAbs(a.getSample(0, 0), 0.0f, 1.0e-3f));
    assert(nearAbs(b.getSample(0, 0), 0.0f, 1.0e-3f));
    assert(nearAbs(c.getSample(0, 0), 1.0f, 2.0e-3f));
}

static void testParallelThirdsUnity()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 128, 2);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelThirdsDesc());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(2, 128);
    buf.clear();
    buf.setSample(0, 10, 0.75f);
    buf.setSample(1, 10, -0.4f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(nearAbs(buf.getSample(0, 10), 0.75f, 1.0e-3f));
    assert(nearAbs(buf.getSample(1, 10), -0.4f, 1.0e-3f));
}

static void testNestedParallelUnity()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 256, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeNestedParallelHalvesDesc());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 256);
    buf.clear();
    buf.setSample(0, 5, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    assert(nearAbs(buf.getSample(0, 5), 1.0f, 1.0e-3f));
}

static void testReportedLatencyMatchesPlan()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(44100.0, 512, 2);
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    const int expected = plan->computePluginLatencySamples();
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(plan)));
    assert(engine.getReportedLatencySamples() == expected);
    assert(expected == 64);
}

static void testMergePreservesDcHalves()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 64, 1);
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesDesc());
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(u)));

    juce::AudioBuffer<float> buf(1, 64);
    for (int i = 0; i < 64; ++i)
        buf.setSample(0, i, 0.125f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    for (int i = 0; i < 64; ++i)
        assert(nearAbs(buf.getSample(0, i), 0.125f, 1.0e-4f));
}

static void testSerialWideFilterPassesImpulse()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeSerialGainAndWideFilterDesc(48000.0));
    assert(plan->computePluginLatencySamples() == 0);

    GraphEngine engine;
    engine.prepare(48000.0, 128, 1);
    engine.submitPlan(std::shared_ptr<FlexGraphPlan>(std::move(plan)));

    juce::AudioBuffer<float> buf(1, 128);
    buf.clear();
    buf.setSample(0, 0, 1.0f);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;
    engine.process(buf, macros, mod);

    float energy = 0.0f;
    for (int i = 0; i < 128; ++i)
        energy += buf.getSample(0, i) * buf.getSample(0, i);
    assert(energy > 0.9f * 0.9f);
    assert(nearAbs(buf.getSample(0, 0), 1.0f, 0.05f));
}

static void testPlanNestingDepthForNestedGraph()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeNestedParallelHalvesDesc());
    assert(plan->computeMaxSplitNestingDepth() == 2);
}

int main()
{
    testChainLatencySum();
    testParallelHalvesIsUnityGain();
    testMergeDelayPadImpulse();
    testMismatchedParallelImpulsePeakAligned();
    testMismatchedParallelImpulseAcrossSmallBlocks();
    testParallelThirdsUnity();
    testNestedParallelUnity();
    testReportedLatencyMatchesPlan();
    testMergePreservesDcHalves();
    testSerialWideFilterPassesImpulse();
    testPlanNestingDepthForNestedGraph();
    return 0;
}
