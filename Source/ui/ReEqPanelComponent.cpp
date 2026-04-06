#include "ReEqPanelComponent.h"

#include "PluginProcessor.h"
#include "dsp/eq/EqBandShapes.h"
#include "dsp/graph/SpectrumTap.h"
#include "params/ParamIDs.h"
#include "ui/DesignTokens.h"

#include <cmath>
#include <cstring>

namespace
{
namespace eq = razumov::ui::tokens::eqPanel;

constexpr float kDbMin = -12.f;
constexpr float kDbMax = 12.f;

static uint32_t floatBits(float x) noexcept
{
    uint32_t u = 0;
    std::memcpy(&u, &x, sizeof(u));
    return u;
}

static juce::Colour bandColour(int b) noexcept
{
    static const uint32_t c[10] = { eq::band0, eq::band1, eq::band2, eq::band3, eq::band4,
                                    eq::band5, eq::band6, eq::band7, eq::band8, eq::band9 };
    return juce::Colour(c[(size_t) juce::jlimit(0, 9, b)]);
}

const char* const kFreqIds[10] = {
    razumov::params::eqBand1FreqHz,
    razumov::params::eqBand2FreqHz,
    razumov::params::eqBand3FreqHz,
    razumov::params::eqBand4FreqHz,
    razumov::params::eqBand5FreqHz,
    razumov::params::eqBand6FreqHz,
    razumov::params::eqBand7FreqHz,
    razumov::params::eqBand8FreqHz,
    razumov::params::eqBand9FreqHz,
    razumov::params::eqBand10FreqHz,
};
const char* const kGainIds[10] = {
    razumov::params::eqBand1GainDb,
    razumov::params::eqBand2GainDb,
    razumov::params::eqBand3GainDb,
    razumov::params::eqBand4GainDb,
    razumov::params::eqBand5GainDb,
    razumov::params::eqBand6GainDb,
    razumov::params::eqBand7GainDb,
    razumov::params::eqBand8GainDb,
    razumov::params::eqBand9GainDb,
    razumov::params::eqBand10GainDb,
};
const char* const kQIds[10] = {
    razumov::params::eqBand1Q,
    razumov::params::eqBand2Q,
    razumov::params::eqBand3Q,
    razumov::params::eqBand4Q,
    razumov::params::eqBand5Q,
    razumov::params::eqBand6Q,
    razumov::params::eqBand7Q,
    razumov::params::eqBand8Q,
    razumov::params::eqBand9Q,
    razumov::params::eqBand10Q,
};
const char* const kTypeIds[10] = {
    razumov::params::eqBand1Type,
    razumov::params::eqBand2Type,
    razumov::params::eqBand3Type,
    razumov::params::eqBand4Type,
    razumov::params::eqBand5Type,
    razumov::params::eqBand6Type,
    razumov::params::eqBand7Type,
    razumov::params::eqBand8Type,
    razumov::params::eqBand9Type,
    razumov::params::eqBand10Type,
};
const char* const kSlopeIds[10] = {
    razumov::params::eqBand1Slope,
    razumov::params::eqBand2Slope,
    razumov::params::eqBand3Slope,
    razumov::params::eqBand4Slope,
    razumov::params::eqBand5Slope,
    razumov::params::eqBand6Slope,
    razumov::params::eqBand7Slope,
    razumov::params::eqBand8Slope,
    razumov::params::eqBand9Slope,
    razumov::params::eqBand10Slope,
};

// Pure log 10 Hz..30 kHz (full width). Old piecewise margins compressed sub-bass vs Kirchhoff-style EQs.
static constexpr float kFreqVisMin = 10.f;
static constexpr float kFreqVisMax = 30000.f;

static float hzToT(float hz) noexcept
{
    const float h = juce::jlimit(kFreqVisMin, kFreqVisMax, hz);
    return (std::log10(h) - std::log10(kFreqVisMin))
         / (std::log10(kFreqVisMax) - std::log10(kFreqVisMin));
}

static float tToHz(float t) noexcept
{
    const float u = juce::jlimit(0.f, 1.f, t);
    return std::pow(10.f, std::log10(kFreqVisMin) + u * (std::log10(kFreqVisMax) - std::log10(kFreqVisMin)));
}

/** Kirchhoff-style tick labels (compact). */
static juce::String formatFreqTickLabel(float hz) noexcept
{
    if (hz < 999.5f)
        return juce::String((int) juce::roundToInt(hz));
    const float k = hz / 1000.f;
    const float rk = std::round(k);
    if (std::abs(k - rk) < 0.02f)
        return juce::String((int) rk) + "k";
    return juce::String(k, 1) + "k";
}

/** Same log Hz edges as SpectrumTap::pushStereoBlock (bin index 0..nBins). */
static float hzEdgeBin(int binIndex, int nBins, double sampleRate) noexcept
{
    using ST = razumov::graph::SpectrumTap;
    const float hzMin = ST::kAnalyzerHzMin;
    const float hzTop = juce::jmin(ST::kAnalyzerHzMax, (float) sampleRate * 0.499f);
    const float ratio = hzTop / hzMin;
    const float t = (float) binIndex / (float) nBins;
    return hzMin * std::pow(ratio, t);
}

/** Matches SpectrumTap norm: 0...1 -> 0...-120 dB (Kirchhoff-style inner scale). */
static constexpr float kAnalyserFloorDb = -120.f;
static constexpr float kAnalyserCeilDb = 0.f;

} // namespace

