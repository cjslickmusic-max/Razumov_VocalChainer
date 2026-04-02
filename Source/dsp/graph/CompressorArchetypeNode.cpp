#include "CompressorArchetypeNode.h"

#include <cmath>

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
    smoothedGrDb_ = 0.f;
    grMeterDbAtomic_.store(0.f, std::memory_order_relaxed);
}

float CompressorArchetypeNode::getGainReductionDbForUi() const noexcept
{
    return grMeterDbAtomic_.load(std::memory_order_relaxed);
}

void CompressorArchetypeNode::process(juce::AudioBuffer<float>& buffer)
{
    comp_.setThreshold(thresholdDb_);
    comp_.setRatio(ratio_);

    const int n = buffer.getNumSamples();
    const int ch = juce::jmin(buffer.getNumChannels(), 2);
    if (n <= 0 || ch <= 0)
        return;

    float blockPeakGrDb = 0.f;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float in = buffer.getSample(c, i);
            const float out = comp_.processSample(c, in);
            buffer.setSample(c, i, out);

            const float ain = std::abs(in);
            if (ain > 1.0e-12f)
            {
                const float g = std::abs(out) / ain;
                if (g < 1.0f)
                {
                    const float grDb = -20.0f * std::log10(juce::jmax(g, 1.0e-8f));
                    blockPeakGrDb = juce::jmax(blockPeakGrDb, grDb);
                }
            }
        }
    }

    const float dt = (float) n / (float) juce::jmax(1.0, sampleRate_);
    constexpr float releaseSec = 0.35f;
    const float relCoeff = std::exp(-dt / releaseSec);

    if (blockPeakGrDb >= smoothedGrDb_)
        smoothedGrDb_ = blockPeakGrDb;
    else
        smoothedGrDb_ = blockPeakGrDb + (smoothedGrDb_ - blockPeakGrDb) * relCoeff;

    grMeterDbAtomic_.store(smoothedGrDb_, std::memory_order_relaxed);

    const float makeupG = juce::Decibels::decibelsToGain(makeupDb_);
    if (makeupG != 1.0f)
        buffer.applyGain(makeupG);
}

} // namespace razumov::graph
