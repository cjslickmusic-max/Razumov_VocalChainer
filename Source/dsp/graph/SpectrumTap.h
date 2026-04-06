#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include <juce_dsp/juce_dsp.h>

#include "ISpectrumSource.h"

namespace razumov::graph
{

/**
 * FFT-тап для отображения спектра (Reaper/ReJJ-style analyzer path).
 * Без аллокаций в process: буферы в prepare.
 */
class SpectrumTap
{
public:
    static constexpr int kDisplayBins = ISpectrumSource::kSpectrumBins;
    /** FFT size = 2^kFftOrder (8192): finer freq bins for harmonic detail vs 4096. */
    static constexpr int kFftOrder = 13;
    static constexpr int kFftSize = 1 << kFftOrder;

    /** Log-spaced analyzer range (Hz); must match ReEq frequency axis (Kirchhoff-style 10...30k). */
    static constexpr float kAnalyzerHzMin = 10.f;
    static constexpr float kAnalyzerHzMax = 30000.f;

    void prepare(double sampleRate, int maxBlockHint);
    void reset() noexcept;

    /** Моно (L+R)/2, Hann, FFT, log-frequency bins, dBFS-ish scale -> kDisplayBins x 0...1. */
    void pushStereoBlock(const float* L, const float* R, int numSamples) noexcept;

    void copyDisplayBins(float* dst, int n) const noexcept;

private:
    double sampleRate_ { 44100.0 };
    int fftOrder_ { kFftOrder };
    int fftSize_ { kFftSize };

    juce::dsp::FFT fft_ { kFftOrder };

    std::vector<float> window_;
    std::vector<juce::dsp::Complex<float>> time_;
    std::vector<juce::dsp::Complex<float>> freq_;

    std::array<std::atomic<float>, kDisplayBins> bins_ {};
};

} // namespace razumov::graph
