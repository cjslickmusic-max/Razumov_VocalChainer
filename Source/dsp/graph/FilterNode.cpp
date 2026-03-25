#include "FilterNode.h"

namespace razumov::graph
{

void FilterNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(numChannels);
    sampleRate_ = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax(1, maxBlockSize), 2 };
    iir_.prepare(spec);
    updateCoeffs();
    reset();
}

void FilterNode::reset()
{
    iir_.reset();
}

void FilterNode::process(juce::AudioBuffer<float>& buffer)
{
    const float nyquist = (float) (sampleRate_ * 0.45);
    if (cutoffHz_ >= nyquist)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    iir_.process(ctx);
}

void FilterNode::updateCoeffs()
{
    const float sr = (float) sampleRate_;
    const float nyquist = sr * 0.45f;
    const float fc = juce::jmin(cutoffHz_, nyquist);

    auto coeffs = Coefficients::makeLowPass(sr, fc, 0.707f);
    *iir_.state = *coeffs;
}

} // namespace razumov::graph
