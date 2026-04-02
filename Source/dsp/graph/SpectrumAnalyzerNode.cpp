#include "SpectrumAnalyzerNode.h"

namespace razumov::graph
{

void SpectrumAnalyzerNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(numChannels);
    spectrumTap_.prepare(sampleRate, maxBlockSize);
    reset();
}

void SpectrumAnalyzerNode::reset()
{
    spectrumTap_.reset();
}

void SpectrumAnalyzerNode::process(juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    if (n <= 0)
        return;

    const float* L = buffer.getReadPointer(0);
    const float* R = ch > 1 ? buffer.getReadPointer(1) : L;
    spectrumTap_.pushStereoBlock(L, R, n);
}

void SpectrumAnalyzerNode::copySpectrum256(float* dst) const noexcept
{
    spectrumTap_.copyDisplayBins(dst, ISpectrumSource::kSpectrumBins);
}

} // namespace razumov::graph