void ReEqPanelComponent::notifySelectionChanged(int previousSelection) noexcept
{
    if (selectedBand_ == previousSelection)
        return;
    if (onSelectionChanged_)
        onSelectionChanged_();
}

void ReEqPanelComponent::notifyAuxRefreshNeeded() noexcept
{
    if (onSelectionChanged_)
        onSelectionChanged_();
}

void ReEqPanelComponent::updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId)
{
    slotId_ = slotId;
    sampleRate_ = proc.getSampleRate() > 1.0 ? proc.getSampleRate() : 48000.0;
    if (slotId == 0 || !proc.copySpectrumInOutForSlot(slotId, binsIn_.data(), binsOut_.data()))
    {
        binsIn_.fill(0.f);
        binsOut_.fill(0.f);
    }

    /** Fast attack keeps harmonic peaks sharp; release avoids flicker (Kirchhoff-style teeth). */
    constexpr float kSpecAttackOut = 0.92f;
    constexpr float kSpecReleaseOut = 0.52f;
    constexpr float kSpecAttackIn = 0.48f;
    constexpr float kSpecReleaseIn = 0.32f;
    for (int i = 0; i < kBins; ++i)
    {
        const float x = binsOut_[(size_t) i];
        const float a = x > spectrumDisplayOut_[(size_t) i] ? kSpecAttackOut : kSpecReleaseOut;
        spectrumDisplayOut_[(size_t) i] = spectrumDisplayOut_[(size_t) i] * (1.f - a) + x * a;
    }
    for (int i = 0; i < kBins; ++i)
    {
        const float x = binsIn_[(size_t) i];
        const float a = x > spectrumDisplayIn_[(size_t) i] ? kSpecAttackIn : kSpecReleaseIn;
        spectrumDisplayIn_[(size_t) i] = spectrumDisplayIn_[(size_t) i] * (1.f - a) + x * a;
    }

    constexpr float kTrailDecay = 0.988f;
    for (int i = 0; i < kBins; ++i)
        spectrumTrailOut_[(size_t) i] =
            juce::jmax(spectrumDisplayOut_[(size_t) i], spectrumTrailOut_[(size_t) i] * kTrailDecay);

    using namespace razumov::params;
    eqBypass_ = proc.getModuleBoolParam(slotId, eqBypass);
    activeBandCount_ = juce::jlimit(0, kBands, (int) std::lround(proc.getModuleFloatParam(slotId, eqActiveBandCount)));
    if (selectedBand_ >= activeBandCount_)
        selectedBand_ = activeBandCount_ > 0 ? activeBandCount_ - 1 : -1;
    for (int i = 0; i < kBands; ++i)
    {
        freq_[(size_t) i] = proc.getModuleFloatParam(slotId, kFreqIds[i]);
        gainDb_[(size_t) i] = proc.getModuleFloatParam(slotId, kGainIds[i]);
        q_[(size_t) i] = proc.getModuleFloatParam(slotId, kQIds[i]);
        type_[(size_t) i] = proc.getModuleFloatParam(slotId, kTypeIds[i]);
        slope_[(size_t) i] = proc.getModuleFloatParam(slotId, kSlopeIds[i]);
    }
    lastResponseCacheHash_ = 0;
    repaint();
}

