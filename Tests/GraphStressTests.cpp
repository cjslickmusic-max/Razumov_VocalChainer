#include "DspTestHelpers.h"

#include <dsp/graph/FlexGraphPlan.h>
#include <dsp/graph/GraphEngine.h>
#include <dsp/graph/GraphPlanFactory.h>
#include <dsp/graph/MergeDelayPad.h>
#include <dsp/graph/SpectralCompressorNode.h>
#include <params/ModuleParamsRuntime.h>

#include <juce_core/juce_core.h>

#include <cassert>
#include <cmath>
#include <cstdint>

namespace
{

/**
 * Долгий регрессионный прогон: миллионы сэмплов через DSP, без аллокаций в process.
 * Для ещё более длинного прогона увеличьте константы здесь или соберите с -O3.
 */
constexpr int kStressBlocksPerScenario = 400000;
constexpr int kMaxBlockSize = 512;
constexpr int kChannels = 2;
constexpr double kSr = 48000.0;

constexpr int kPlanSwapIterations = 120000;
constexpr int kMergePadChunks = 220000;
constexpr int kSpectralBlocks = 150000;
constexpr int kPhase3Blocks = 15000;

constexpr int kDualEnginePairs = 120000;

uint32_t xorshift32(uint32_t& state) noexcept
{
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    return x;
}

void fillDeterministic(juce::AudioBuffer<float>& buf, uint32_t& rng) noexcept
{
    for (int c = 0; c < buf.getNumChannels(); ++c)
    {
        for (int i = 0; i < buf.getNumSamples(); ++i)
        {
            const uint32_t u = xorshift32(rng);
            const float v = (float(u & 0xFFFFFF) / float(0x800000)) - 1.0f;
            buf.setSample(c, i, v * 0.35f);
        }
    }
}

bool bufferAllFinite(const juce::AudioBuffer<float>& buf) noexcept
{
    for (int c = 0; c < buf.getNumChannels(); ++c)
        for (int i = 0; i < buf.getNumSamples(); ++i)
            if (!std::isfinite((double) buf.getSample(c, i)))
                return false;
    return true;
}

bool peakBelow(const juce::AudioBuffer<float>& buf, float limit) noexcept
{
    for (int c = 0; c < buf.getNumChannels(); ++c)
        for (int i = 0; i < buf.getNumSamples(); ++i)
            if (std::abs(buf.getSample(c, i)) > limit)
                return false;
    return true;
}

int blockSizeForIndex(int i) noexcept
{
    const int step = 32 + (i % 11) * 32;
    return juce::jlimit(32, kMaxBlockSize, step);
}

void stressOnePlan(const razumov::graph::FlexSegmentDesc& desc)
{
    using namespace razumov::graph;
    auto u = GraphPlanFactory::makePlanFromDesc(desc);
    std::shared_ptr<FlexGraphPlan> plan(std::move(u));
    GraphEngine engine;
    engine.prepare(kSr, kMaxBlockSize, kChannels);
    engine.submitPlan(plan);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;

    uint32_t rng = 0xC001D00Du;
    juce::AudioBuffer<float> buf(kChannels, kMaxBlockSize);

    for (int i = 0; i < kStressBlocksPerScenario; ++i)
    {
        const int n = blockSizeForIndex(i);
        buf.setSize(kChannels, n, false, false, true);
        fillDeterministic(buf, rng);
        engine.process(buf, macros, mod);
        assert(bufferAllFinite(buf));
        assert(peakBelow(buf, 1.0e6f));
    }
}

void stressGraphEnginesLongRun()
{
    using namespace razumov::graph;

    stressOnePlan(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    stressOnePlan(GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests());
    stressOnePlan(GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests());
    stressOnePlan(GraphPlanFactory::makeParallelHalvesDesc());
    stressOnePlan(GraphPlanFactory::makeParallelThirdsDesc());
}

void stressDualEnginesBitIdenticalStream()
{
    using namespace razumov::graph;
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    auto plan = std::shared_ptr<FlexGraphPlan>(std::move(u));

    GraphEngine a;
    GraphEngine b;
    a.prepare(kSr, kMaxBlockSize, kChannels);
    b.prepare(kSr, kMaxBlockSize, kChannels);
    a.submitPlan(plan);
    b.submitPlan(plan);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;

    uint32_t rng = 0xDEADBEEFu;
    juce::AudioBuffer<float> bufA(kChannels, kMaxBlockSize);
    juce::AudioBuffer<float> bufB(kChannels, kMaxBlockSize);

    for (int i = 0; i < kDualEnginePairs; ++i)
    {
        const int n = blockSizeForIndex(i * 7);
        bufA.setSize(kChannels, n, false, false, true);
        bufB.setSize(kChannels, n, false, false, true);
        fillDeterministic(bufA, rng);
        razumov::tests::copyBuffer(bufA, bufB);
        a.process(bufA, macros, mod);
        b.process(bufB, macros, mod);
        assert(razumov::tests::buffersExactlyEqual(bufA, bufB));
    }
}

void stressAlternatingPlansNoCrash()
{
    using namespace razumov::graph;
    auto ua = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelHalvesDesc());
    auto ub = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeParallelMismatchedLatencyDescForTests());
    auto planA = std::shared_ptr<FlexGraphPlan>(std::move(ua));
    auto planB = std::shared_ptr<FlexGraphPlan>(std::move(ub));

