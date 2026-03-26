#include "CompressorArchetypeNode.h"

namespace razumov::graph
{

CompressorArchetypeNode::CompressorArchetypeNode(Archetype a)
    : archetype_(a)
{
}

AudioNodeKind CompressorArchetypeNode::getKind() const noexcept
{
    switch (archetype_)
    {
        case Archetype::Opto:
            return AudioNodeKind::OptoCompressor;
        case Archetype::Fet:
            return AudioNodeKind::FetCompressor;
        case Archetype::Vca:
            return AudioNodeKind::VcaCompressor;
    }
    return AudioNodeKind::Unknown;
}

void CompressorArchetypeNode::applyArchetypeTiming()
{
    switch (archetype_)
    {
        case Archetype::Opto:
            comp_.setAttack(10.0f);
            comp_.setRelease(300.0f);
            break;
        case Archetype::Fet:
            comp_.setAttack(0.5f);
            comp_.setRelease(50.0f);
            break;
        case Archetype::Vca:
            comp_.setAttack(5.0f);
            comp_.setRelease(100.0f);
            break;
    }
}

void CompressorArchetypeNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(numChannels);
    sampleRate_ = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax(1, maxBlockSize), 2 };
    comp_.prepare(spec);
    applyArchetypeTiming();
    comp_.setThreshold(thresholdDb_);
    comp_.setRatio(ratio_);
    reset();
}

void CompressorArchetypeNode::reset()
{
    comp_.reset();
}

void CompressorArchetypeNode::process(juce::AudioBuffer<float>& buffer)
{
    comp_.setThreshold(thresholdDb_);
    comp_.setRatio(ratio_);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    comp_.process(ctx);

    const float g = juce::Decibels::decibelsToGain(makeupDb_);
    if (g != 1.0f)
        buffer.applyGain(g);
}

} // namespace razumov::graph