void ReEqPanelComponent::resized()
{
    lastResponseCacheHash_ = 0;
}

juce::Rectangle<float> ReEqPanelComponent::getPlotArea() const noexcept
{
    auto r = getLocalBounds().toFloat().reduced(6.f, 6.f);
    r.removeFromBottom(22.f);
    r.removeFromLeft(30.f);
    r.removeFromRight(26.f);
    r.removeFromBottom(20.f);
    return r;
}

float ReEqPanelComponent::hzToX(float hz, const juce::Rectangle<float>& plot) const noexcept
{
    return plot.getX() + hzToT(hz) * plot.getWidth();
}

float ReEqPanelComponent::xToHz(float x, const juce::Rectangle<float>& plot) const noexcept
{
    const float t = juce::jlimit(0.f, 1.f, (x - plot.getX()) / juce::jmax(1.f, plot.getWidth()));
    return tToHz(t);
}

float ReEqPanelComponent::dbToY(float db, const juce::Rectangle<float>& plot) const noexcept
{
    const float t = (db - kDbMin) / (kDbMax - kDbMin);
    return plot.getBottom() - juce::jlimit(0.f, 1.f, t) * plot.getHeight();
}

float ReEqPanelComponent::yToDb(float y, const juce::Rectangle<float>& plot) const noexcept
{
    const float t = juce::jlimit(0.f, 1.f, (plot.getBottom() - y) / juce::jmax(1.f, plot.getHeight()));
    return kDbMin + t * (kDbMax - kDbMin);
}

float ReEqPanelComponent::analyzerDbToY(float dbAnalyser, const juce::Rectangle<float>& plot) const noexcept
{
    const float t = (dbAnalyser - kAnalyserFloorDb) / (kAnalyserCeilDb - kAnalyserFloorDb);
    return plot.getBottom() - juce::jlimit(0.f, 1.f, t) * plot.getHeight();
}

uint64_t ReEqPanelComponent::computeResponseCacheHash(const juce::Rectangle<float>& plot) const noexcept
{
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](uint64_t v) {
        h ^= v;
        h *= 1099511628211ull;
    };
    mix((uint64_t) juce::roundToInt(plot.getWidth()));
    mix((uint64_t) juce::roundToInt(plot.getHeight()));
    mix(eqBypass_ ? 1ull : 0ull);
    mix((uint64_t) juce::roundToInt(sampleRate_));
    mix((uint64_t) juce::jlimit(0, kBands, activeBandCount_));
    for (int i = 0; i < kBands; ++i)
    {
        mix((uint64_t) floatBits(freq_[(size_t) i]));
        mix((uint64_t) floatBits(gainDb_[(size_t) i]));
        mix((uint64_t) floatBits(q_[(size_t) i]));
        mix((uint64_t) floatBits(type_[(size_t) i]));
        mix((uint64_t) floatBits(slope_[(size_t) i]));
    }
    return h;
}

