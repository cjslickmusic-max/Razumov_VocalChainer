#include "FilterNode.h"

#include <cmath>

namespace razumov::graph
{

void FilterNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(numChannels);
    sampleRate_ = sampleRate;
    smoothedCutoffHz_ = targetCutoffHz_;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax(1, maxBlockSize), 2 };
    iir_.prepare(spec);
    updateCoeffs(smoothedCutoffHz_);
    reset();
}

void FilterNode::reset()
{
    iir_.reset();
    smoothedCutoffHz_ = targetCutoffHz_;
}

void FilterNode::process(juce::AudioBuffer<float>& buffer)
{
    const float nyquist = (float) (sampleRate_ * 0.45);
    const int n = buffer.getNumSamples();
    if (n <= 0)
        return;

    const float dt = (float) n / (float) sampleRate_;
    constexpr float tauSec = 0.05f;
    const float alpha = 1.0f - std::exp(-dt / tauSec);
    smoothedCutoffHz_ += (targetCutoffHz_ - smoothedCutoffHz_) * alpha;

    if (smoothedCutoffHz_ >= nyquist)
        return;

    updateCoeffs(smoothedCutoffHz_);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    iir_.process(ctx);
}

void FilterNode::updateCoeffs(float cutoffHz)
{
    const float sr = (float) sampleRate_;
    const float nyquist = sr * 0.45f;
    const float fc = juce::jmin(cutoffHz, nyquist);

    auto coeffs = Coefficients::makeLowPass(sr, fc, 0.707f);
    *iir_.state = *coeffs;
}

} // namespace razumov::graph
