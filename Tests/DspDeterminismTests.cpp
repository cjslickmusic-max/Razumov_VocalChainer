#include "DspTestHelpers.h"

#include <dsp/graph/CompressorArchetypeNode.h>
#include <dsp/graph/DeesserNode.h>
#include <dsp/graph/ExciterNode.h>
#include <dsp/graph/FilterNode.h>
#include <dsp/graph/GainNode.h>
#include <dsp/graph/LatencyNode.h>
#include <dsp/graph/MergeDelayPad.h>
#include <dsp/graph/MicCorrectionNode.h>

#include <cassert>

namespace
{

constexpr double kSr = 48000.0;
constexpr int kBlock = 512;

void testCompressorSilenceStaysNearZero(razumov::graph::CompressorArchetypeNode::Archetype arch)
{
    using namespace razumov::graph;
    CompressorArchetypeNode node(arch);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> buf(2, kBlock);
    buf.clear();
    node.process(buf);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < kBlock; ++i)
            assert(razumov::tests::nearAbs(buf.getSample(c, i), 0.0f, 1.0e-4f));
}

void testCompressorDeterminism(razumov::graph::CompressorArchetypeNode::Archetype arch)
{
    using namespace razumov::graph;
    CompressorArchetypeNode node(arch);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 123.45, 0.2f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-5f);
}

void testLatencyDeterminism()
{
    using namespace razumov::graph;
    LatencyNode node(47);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 200.0, 0.15f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-6f);
}

void testLatencySineShiftPreservesPhaseRelation()
{
    using namespace razumov::graph;
    const int delay = 64;
    LatencyNode node(delay);
    node.prepare(kSr, kBlock, 1);
    juce::AudioBuffer<float> buf(1, kBlock);
    razumov::tests::fillSine(buf, kSr, 1000.0, 0.25f);
    juce::AudioBuffer<float> ref(1, kBlock);
    razumov::tests::copyBuffer(buf, ref);
    node.process(buf);

    for (int i = delay; i < kBlock; ++i)
    {
        const float expected = ref.getSample(0, i - delay);
        const float actual = buf.getSample(0, i);
        assert(razumov::tests::nearAbs(expected, actual, 1.0e-5f));
    }
}

void testGainDeterminism()
{
    using namespace razumov::graph;
    GainNode node;
    node.setLinearGain(0.75f);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 440.0, 0.1f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-6f);
}

void testFilterDeterminism()
{
    using namespace razumov::graph;
    FilterNode node;
    node.setCutoffHz(8000.0f);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 880.0, 0.18f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-4f);
}

void testExciterDeterminism()
{
    using namespace razumov::graph;
    ExciterNode node;
    node.setDrive(1.2f);
    node.setMix(0.3f);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 2000.0, 0.05f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-6f);
}

void testDeesserDeterminism()
{
    using namespace razumov::graph;
    DeesserNode node;
    node.setCrossoverHz(5000.0f);
    node.setThresholdDb(-24.0f);
    node.setRatio(3.0f);
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 3000.0, 0.12f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-4f);
}

void testMicPassthroughDeterminism()
{
    using namespace razumov::graph;
    MicCorrectionNode node;
    node.prepare(kSr, kBlock, 2);
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 500.0, 0.11f);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    node.process(a);

    node.reset();
    razumov::tests::copyBuffer(in, b);
    node.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-7f);
}

void testMergeDelayPadDeterminism()
{
    using namespace razumov::graph;
    juce::AudioBuffer<float> in(2, kBlock);
    razumov::tests::fillSine(in, kSr, 150.0, 0.09f);

    MergeDelayPad padA;
    MergeDelayPad padB;
    padA.prepare(2, kBlock, 200);
    padB.prepare(2, kBlock, 200);
    padA.setDelaySamples(99);
    padB.setDelaySamples(99);

    juce::AudioBuffer<float> a(2, kBlock);
    juce::AudioBuffer<float> b(2, kBlock);
    razumov::tests::copyBuffer(in, a);
    razumov::tests::copyBuffer(in, b);
    padA.process(a);
    padB.process(b);

    razumov::tests::assertBuffersNearEqual(a, b, 1.0e-6f);
}

/** Постоянное смещение через Gain: линейное масштабирование DC без «плавающего» уровня. */
void testConstantDcThroughGainLinear()
{
    using namespace razumov::graph;
    GainNode node;
    node.setLinearGain(0.5f);
    node.prepare(kSr, kBlock, 1);
    juce::AudioBuffer<float> buf(1, kBlock);
    for (int i = 0; i < kBlock; ++i)
        buf.setSample(0, i, 0.4f);
    node.process(buf);
    for (int i = 0; i < kBlock; ++i)
        assert(razumov::tests::nearAbs(buf.getSample(0, i), 0.2f, 1.0e-3f));
}

} // namespace

void runDspDeterminismTests()
{
    using razumov::graph::CompressorArchetypeNode;
    testCompressorSilenceStaysNearZero(CompressorArchetypeNode::Archetype::Opto);
    testCompressorSilenceStaysNearZero(CompressorArchetypeNode::Archetype::Fet);
    testCompressorSilenceStaysNearZero(CompressorArchetypeNode::Archetype::Vca);

    testCompressorDeterminism(CompressorArchetypeNode::Archetype::Opto);
    testCompressorDeterminism(CompressorArchetypeNode::Archetype::Fet);
    testCompressorDeterminism(CompressorArchetypeNode::Archetype::Vca);

    testLatencyDeterminism();
    testLatencySineShiftPreservesPhaseRelation();
    testGainDeterminism();
    testFilterDeterminism();
    testExciterDeterminism();
    testDeesserDeterminism();
    testMicPassthroughDeterminism();
    testMergeDelayPadDeterminism();
    testConstantDcThroughGainLinear();
}