void ReEqPanelComponent::rebuildResponsePaths(const juce::Rectangle<float>& plot, float nyq)
{
    cachedSumPath_.clear();
    for (auto& p : cachedBandPaths_)
        p.clear();

    const int steps = juce::jmax(32, juce::jmin(384, juce::roundToInt(plot.getWidth())));

    if (eqBypass_)
    {
        const float y0 = dbToY(0.f, plot);
        cachedSumPath_.startNewSubPath(plot.getX(), y0);
        cachedSumPath_.lineTo(plot.getRight(), y0);
        return;
    }

    const int nActive = juce::jlimit(0, kBands, activeBandCount_);
    if (nActive <= 0)
    {
        const float y0 = dbToY(0.f, plot);
        cachedSumPath_.startNewSubPath(plot.getX(), y0);
        cachedSumPath_.lineTo(plot.getRight(), y0);
        return;
    }

    for (int b = 0; b < nActive; ++b)
    {
        const auto t = razumov::dsp::eq::EqTypeFromFloat(type_[(size_t) b]);
        juce::Path& bp = cachedBandPaths_[(size_t) b];
        bool started = false;
        for (int s = 0; s <= steps; ++s)
        {
            const float tx = plot.getX() + (float) s / (float) steps * plot.getWidth();
            const float hz = xToHz(tx, plot);
            if (hz > nyq * 0.999f)
                continue;
            const double hzD = (double) juce::jlimit(1.f, (float) nyq * 0.999f, hz);
            const float db = razumov::dsp::eq::bandMagDbAtHz(t, sampleRate_, freq_[(size_t) b], gainDb_[(size_t) b], q_[(size_t) b],
                                                             slope_[(size_t) b], hzD);
            const float y = dbToY(db, plot);
            if (!started)
            {
                bp.startNewSubPath(tx, y);
                started = true;
            }
            else
            {
                bp.lineTo(tx, y);
            }
        }
    }

    bool sumStarted = false;
    for (int s = 0; s <= steps; ++s)
    {
        const float tx = plot.getX() + (float) s / (float) steps * plot.getWidth();
        const float hz = xToHz(tx, plot);
        if (hz > nyq * 0.999f)
            continue;
        const double hzD = (double) juce::jlimit(1.f, (float) nyq * 0.999f, hz);
        float sumDb = 0.f;
        for (int b = 0; b < nActive; ++b)
        {
            const auto t = razumov::dsp::eq::EqTypeFromFloat(type_[(size_t) b]);
            sumDb += razumov::dsp::eq::bandMagDbAtHz(t, sampleRate_, freq_[(size_t) b], gainDb_[(size_t) b], q_[(size_t) b],
                                                     slope_[(size_t) b], hzD);
        }
        const float y = dbToY(sumDb, plot);
        if (!sumStarted)
        {
            cachedSumPath_.startNewSubPath(tx, y);
            sumStarted = true;
        }
        else
        {
            cachedSumPath_.lineTo(tx, y);
        }
    }
}

void ReEqPanelComponent::fillSpectrumHistogramPath(juce::Path& p, const juce::Rectangle<float>& plot,
                                                   const float* spectrumData) const noexcept
{
    p.clear();
    const float yBottom = analyzerDbToY(kAnalyserFloorDb, plot);
    const int n = kBins;
    const double sr = sampleRate_;

    auto yAtNorm = [&](float v) {
        const float db = kAnalyserFloorDb + juce::jlimit(0.f, 1.f, v) * (kAnalyserCeilDb - kAnalyserFloorDb);
        return analyzerDbToY(db, plot);
    };

    const float xL0 = hzToX(hzEdgeBin(0, n, sr), plot);
    p.startNewSubPath(xL0, yBottom);
    for (int b = 0; b < n; ++b)
    {
        const float xL = hzToX(hzEdgeBin(b, n, sr), plot);
        const float xR = hzToX(hzEdgeBin(b + 1, n, sr), plot);
        const float v = juce::jlimit(0.f, 1.f, spectrumData[(size_t) b]);
        const float yb = yAtNorm(v);
        if (b == 0)
            p.lineTo(xL, yb);
        p.lineTo(xR, yb);
        if (b < n - 1)
        {
            const float vNext = juce::jlimit(0.f, 1.f, spectrumData[(size_t) b + 1]);
            const float yNext = yAtNorm(vNext);
            p.lineTo(xR, yNext);
        }
    }
    p.lineTo(hzToX(hzEdgeBin(n, n, sr), plot), yBottom);
    p.closeSubPath();
}

