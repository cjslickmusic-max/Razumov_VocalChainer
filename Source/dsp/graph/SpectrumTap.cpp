#include "SpectrumTap.h"

#include <cmath>

namespace razumov::graph
{

void SpectrumTap::prepare(double sampleRate, int maxBlockHint)
{
    juce::ignoreUnused(maxBlockHint);
    sampleRate_ = sampleRate;
    fftSize_ = 1 << fftOrder_;
    window_.assign((size_t) fftSize_, 0.f);
    time_.assign((size_t) fftSize_, {});
    freq_.assign((size_t) fftSize_, {});
    for (int i = 0; i < fftSize_; ++i)
    {
        window_[(size_t) i] =
            0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (float) i / (float) juce::jmax(1, fftSize_ - 1)));
    }
    reset();
}

void SpectrumTap::reset() noexcept
{
    for (auto& b : bins_)
        b.store(0.f, std::memory_order_relaxed);
}

void SpectrumTap::pushStereoBlock(const float* L, const float* R, int numSamples) noexcept
{
    if (L == nullptr || R == nullptr || numSamples <= 0)
        return;

    const int take = juce::jmin(numSamples, fftSize_);
    for (int i = 0; i < take; ++i)
    {
        const float m = (L[i] + R[i]) * 0.5f;
        time_[(size_t) i] = { m * window_[(size_t) i], 0.f };
    }
    for (int i = take; i < fftSize_; ++i)
        time_[(size_t) i] = {};

    fft_.perform(time_.data(), freq_.data(), false);

    const int half = fftSize_ / 2;
    const float sr = (float) sampleRate_;
    const float hzTop = juce::jmin(kAnalyzerHzMax, sr * 0.499f);
    const float hzMin = kAnalyzerHzMin;
    const float ratio = hzTop / hzMin;

    for (int b = 0; b < kDisplayBins; ++b)
    {
        const float t0 = (float) b / (float) kDisplayBins;
        const float t1 = (float) (b + 1) / (float) kDisplayBins;
        const float hz0 = hzMin * std::pow(ratio, t0);
        const float hz1 = hzMin * std::pow(ratio, t1);
        int k0 = (int) std::floor(hz0 * (float) fftSize_ / sr);
        int k1 = (int) std::ceil(hz1 * (float) fftSize_ / sr);
        k0 = juce::jmax(1, k0);
        k1 = juce::jmin(half, k1);
        if (k1 < k0)
            k1 = k0;

        float mx = 0.f;
        for (int k = k0; k <= k1; ++k)
        {
            const auto& c = freq_[(size_t) k];
            const float mag = std::sqrt(c.real() * c.real() + c.imag() * c.imag());
            mx = juce::jmax(mx, mag);
        }

        /** Unnormalized FFT grows with N; map to ~amplitude then dB so music sits in -90..0 dB band. */
        const float magLin = mx / (float) fftSize_;
        const float db = 20.0f * std::log10(magLin + 1.0e-15f);
        /** Kirchhoff-style analyzer band: full plot maps ~0...-120 dBFS (norm 0...1). */
        constexpr float floorDb = -120.f;
        constexpr float ceilDb = 0.f;
        const float norm = juce::jlimit(0.f, 1.f, (db - floorDb) / (ceilDb - floorDb));
        bins_[(size_t) b].store(norm, std::memory_order_relaxed);
    }
}

void SpectrumTap::copyDisplayBins(float* dst, int n) const noexcept
{
    if (dst == nullptr || n <= 0)
        return;
    const int m = juce::jmin(n, kDisplayBins);
    for (int i = 0; i < m; ++i)
        dst[i] = bins_[(size_t) i].load(std::memory_order_relaxed);
    for (int i = m; i < n; ++i)
        dst[i] = 0.f;
}

} // namespace razumov::graph
