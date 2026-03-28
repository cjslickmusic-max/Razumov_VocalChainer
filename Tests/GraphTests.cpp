#include <dsp/graph/FlexGraphPlan.h>
#include <dsp/graph/GraphEngine.h>
#include <dsp/graph/GraphPlanFactory.h>
#include <params/ModuleParamsRuntime.h>

#include <cassert>
#include <cmath>
#include <memory>

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
    assert(std::abs(l - 1.0f) < 1.0e-3f);
    assert(std::abs(r - 1.0f) < 1.0e-3f);
}

int main()
{
    testChainLatencySum();
    testParallelHalvesIsUnityGain();
    return 0;
}