int ReEqPanelComponent::hitTestBand(juce::Point<float> pos) const noexcept
{
    const auto plot = getPlotArea();
    if (!plot.contains(pos))
        return -1;

    float bestD2 = 400.f;
    int best = -1;
    const int n = juce::jlimit(0, kBands, activeBandCount_);
    for (int b = 0; b < n; ++b)
    {
        const float x = hzToX(freq_[(size_t) b], plot);
        const float y = dbToY(gainDb_[(size_t) b], plot);
        const float dx = pos.x - x;
        const float dy = pos.y - y;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2)
        {
            bestD2 = d2;
            best = b;
        }
    }
    if (best >= 0 && bestD2 < 28.f * 28.f)
        return best;
    return -1;
}

void ReEqPanelComponent::pushBandParamsFromMouse(juce::Point<float> localPos, bool writeFreqGain)
{
    if (processor_ == nullptr || slotId_ == 0 || dragBand_ < 0 || dragBand_ >= kBands)
        return;

    const auto plot = getPlotArea();
    const int bi = dragBand_;
    if (writeFreqGain)
    {
        const float hz = juce::jlimit(20.f, 20000.f, xToHz(localPos.x, plot));
        const float db = juce::jlimit(kDbMin, kDbMax, yToDb(localPos.y, plot));
        processor_->setModuleFloatParam(slotId_, kFreqIds[bi], hz);
        processor_->setModuleFloatParam(slotId_, kGainIds[bi], db);
    }
    freq_[(size_t) bi] = processor_->getModuleFloatParam(slotId_, kFreqIds[bi]);
    gainDb_[(size_t) bi] = processor_->getModuleFloatParam(slotId_, kGainIds[bi]);
    q_[(size_t) bi] = processor_->getModuleFloatParam(slotId_, kQIds[bi]);
    slope_[(size_t) bi] = processor_->getModuleFloatParam(slotId_, kSlopeIds[bi]);
    lastResponseCacheHash_ = 0;
    repaint();
}

void ReEqPanelComponent::showTypeMenuForBand(int bandIndex, juce::Point<int> screenPos)
{
    if (processor_ == nullptr || slotId_ == 0 || bandIndex < 0 || bandIndex >= kBands)
        return;

    juce::PopupMenu m;
    m.addItem(1, "Peak");
    m.addItem(2, "Low shelf");
    m.addItem(3, "High shelf");
    m.addItem(4, "Low pass");
    m.addItem(5, "High pass");
    m.addItem(6, "Notch");

    m.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1)),
                    [this, bandIndex](int r) {
                        if (r <= 0 || processor_ == nullptr || slotId_ == 0)
                            return;
                        const float v = (float) (r - 1);
                        processor_->setModuleFloatParam(slotId_, kTypeIds[bandIndex], v);
                        type_[(size_t) bandIndex] = v;
                        lastResponseCacheHash_ = 0;
                        repaint();
                        notifyAuxRefreshNeeded();
                    });
}

void ReEqPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(eq::plotSolidFill));
    g.fillRoundedRectangle(bounds.reduced(0.5f), 6.f);
    g.setColour(juce::Colour(eq::frameBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.f, 1.f);

    const auto plot = getPlotArea();
    const float nyq = (float) (sampleRate_ * 0.5);

    const uint64_t h = computeResponseCacheHash(plot);
    if (h != lastResponseCacheHash_)
    {
        rebuildResponsePaths(plot, nyq);
        lastResponseCacheHash_ = h;
    }

    static const float kVertGridHz[] = { 10.f,  20.f,  30.f,  40.f,  50.f,  100.f, 200.f, 300.f, 400.f, 500.f,
                                         1000.f, 2000.f, 3000.f, 4000.f, 5000.f, 7000.f, 10000.f, 20000.f, 30000.f };
    for (float hz : kVertGridHz)
    {
        const float x = hzToX(hz, plot);
        if (x < plot.getX() || x > plot.getRight())
            continue;
        const bool major = (hz == 100.f || hz == 1000.f || hz == 10000.f);
        g.setColour(juce::Colour(eq::gridLine).withAlpha(major ? 0.22f : 0.10f));
        g.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
    }
    const float y0 = dbToY(0.f, plot);
    g.setColour(juce::Colour(eq::zeroDbLine));
    g.drawHorizontalLine(juce::roundToInt(y0), plot.getX(), plot.getRight());

    for (float db : { -9.f, -6.f, -3.f, 3.f, 6.f, 9.f })
    {
        const float yy = dbToY(db, plot);
        g.setColour(juce::Colour(eq::gridLine).withAlpha(0.30f));
        g.drawHorizontalLine(juce::roundToInt(yy), plot.getX(), plot.getRight());
    }
    for (float db : { kDbMin, 0.f, kDbMax })
    {
        const float yy = dbToY(db, plot);
        g.setColour(juce::Colour(eq::gridLine).withAlpha(0.55f));
        g.drawHorizontalLine(juce::roundToInt(yy), plot.getX(), plot.getRight());
    }

    juce::Path trailPath;
    fillSpectrumHistogramPath(trailPath, plot, spectrumTrailOut_.data());
    g.setColour(juce::Colour(eq::spectrumFillSolid).withAlpha(0.11f));
    g.fillPath(trailPath);
    g.setColour(juce::Colour(eq::spectrumLine).withAlpha(0.08f));
    g.strokePath(trailPath, juce::PathStrokeType(4.2f));
    g.setColour(juce::Colour(eq::spectrumLine).withAlpha(0.14f));
    g.strokePath(trailPath, juce::PathStrokeType(2.0f));

    juce::Path inPath;
    fillSpectrumHistogramPath(inPath, plot, spectrumDisplayIn_.data());
    g.setColour(juce::Colour(eq::spectrumFillSolid).withAlpha(0.17f));
    g.fillPath(inPath);
    g.setColour(juce::Colour(eq::spectrumLine).withAlpha(0.30f));
    g.strokePath(inPath, juce::PathStrokeType(1.0f));

    juce::Path specPath;
    fillSpectrumHistogramPath(specPath, plot, spectrumDisplayOut_.data());
    g.setColour(juce::Colour(eq::spectrumFillSolid).withAlpha(0.40f));
    g.fillPath(specPath);
    g.setColour(juce::Colour(eq::spectrumLine).withAlpha(0.82f));
    g.strokePath(specPath, juce::PathStrokeType(1.0f));

    if (!eqBypass_)
    {
        const int nDraw = juce::jlimit(0, kBands, activeBandCount_);
        for (int b = 0; b < nDraw; ++b)
        {
            g.setColour(bandColour(b).withAlpha(0.38f));
            g.strokePath(cachedBandPaths_[(size_t) b], juce::PathStrokeType(1.0f));
        }
    }

    g.setColour(juce::Colour(eq::curveGlow).withAlpha(eqBypass_ ? 0.25f : 0.55f));
    g.strokePath(cachedSumPath_, juce::PathStrokeType(2.4f));
    g.setColour(juce::Colour(eq::curveCore).withAlpha(eqBypass_ ? 0.35f : 0.95f));
    g.strokePath(cachedSumPath_, juce::PathStrokeType(1.25f));

    g.setColour(juce::Colour(eq::captionText));
    g.setFont(juce::FontOptions(8.2f));
    for (float hz : kVertGridHz)
    {
        const float x = hzToX(hz, plot);
        if (x < plot.getX() - 8.f || x > plot.getRight() + 8.f)
            continue;
        const juce::String lab = formatFreqTickLabel(hz);
        g.drawText(lab, juce::Rectangle<int>((int) x - 20, (int) plot.getBottom() + 1, 40, 12), juce::Justification::centred);
    }

    g.setFont(juce::FontOptions(9.5f));
    for (float db = kDbMax; db >= kDbMin - 0.1f; db -= 3.f)
    {
        const float yy = dbToY(db, plot);
        juce::String lab = (db > 0.f ? "+" : "") + juce::String(db, 0) + " dB";
        g.drawText(lab, juce::Rectangle<int>((int) plot.getX() - 30, (int) yy - 7, 28, 14), juce::Justification::centredRight);
    }

    g.setFont(juce::FontOptions(8.0f));
    g.setColour(juce::Colour(eq::captionText).withAlpha(0.55f));
    for (float adb = 0.f; adb >= -120.1f; adb -= 24.f)
    {
        const float yy = analyzerDbToY(adb, plot);
        g.drawText(juce::String((int) adb),
                   juce::Rectangle<int>((int) plot.getRight() + 2, (int) yy - 6, 26, 12),
                   juce::Justification::centredLeft);
    }

    {
        const int nDraw = juce::jlimit(0, kBands, activeBandCount_);
        for (int b = 0; b < nDraw; ++b)
        {
            const float cx = hzToX(freq_[(size_t) b], plot);
            const float cy = dbToY(gainDb_[(size_t) b], plot);
            const auto col = bandColour(b);
            const bool sel = (b == selectedBand_);
            g.setColour(col.withAlpha(0.32f));
            g.fillEllipse(cx - 9.f, cy - 9.f, 18.f, 18.f);
            g.setColour(col.withAlpha(sel ? 1.f : 0.9f));
            g.drawEllipse(cx - 9.f, cy - 9.f, 18.f, 18.f, sel ? 2.2f : 1.4f);
            g.setColour(juce::Colour(eq::nodeLabel).withAlpha(0.95f));
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawText(juce::String(b + 1), juce::Rectangle<int>((int) cx - 8, (int) cy - 8, 16, 16), juce::Justification::centred);
        }
    }

    g.setColour(juce::Colour(eq::captionText));
    g.setFont(juce::FontOptions(10.5f));
    juce::String cap = "Double-click empty plot: add band. Double-click dot: remove band. Drag: freq + gain. Wheel: Q. RMB: type. LP/HP: Slope 0...96 dB/oct.";
    if (selectedBand_ >= 0 && selectedBand_ < kBands)
    {
        cap += "  |  Band " + juce::String(selectedBand_ + 1) + "  "
               + juce::String(freq_[(size_t) selectedBand_], 0) + " Hz  "
               + juce::String(gainDb_[(size_t) selectedBand_], 1) + " dB  Q "
               + juce::String(q_[(size_t) selectedBand_], 2);
        const int t = juce::jlimit(0, 5, juce::roundToInt(type_[(size_t) selectedBand_]));
        if (t == 3 || t == 4)
            cap += "  Slope " + juce::String(slope_[(size_t) selectedBand_], 0) + " dB/oct";
    }
    g.drawText(cap, getLocalBounds().removeFromBottom(20).reduced(8, 0).toFloat(), juce::Justification::centredLeft);
}

