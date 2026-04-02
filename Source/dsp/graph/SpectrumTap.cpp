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
    for (int b = 0; b < kDisplayBins; ++b)
    {
        const int k0 = b * (half + 1) / kDisplayBins;
        const int k1 = (b + 1) * (half + 1) / kDisplayBins;
        float mx = 0.f;
        for (int k = k0; k < k1 && k <= half; ++k)
        {
            const auto& c = freq_[(size_t) k];
            const float mag = std::sqrt(c.real() * c.real() + c.imag() * c.imag());
            mx = juce::jmax(mx, mag);
        }
        const float db = 20.0f * std::log10(mx + 1.0e-15f);
        const float norm = juce::jlimit(0.f, 1.f, (db + 90.f) / 90.f);
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