    GraphEngine engine;
    engine.prepare(kSr, 256, 1);
    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;

    juce::AudioBuffer<float> buf(1, 256);
    uint32_t rng = 0xA11CEu;

    for (int i = 0; i < kPlanSwapIterations; ++i)
    {
        engine.submitPlan((i & 1) == 0 ? planA : planB);
        fillDeterministic(buf, rng);
        engine.process(buf, macros, mod);
        assert(bufferAllFinite(buf));
    }
}

void stressMergeDelayPadManyChunks()
{
    using namespace razumov::graph;
    MergeDelayPad pad;
    pad.prepare(2, kMaxBlockSize, kMaxBlockSize * 4);
    pad.setDelaySamples(111);

    juce::AudioBuffer<float> buf(2, 64);
    uint32_t rng = 0xBEEF00Du;

    for (int i = 0; i < kMergePadChunks; ++i)
    {
        fillDeterministic(buf, rng);
        pad.process(buf);
        assert(bufferAllFinite(buf));
    }
}

void stressSpectralCompressorManyBlocks()
{
    using namespace razumov::graph;
    SpectralCompressorNode node;
    node.setMix(0.55f);
    node.setThresholdDb(-28.0f);
    node.setRatio(2.5f);
    node.prepare(kSr, kMaxBlockSize, kChannels);

    juce::AudioBuffer<float> buf(kChannels, kMaxBlockSize);
    uint32_t rng = 0x51EC7u;

    for (int i = 0; i < kSpectralBlocks; ++i)
    {
        fillDeterministic(buf, rng);
        node.process(buf);
        assert(bufferAllFinite(buf));
        assert(peakBelow(buf, 1.0e6f));
    }
}

void stressDefaultPhase3Chain()
{
    using namespace razumov::graph;
    auto u = GraphPlanFactory::makePlanFromDesc(GraphPlanFactory::makeDefaultVocalChainPhase3Desc(kSr));
    auto plan = std::shared_ptr<FlexGraphPlan>(std::move(u));

    GraphEngine engine;
    engine.prepare(kSr, kMaxBlockSize, kChannels);
    engine.submitPlan(plan);

    razumov::params::MacroAudioState macros;
    razumov::params::ModuleParamsRuntime mod;

    juce::AudioBuffer<float> buf(kChannels, kMaxBlockSize);
    uint32_t rng = 0xFACADEu;

    for (int i = 0; i < kPhase3Blocks; ++i)
    {
        fillDeterministic(buf, rng);
        engine.process(buf, macros, mod);
        assert(bufferAllFinite(buf));
        assert(peakBelow(buf, 1.0e7f));
    }
}

} // namespace

void runGraphStressTests()
{
    stressGraphEnginesLongRun();
    stressDualEnginesBitIdenticalStream();
    stressAlternatingPlansNoCrash();
    stressMergeDelayPadManyChunks();
    stressSpectralCompressorManyBlocks();
    stressDefaultPhase3Chain();
}