void ReEqPanelComponent::mouseDown(const juce::MouseEvent& e)
{
    if (processor_ == nullptr || slotId_ == 0)
        return;

    const auto pos = e.position;
    const int prevSel = selectedBand_;

    if (e.mods.isPopupMenu())
    {
        const int hit = hitTestBand(pos);
        if (hit >= 0)
        {
            selectedBand_ = hit;
            notifySelectionChanged(prevSel);
            showTypeMenuForBand(hit, e.getScreenPosition());
        }
        repaint();
        return;
    }

    const int hit = hitTestBand(pos);
    selectedBand_ = hit;
    notifySelectionChanged(prevSel);
    if (hit >= 0)
    {
        dragBand_ = hit;
        pushBandParamsFromMouse(pos, true);
    }
    else
        dragBand_ = -1;
    repaint();
}

void ReEqPanelComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (dragBand_ < 0)
        return;
    pushBandParamsFromMouse(e.position, true);
}

void ReEqPanelComponent::mouseUp(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    dragBand_ = -1;
}

void ReEqPanelComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (processor_ == nullptr || slotId_ == 0)
        return;

    int band = hitTestBand(e.position);
    if (band < 0 && selectedBand_ >= 0)
        band = selectedBand_;
    if (band < 0)
        return;

    const float delta = wheel.deltaY > 0 ? 1.06f : (1.f / 1.06f);
    float q = processor_->getModuleFloatParam(slotId_, kQIds[band]);
    q = juce::jlimit(0.3f, 20.f, q * delta);
    processor_->setModuleFloatParam(slotId_, kQIds[band], q);
    q_[(size_t) band] = q;
    lastResponseCacheHash_ = 0;
    repaint();
}

void ReEqPanelComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (processor_ == nullptr || slotId_ == 0)
        return;

    const auto pos = e.position;
    const int hit = hitTestBand(pos);
    const int n = juce::jlimit(0, kBands, activeBandCount_);

    if (hit >= 0)
    {
        if (n <= 0 || hit >= n)
            return;

        for (int i = hit; i < n - 1; ++i)
        {
            const int j = i + 1;
            processor_->setModuleFloatParam(slotId_, kFreqIds[i], processor_->getModuleFloatParam(slotId_, kFreqIds[j]));
            processor_->setModuleFloatParam(slotId_, kGainIds[i], processor_->getModuleFloatParam(slotId_, kGainIds[j]));
            processor_->setModuleFloatParam(slotId_, kQIds[i], processor_->getModuleFloatParam(slotId_, kQIds[j]));
            processor_->setModuleFloatParam(slotId_, kTypeIds[i], processor_->getModuleFloatParam(slotId_, kTypeIds[j]));
            processor_->setModuleFloatParam(slotId_, kSlopeIds[i], processor_->getModuleFloatParam(slotId_, kSlopeIds[j]));
        }
        processor_->setModuleFloatParam(slotId_, razumov::params::eqActiveBandCount, (float) (n - 1));

        const int newCount = n - 1;
        int newSel = selectedBand_;
        if (newSel == hit)
            newSel = newCount > 0 ? juce::jmin(hit, newCount - 1) : -1;
        else if (newSel > hit)
            newSel--;

        const int prevSel = selectedBand_;
        updateFrom(*processor_, slotId_);
        selectedBand_ = newSel;
        notifySelectionChanged(prevSel);
        if (selectedBand_ == prevSel)
            notifyAuxRefreshNeeded();
        repaint();
        return;
    }

    const auto plot = getPlotArea();
    if (!plot.contains(pos))
        return;

    if (n >= kBands)
        return;

    const float hz = juce::jlimit(20.f, 20000.f, xToHz(pos.x, plot));
    const float db = juce::jlimit(kDbMin, kDbMax, yToDb(pos.y, plot));
    const int newIdx = n;

    processor_->setModuleFloatParam(slotId_, kFreqIds[newIdx], hz);
    processor_->setModuleFloatParam(slotId_, kGainIds[newIdx], db);
    processor_->setModuleFloatParam(slotId_, kQIds[newIdx], 1.0f);
    processor_->setModuleFloatParam(slotId_, kTypeIds[newIdx], 0.0f);
    processor_->setModuleFloatParam(slotId_, kSlopeIds[newIdx], 48.0f);
    processor_->setModuleFloatParam(slotId_, razumov::params::eqActiveBandCount, (float) (newIdx + 1));

    updateFrom(*processor_, slotId_);
    const int prevSel = selectedBand_;
    selectedBand_ = newIdx;
    notifySelectionChanged(prevSel);
    repaint();
}
