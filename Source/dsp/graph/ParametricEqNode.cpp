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
    smoothStep();
    for (int b = 0; b < tgtActiveCount_; ++b)
        updateBandCoeffs(b);
}

void ParametricEqNode::reset()
{
    spectrumTap_.reset();
    for (auto& band : bands_)
        for (auto& f : band)
            f.reset();

    /** Align smoothing state with targets so graph rebuild / reset is deterministic. */
    smoothFreq_ = tgtFreq_;
    smoothGainDb_ = tgtGainDb_;
    smoothQ_ = tgtQ_;
    for (int i = 0; i < kNumBands; ++i)
        smoothType_[(size_t) i] = tgtType_[(size_t) i];
    smoothBypass_ = tgtBypass_;

    coeffSnapshotValid_ = false;
    prevBypass_ = tgtBypass_;
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
    constexpr float epsF = 2e-3f;
    constexpr float epsG = 2e-4f;
    constexpr float epsQ = 2e-4f;

    if (coeffSnapshotValid_)
    {
        if (std::abs(smoothFreq_[bi] - lastCoeffFreq_[bi]) < epsF
            && std::abs(smoothGainDb_[bi] - lastCoeffGainDb_[bi]) < epsG
            && std::abs(smoothQ_[bi] - lastCoeffQ_[bi]) < epsQ && smoothType_[bi] == lastCoeffType_[bi])
            return;
    }

    const auto t = razumov::dsp::eq::EqTypeFromFloat(smoothType_[bi]);
    auto c = razumov::dsp::eq::makeBandCoeffs(t, sampleRate_, smoothFreq_[bi], smoothGainDb_[bi], smoothQ_[bi]);
    for (int ch = 0; ch < 2; ++ch)
        bands_[bi][(size_t) ch].coefficients = c;

    lastCoeffFreq_[bi] = smoothFreq_[bi];
    lastCoeffGainDb_[bi] = smoothGainDb_[bi];
    lastCoeffQ_[bi] = smoothQ_[bi];
    lastCoeffType_[bi] = smoothType_[bi];
    coeffSnapshotValid_ = true;
}

void ParametricEqNode::processOneChannel(float* data, int numSamples, int channelIndex) noexcept
{
    if (data == nullptr || numSamples <= 0 || channelIndex < 0 || channelIndex > 1)
        return;

    float* chans[1] = { data };
    juce::dsp::AudioBlock<float> block(chans, 1, (size_t) numSamples);
    for (int b = 0; b < tgtActiveCount_; ++b)
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

    if (smoothBypass_)
    {
        prevBypass_ = true;
        return;
    }

    if (prevBypass_)
    {
        coeffSnapshotValid_ = false;
        prevBypass_ = false;
    }

    for (int b = 0; b < tgtActiveCount_; ++b)
        updateBandCoeffs(b);

    processOneChannel(L, n, 0);
    if (ch > 1)
        processOneChannel(R, n, 1);
}

void ParametricEqNode::applyPhase3(const razumov::params::Phase3RealtimeParams& p) noexcept
{
    const int newCount = juce::jlimit(0, kNumBands, (int) std::lround(p.eqActiveBandCount));
    if (newCount != tgtActiveCount_)
        coeffSnapshotValid_ = false;
    tgtActiveCount_ = newCount;

    tgtBypass_ = p.eqBypass;
    tgtFreq_[0] = p.eqBand1FreqHz;
    tgtFreq_[1] = p.eqBand2FreqHz;
    tgtFreq_[2] = p.eqBand3FreqHz;
    tgtFreq_[3] = p.eqBand4FreqHz;
    tgtFreq_[4] = p.eqBand5FreqHz;
    tgtFreq_[5] = p.eqBand6FreqHz;
    tgtFreq_[6] = p.eqBand7FreqHz;
    tgtFreq_[7] = p.eqBand8FreqHz;
    tgtFreq_[8] = p.eqBand9FreqHz;
    tgtFreq_[9] = p.eqBand10FreqHz;
    tgtGainDb_[0] = p.eqBand1GainDb;
    tgtGainDb_[1] = p.eqBand2GainDb;
    tgtGainDb_[2] = p.eqBand3GainDb;
    tgtGainDb_[3] = p.eqBand4GainDb;
    tgtGainDb_[4] = p.eqBand5GainDb;
    tgtGainDb_[5] = p.eqBand6GainDb;
    tgtGainDb_[6] = p.eqBand7GainDb;
    tgtGainDb_[7] = p.eqBand8GainDb;
    tgtGainDb_[8] = p.eqBand9GainDb;
    tgtGainDb_[9] = p.eqBand10GainDb;
    tgtQ_[0] = p.eqBand1Q;
    tgtQ_[1] = p.eqBand2Q;
    tgtQ_[2] = p.eqBand3Q;
    tgtQ_[3] = p.eqBand4Q;
    tgtQ_[4] = p.eqBand5Q;
    tgtQ_[5] = p.eqBand6Q;
    tgtQ_[6] = p.eqBand7Q;
    tgtQ_[7] = p.eqBand8Q;
    tgtQ_[8] = p.eqBand9Q;
    tgtQ_[9] = p.eqBand10Q;
    tgtType_[0] = p.eqBand1Type;
    tgtType_[1] = p.eqBand2Type;
    tgtType_[2] = p.eqBand3Type;
    tgtType_[3] = p.eqBand4Type;
    tgtType_[4] = p.eqBand5Type;
    tgtType_[5] = p.eqBand6Type;
    tgtType_[6] = p.eqBand7Type;
    tgtType_[7] = p.eqBand8Type;
    tgtType_[8] = p.eqBand9Type;
    tgtType_[9] = p.eqBand10Type;
}

void ParametricEqNode::copySpectrum256(float* dst) const noexcept
{
    spectrumTap_.copyDisplayBins(dst, ISpectrumSource::kSpectrumBins);
}

} // namespace razumov::graph
