#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <cassert>
#include <cmath>
#include <cstring>

namespace razumov::tests
{

inline bool nearAbs(float a, float b, float eps) noexcept
{
    return std::abs(a - b) <= eps;
}

/** Побитово одинаковые сэмплы (для детерминизма без сглаживания). */
inline bool buffersExactlyEqual(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b) noexcept
{
    if (a.getNumChannels() != b.getNumChannels() || a.getNumSamples() != b.getNumSamples())
        return false;
    for (int c = 0; c < a.getNumChannels(); ++c)
    {
        if (std::memcmp(a.getReadPointer(c), b.getReadPointer(c), (size_t) a.getNumSamples() * sizeof(float)) != 0)
            return false;
    }
    return true;
}

inline void assertBuffersNearEqual(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b, float eps)
{
    assert(a.getNumChannels() == b.getNumChannels());
    assert(a.getNumSamples() == b.getNumSamples());
    for (int c = 0; c < a.getNumChannels(); ++c)
        for (int i = 0; i < a.getNumSamples(); ++i)
            assert(nearAbs(a.getSample(c, i), b.getSample(c, i), eps));
}

inline void copyBuffer(const juce::AudioBuffer<float>& src, juce::AudioBuffer<float>& dst)
{
    dst.setSize(src.getNumChannels(), src.getNumSamples(), false, false, true);
    for (int c = 0; c < src.getNumChannels(); ++c)
        dst.copyFrom(c, 0, src, c, 0, src.getNumSamples());
}

/** Детерминированный синус (одинаковые входы на всех платформах при тех же параметрах). */
inline void fillSine(juce::AudioBuffer<float>& buf, double sampleRate, double hz, float amp) noexcept
{
    const int n = buf.getNumSamples();
    const double w = juce::MathConstants<double>::twoPi * hz / sampleRate;
    for (int c = 0; c < buf.getNumChannels(); ++c)
        for (int i = 0; i < n; ++i)
            buf.setSample(c, i, amp * (float) std::sin(w * (double) i));
}

/** Сумма выборок (для DC / нулевого теста). */
inline float sumSamples(const juce::AudioBuffer<float>& buf, int channel) noexcept
{
    float s = 0.0f;
    for (int i = 0; i < buf.getNumSamples(); ++i)
        s += buf.getSample(channel, i);
    return s;
}

} // namespace razumov::tests
