#include "ReEqPanelComponent.h"

#include "PluginProcessor.h"
#include "dsp/eq/EqBandShapes.h"
#include "params/ParamIDs.h"
#include "ui/DesignTokens.h"

#include <cmath>
#include <cstring>

namespace
{
namespace eq = razumov::ui::tokens::eqPanel;

constexpr float kDbMin = -18.f;
constexpr float kDbMax = 18.f;

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

static float hzToT(float hz) noexcept
{
    const float h = juce::jlimit(20.f, 20000.f, hz);
    return std::log(h / 20.f) / std::log(20000.f / 20.f);
}

static float tToHz(float t) noexcept
{
    const float u = juce::jlimit(0.f, 1.f, t);
    return 20.f * std::pow(20000.f / 20.f, u);
}

} // namespace

void ReEqPanelComponent::notifySelectionChanged(int previousSelection) noexcept
{
    if (selectedBand_ == previousSelection)
        return;
    if (onSelectionChanged_)
        onSelectionChanged_();
}

void ReEqPanelComponent::updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId)
{
    slotId_ = slotId;
    sampleRate_ = proc.getSampleRate() > 1.0 ? proc.getSampleRate() : 48000.0;
    if (slotId == 0 || !proc.copySpectrumForSlot(slotId, bins_.data()))
        bins_.fill(0.f);

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

    razumov::dsp::eq::Coeffs::Ptr coeffs[kBands] = {};
    for (int b = 0; b < nActive; ++b)
    {
        const auto t = razumov::dsp::eq::EqTypeFromFloat(type_[(size_t) b]);
        coeffs[b] = razumov::dsp::eq::makeBandCoeffs(t, sampleRate_, freq_[(size_t) b], gainDb_[(size_t) b], q_[(size_t) b]);
    }

    for (int b = 0; b < nActive; ++b)
    {
        razumov::dsp::eq::Coeffs::Ptr one[kBands] = {};
        one[b] = coeffs[b];
        juce::Path& bp = cachedBandPaths_[(size_t) b];
        bool started = false;
        for (int s = 0; s <= steps; ++s)
        {
            const float tx = plot.getX() + (float) s / (float) steps * plot.getWidth();
            const float hz = xToHz(tx, plot);
            if (hz > nyq * 0.999f)
                continue;
            const double hzD = (double) juce::jlimit(1.f, (float) nyq * 0.999f, hz);
            const float db = razumov::dsp::eq::sumCascadeMagDbAtHz(one, kBands, hzD, sampleRate_);
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
        const float sumDb = razumov::dsp::eq::sumCascadeMagDbAtHz(coeffs, kBands, hzD, sampleRate_);
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
                    });
}

void ReEqPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bg(juce::Colour(eq::plotGradientTop),
                            bounds.getX(),
                            bounds.getY(),
                            juce::Colour(eq::plotGradientBottom),
                            bounds.getRight(),
                            bounds.getBottom(),
                            false);
    g.setGradientFill(bg);
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

    g.setColour(juce::Colour(eq::gridLine));
    for (float hz : { 100.f, 1000.f, 10000.f })
    {
        const float x = hzToX(hz, plot);
        g.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
    }
    const float y0 = dbToY(0.f, plot);
    g.setColour(juce::Colour(eq::zeroDbLine));
    g.drawHorizontalLine(juce::roundToInt(y0), plot.getX(), plot.getRight());

    for (float db : { kDbMin, 0.f, kDbMax })
    {
        const float yy = dbToY(db, plot);
        g.setColour(juce::Colour(eq::gridLine).withAlpha(0.55f));
        g.drawHorizontalLine(juce::roundToInt(yy), plot.getX(), plot.getRight());
    }

    juce::Path specPath;
    const float base = plot.getBottom() - 1.f;
    const float plotH = plot.getHeight();
    specPath.startNewSubPath(plot.getX(), base);
    for (int i = 0; i < kBins; ++i)
    {
        const float x = plot.getX() + (float) i / (float) juce::jmax(1, kBins - 1) * plot.getWidth();
        const float v = juce::jlimit(0.f, 1.f, bins_[(size_t) i]);
        const float y = base - v * (plotH - 4.f);
        specPath.lineTo(x, y);
    }
    specPath.lineTo(plot.getRight(), base);
    specPath.closeSubPath();
    juce::ColourGradient specGrad(juce::Colour(eq::spectrumFillHi).withAlpha(0.42f),
                                  plot.getCentreX(),
                                  plot.getY(),
                                  juce::Colour(eq::spectrumFillLo).withAlpha(0.12f),
                                  plot.getCentreX(),
                                  plot.getBottom(),
                                  false);
    g.setGradientFill(specGrad);
    g.fillPath(specPath);
    g.setColour(juce::Colour(eq::spectrumLine).withAlpha(0.75f));
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
    g.setFont(juce::FontOptions(9.5f));
    for (float hz : { 100.f, 1000.f, 10000.f })
    {
        const float x = hzToX(hz, plot);
        juce::String lab = (hz >= 1000.f) ? juce::String(hz / 1000.f, (hz >= 10000.f ? 0 : 1)) + " k" : juce::String((int) hz);
        g.drawText(lab, juce::Rectangle<int>((int) x - 18, (int) plot.getBottom() + 2, 36, 14), juce::Justification::centred);
    }

    g.setFont(juce::FontOptions(9.5f));
    for (float db : { kDbMax, 0.f, kDbMin })
    {
        const float yy = dbToY(db, plot);
        juce::String lab = (db > 0.f ? "+" : "") + juce::String(db, 0) + " dB";
        g.drawText(lab, juce::Rectangle<int>((int) plot.getX() - 30, (int) yy - 7, 28, 14), juce::Justification::centredRight);
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
    juce::String cap = "Double-click plot: add band (max 10). Drag: freq + gain. Wheel: Q. RMB: type.";
    if (selectedBand_ >= 0 && selectedBand_ < kBands)
    {
        cap += "  |  Band " + juce::String(selectedBand_ + 1) + "  "
               + juce::String(freq_[(size_t) selectedBand_], 0) + " Hz  "
               + juce::String(gainDb_[(size_t) selectedBand_], 1) + " dB  Q "
               + juce::String(q_[(size_t) selectedBand_], 2);
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
    if (hitTestBand(pos) >= 0)
        return;

    const auto plot = getPlotArea();
    if (!plot.contains(pos))
        return;

    const int n = juce::jlimit(0, kBands, activeBandCount_);
    if (n >= kBands)
        return;

    const float hz = juce::jlimit(20.f, 20000.f, xToHz(pos.x, plot));
    const float db = juce::jlimit(kDbMin, kDbMax, yToDb(pos.y, plot));
    const int newIdx = n;

    processor_->setModuleFloatParam(slotId_, kFreqIds[newIdx], hz);
    processor_->setModuleFloatParam(slotId_, kGainIds[newIdx], db);
    processor_->setModuleFloatParam(slotId_, kQIds[newIdx], 1.0f);
    processor_->setModuleFloatParam(slotId_, kTypeIds[newIdx], 0.0f);
    processor_->setModuleFloatParam(slotId_, razumov::params::eqActiveBandCount, (float) (newIdx + 1));

    updateFrom(*processor_, slotId_);
    const int prevSel = selectedBand_;
    selectedBand_ = newIdx;
    notifySelectionChanged(prevSel);
    repaint();
}
