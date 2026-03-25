#include <dsp/graph/GraphEngine.h>
#include <dsp/graph/GraphPlanFactory.h>
#include <dsp/graph/GraphPlan.h>

#include <cassert>
#include <cmath>

static void testChainLatencySum()
{
    using namespace razumov::graph;
    auto plan = GraphPlanFactory::makeParallelMismatchedLatencyForTests();
    assert(plan != nullptr);
    const int lat = plan->computePluginLatencySamples();
    assert(lat == 64);
}

static void testParallelHalvesIsUnityGain()
{
    using namespace razumov::graph;
    GraphEngine engine;
    engine.prepare(48000.0, 512, 2);
    engine.submitPlan(GraphPlanFactory::makeParallelHalves());

    juce::AudioBuffer<float> buf(2, 512);
    buf.clear();
    buf.setSample(0, 0, 1.0f);
    buf.setSample(1, 0, 1.0f);

    engine.process(buf);

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
