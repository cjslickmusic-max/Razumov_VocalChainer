#include "ParametricEqNode.h"

#include "params/Phase3RealtimeParams.h"

#include <cmath>

namespace razumov::graph
{

void ParametricEqNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = sampleRate;
    maxBlockSize_ = juce::jmax(1, maxBlockSize);
    juce::ignoreUnused(numChannels);

    spectrumTap_.prepare(sampleRate, maxBlockSize);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
    for (auto& band : bands_)
        for (auto& f : band)
            f.prepare(spec);

    reset();
    for (int i = 0; i < kNumBands; ++i)
        smoothType_[(size_t) i] = tgtType_[(size_t) i];
    smoothStep();
    for (int b = 0; b < kNumBands; ++b)
        updateBandCoeffs(b);
}

void ParametricEqNode::reset()
{
    spectrumTap_.reset();
    for (auto& band : bands_)
        for (auto& f : band)
            f.reset();
}

void ParametricEqNode::smoothStep() noexcept
{
    const float dt = (float) maxBlockSize_ / (float) juce::jmax(1.0, sampleRate_);
    constexpr float tau = 0.045f;
    const float a = 1.f - std::exp(-dt / tau);
    for (int i = 0; i < kNumBands; ++i)
    {
        const size_t si = (size_t) i;
        smoothFreq_[si] += (tgtFreq_[si] - smoothFreq_[si]) * a;
        smoothGainDb_[si] += (tgtGainDb_[si] - smoothGainDb_[si]) * a;
        smoothQ_[si] += (tgtQ_[si] - smoothQ_[si]) * a;
        smoothType_[si] = tgtType_[si];
    }
    smoothBypass_ = tgtBypass_;
}

void ParametricEqNode::updateBandCoeffs(int bandIndex) noexcept
{
    if (bandIndex < 0 || bandIndex >= kNumBands)
        return;

    const size_t bi = (size_t) bandIndex;
    const auto t = razumov::dsp::eq::EqTypeFromFloat(smoothType_[bi]);
    auto c = razumov::dsp::eq::makeBandCoeffs(t, sampleRate_, smoothFreq_[bi], smoothGainDb_[bi], smoothQ_[bi]);
    for (int ch = 0; ch < 2; ++ch)
        bands_[bi][(size_t) ch].coefficients = c;
}

void ParametricEqNode::processOneChannel(float* data, int numSamples, int channelIndex) noexcept
{
    if (data == nullptr || numSamples <= 0 || channelIndex < 0 || channelIndex > 1)
        return;

    float* channelData = data;
    juce::AudioBuffer<float> wrap(&channelData, 1, numSamples);
    juce::dsp::AudioBlock<float> block(wrap);
    for (int b = 0; b < kNumBands; ++b)
    {
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        bands_[(size_t) b][(size_t) channelIndex].process(ctx);
    }
}

void ParametricEqNode::process(juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    float* L = buffer.getWritePointer(0);
    float* R = ch > 1 ? buffer.getWritePointer(1) : L;

    spectrumTap_.pushStereoBlock(L, R, n);

    smoothStep();
    for (int b = 0; b < kNumBands; ++b)
        updateBandCoeffs(b);

    if (smoothBypass_)
        return;

    processOneChannel(L, n, 0);
    if (ch > 1)
        processOneChannel(R, n, 1);
}

void ParametricEqNode::applyPhase3(const razumov::params::Phase3RealtimeParams& p) noexcept
{
    tgtBypass_ = p.eqBypass;
    tgtFreq_[0] = p.eqBand1FreqHz;
    tgtFreq_[1] = p.eqBand2FreqHz;
    tgtFreq_[2] = p.eqBand3FreqHz;
    tgtFreq_[3] = p.eqBand4FreqHz;
    tgtFreq_[4] = p.eqBand5FreqHz;
    tgtGainDb_[0] = p.eqBand1GainDb;
    tgtGainDb_[1] = p.eqBand2GainDb;
    tgtGainDb_[2] = p.eqBand3GainDb;
    tgtGainDb_[3] = p.eqBand4GainDb;
    tgtGainDb_[4] = p.eqBand5GainDb;
    tgtQ_[0] = p.eqBand1Q;
    tgtQ_[1] = p.eqBand2Q;
    tgtQ_[2] = p.eqBand3Q;
    tgtQ_[3] = p.eqBand4Q;
    tgtQ_[4] = p.eqBand5Q;
    tgtType_[0] = p.eqBand1Type;
    tgtType_[1] = p.eqBand2Type;
    tgtType_[2] = p.eqBand3Type;
    tgtType_[3] = p.eqBand4Type;
    tgtType_[4] = p.eqBand5Type;
}

void ParametricEqNode::copySpectrum256(float* dst) const noexcept
{
    spectrumTap_.copyDisplayBins(dst, ISpectrumSource::kSpectrumBins);
}

} // namespace razumov::graph
