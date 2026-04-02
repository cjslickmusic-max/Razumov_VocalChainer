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

    void prepare(double sampleRate, int maxBlockHint);
    void reset() noexcept;

    /** Моно (L+R)/2, окно Ханна, FFT, магнитуда -> 256 бинов в dB, нормализация в 0...1. */
    void pushStereoBlock(const float* L, const float* R, int numSamples) noexcept;

    void copyDisplayBins(float* dst, int n) const noexcept;

private:
    double sampleRate_ { 44100.0 };
    int fftOrder_ { 10 };
    int fftSize_ { 1024 };

    juce::dsp::FFT fft_ { 10 };

    std::vector<float> window_;
    std::vector<juce::dsp::Complex<float>> time_;
    std::vector<juce::dsp::Complex<float>> freq_;

    std::array<std::atomic<float>, kDisplayBins> bins_ {};
};

} // namespace razumov::graph
