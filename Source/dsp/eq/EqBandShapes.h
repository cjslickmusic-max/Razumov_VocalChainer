#pragma once

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace razumov::dsp::eq
{

/** ReEQ-style band kinds (subset; M/S analog paths not mapped). */
enum class EqBandType : uint8_t
{
    Peak = 0,
    LowShelf,
    HighShelf,
    LowPass,
    HighPass,
    Notch
};

inline EqBandType EqTypeFromFloat(float v) noexcept
{
    const int t = (int) std::round(v);
    return (EqBandType) juce::jlimit(0, 5, t);
}

inline float EqTypeToFloat(EqBandType t) noexcept
{
    return (float) (int) t;
}

using Coeffs = juce::dsp::IIR::Coefficients<float>;

/** RBJ biquads for stereo-linked EQ; same formulas as ParametricEqNode.
    JUCE makePeakFilter / makeLowShelf / makeHighShelf expect linear gain (1.0 = 0 dB), not dB. */
inline Coeffs::Ptr makeBandCoeffs(EqBandType t, double sampleRate, float fcHz, float gainDb, float q) noexcept
{
    const float sr = (float) sampleRate;
    const float nyq = sr * 0.48f;
    const float fc = juce::jlimit(20.f, nyq, fcHz);
    const float qq = juce::jlimit(0.3f, 20.f, q);
    const double srd = sampleRate;
    const float gainLinear = juce::Decibels::decibelsToGain(gainDb);

    switch (t)
    {
        case EqBandType::Peak:
            return Coeffs::makePeakFilter(srd, (double) fc, (double) qq, (double) gainLinear);
        case EqBandType::LowShelf:
            return Coeffs::makeLowShelf(srd, (double) fc, (double) qq, (double) gainLinear);
        case EqBandType::HighShelf:
            return Coeffs::makeHighShelf(srd, (double) fc, (double) qq, (double) gainLinear);
        case EqBandType::LowPass:
            return Coeffs::makeLowPass(srd, (double) fc, (double) qq);
        case EqBandType::HighPass:
            return Coeffs::makeHighPass(srd, (double) fc, (double) qq);
        case EqBandType::Notch:
            return Coeffs::makeNotch(srd, (double) fc, (double) qq);
        default:
            return Coeffs::makePeakFilter(srd, (double) fc, (double) qq, (double) gainLinear);
    }
}

/** How many 2nd-order (12 dB/oct) stages to approximate slope for LP/HP (0..96 dB/oct). */
inline int countSlopeStagesForLpHp(float slopeDbPerOct) noexcept
{
    if (slopeDbPerOct <= 0.f)
        return 0;
    return juce::jmin(8, (int) std::ceil((double) slopeDbPerOct / 12.0));
}

/**
 * Fill up to 8 biquad stages. LP/HP: cascaded Butterworth-like 2nd-order sections (Q=0.707 each).
 * Other types: single stage in outStages[0]. Caller must provide outStages[8].
 */
inline void assignStageCoeffs(EqBandType t, double sampleRate, float fcHz, float gainDb, float q,
                              float slopeDbPerOct, Coeffs::Ptr* outStages, int& outNumStages) noexcept
{
    const float sr = (float) sampleRate;
    const float nyq = sr * 0.48f;
    const float fc = juce::jlimit(20.f, nyq, fcHz);
    const float qq = juce::jlimit(0.3f, 20.f, q);
    const double srd = sampleRate;

    switch (t)
    {
        case EqBandType::LowPass:
        {
            outNumStages = countSlopeStagesForLpHp(slopeDbPerOct);
            for (int s = 0; s < outNumStages; ++s)
                outStages[(size_t) s] = Coeffs::makeLowPass(srd, (double) fc, 0.707);
            break;
        }
        case EqBandType::HighPass:
        {
            outNumStages = countSlopeStagesForLpHp(slopeDbPerOct);
            for (int s = 0; s < outNumStages; ++s)
                outStages[(size_t) s] = Coeffs::makeHighPass(srd, (double) fc, 0.707);
            break;
        }
        default:
            outStages[0] = makeBandCoeffs(t, sampleRate, fcHz, gainDb, q);
            outNumStages = 1;
            break;
    }
}

/** Magnitude of one band (possibly multi-stage LP/HP) at hz, in dB. */
inline float bandMagDbAtHz(EqBandType t, double sampleRate, float fcHz, float gainDb, float q,
                           float slopeDbPerOct, double hz) noexcept
{
    Coeffs::Ptr stages[8] = {};
    int n = 0;
    assignStageCoeffs(t, sampleRate, fcHz, gainDb, q, slopeDbPerOct, stages, n);
    if (n <= 0)
        return 0.f;
    double m = 1.0;
    for (int s = 0; s < n; ++s)
    {
        if (stages[(size_t) s] == nullptr)
            continue;
        m *= (double) stages[(size_t) s]->getMagnitudeForFrequency(hz, sampleRate);
    }
    return (float) (20.0 * std::log10(juce::jmax(1.0e-12, m)));
}

/** Sum of magnitude responses in dB (cascade chain); one coeff per band (legacy). */
inline float sumCascadeMagDbAtHz(const Coeffs::Ptr* coeffs, int numBands, double hz, double sampleRate) noexcept
{
    float sumDb = 0.f;
    for (int i = 0; i < numBands; ++i)
    {
        if (coeffs[i] == nullptr)
            continue;
        const double m = coeffs[i]->getMagnitudeForFrequency(hz, sampleRate);
        sumDb += (float) (20.0 * std::log10(juce::jmax(1.0e-12, m)));
    }
    return sumDb;
}

} // namespace razumov::dsp::eq
