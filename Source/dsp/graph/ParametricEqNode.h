#pragma once

#include "AudioNode.h"
#include "ISpectrumSource.h"
#include "SpectrumTap.h"
#include "dsp/eq/EqBandShapes.h"

#include <array>
#include <juce_dsp/juce_dsp.h>

namespace razumov::params
{
struct Phase3RealtimeParams;
}

namespace razumov::graph
{

/** Up to 10 bands (stereo linked), RBJ-style per-band types; spectrum taps: in pre-EQ, out post-EQ.
    LP/HP: up to 8 cascaded 2nd-order sections from Slope (dB/oct). */
class ParametricEqNode final : public AudioNode, public ISpectrumSource
{
public:
    static constexpr int kNumBands = 10;
    static constexpr int kMaxStagesPerBand = 8;

    ParametricEqNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::ParametricEq; }

    ISpectrumSource* asSpectrumSource() noexcept override { return this; }

    ParametricEqNode* asParametricEq() noexcept override { return this; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void applyPhase3(const razumov::params::Phase3RealtimeParams& p) noexcept;

    /** ISpectrumSource: post-EQ (same as copySpectrumOut256). */
    void copySpectrum256(float* dst) const noexcept override;

    void copySpectrumIn256(float* dst) const noexcept;
    void copySpectrumOut256(float* dst) const noexcept;

private:
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void smoothStep() noexcept;
    void updateBandCoeffs(int bandIndex) noexcept;
    void processOneChannel(float* data, int numSamples, int channelIndex) noexcept;

    double sampleRate_ { 44100.0 };
    int maxBlockSize_ { 512 };

    SpectrumTap spectrumTapIn_;
    SpectrumTap spectrumTapOut_;

    /** [band][stage 0..7][L/R] */
    std::array<std::array<std::array<IIRFilter, 2>, kMaxStagesPerBand>, kNumBands> bands_ {};

    std::array<float, kNumBands> tgtFreq_ { 120.f, 400.f, 2500.f, 7000.f, 10000.f, 12000.f, 13000.f, 15000.f, 16000.f, 18000.f };
    std::array<float, kNumBands> tgtGainDb_ {};
    std::array<float, kNumBands> tgtQ_ { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
    std::array<float, kNumBands> tgtType_ {}; /** 0..5 EqBandType */
    /** 0..96 dB/oct; used for LowPass/HighPass cascade. */
    std::array<float, kNumBands> tgtSlopeDbPerOct_ { 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f };
    bool tgtBypass_ { false };
    int tgtActiveCount_ { 0 };

    std::array<float, kNumBands> smoothFreq_ { 120.f, 400.f, 2500.f, 7000.f, 10000.f, 12000.f, 13000.f, 15000.f, 16000.f, 18000.f };
    std::array<float, kNumBands> smoothGainDb_ {};
    std::array<float, kNumBands> smoothQ_ { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
    std::array<float, kNumBands> smoothType_ {};
    std::array<float, kNumBands> smoothSlopeDbPerOct_ { 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f };
    bool smoothBypass_ { false };

    std::array<int, kNumBands> effectiveStagesPerBand_ {};

    /** Last coeffs pushed to filters (skip rebuild when unchanged; RT-safe). */
    std::array<float, kNumBands> lastCoeffFreq_{};
    std::array<float, kNumBands> lastCoeffGainDb_{};
    std::array<float, kNumBands> lastCoeffQ_{};
    std::array<float, kNumBands> lastCoeffType_{};
    std::array<float, kNumBands> lastCoeffSlope_{};
    bool coeffSnapshotValid_ { false };
    bool prevBypass_ { false };
};

} // namespace razumov::graph
