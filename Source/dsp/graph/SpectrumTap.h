#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/**
 * FFT-тап для отображения спектра (Reaper/ReJJ-style analyzer path).
 * Без аллокаций в process: буферы в prepare.
 */
class SpectrumTap
{
public:
    static constexpr int kDisplayBins = 256;
    /** FFT size = 2^kFftOrder (2048): finer frequency resolution than 1024 for smoother display). */
    static constexpr int kFftOrder = 11;
    static constexpr int kFftSize = 1 << kFftOrder;

    void prepare(double sampleRate, int maxBlockHint);
    void reset() noexcept;

    /** Моно (L+R)/2, окно Ханна, FFT, магнитуда -> 256 бинов в dB, нормализация в 0...1. */
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
