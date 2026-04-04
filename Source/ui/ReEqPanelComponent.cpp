#include "ReEqPanelComponent.h"

#include "PluginProcessor.h"
#include "dsp/eq/EqBandShapes.h"
#include "params/ParamIDs.h"
#include "ui/DesignTokens.h"

#include <cmath>

namespace
{
namespace tkn = razumov::ui::tokens::argb;

constexpr float kDbMin = -18.f;
constexpr float kDbMax = 18.f;

const juce::Colour kBandColours[5] = {
    juce::Colour(0xff5b82c8),
    juce::Colour(0xff3cb878),
    juce::Colour(0xffd9a23c),
    juce::Colour(0xffa878d8),
    juce::Colour(0xff4ab0c8),
};

const char* const kFreqIds[5] = {
    razumov::params::eqBand1FreqHz,
    razumov::params::eqBand2FreqHz,
    razumov::params::eqBand3FreqHz,
    razumov::params::eqBand4FreqHz,
    razumov::params::eqBand5FreqHz,
};
const char* const kGainIds[5] = {
    razumov::params::eqBand1GainDb,
    razumov::params::eqBand2GainDb,
    razumov::params::eqBand3GainDb,
    razumov::params::eqBand4GainDb,
    razumov::params::eqBand5GainDb,
};
const char* const kQIds[5] = {
    razumov::params::eqBand1Q,
    razumov::params::eqBand2Q,
    razumov::params::eqBand3Q,
    razumov::params::eqBand4Q,
    razumov::params::eqBand5Q,
};
const char* const kTypeIds[5] = {
    razumov::params::eqBand1Type,
    razumov::params::eqBand2Type,
    razumov::params::eqBand3Type,
    razumov::params::eqBand4Type,
    razumov::params::eqBand5Type,
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

void ReEqPanelComponent::updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId)
{
    slotId_ = slotId;
    sampleRate_ = proc.getSampleRate() > 1.0 ? proc.getSampleRate() : 48000.0;
    if (slotId == 0 || !proc.copySpectrumForSlot(slotId, bins_.data()))
        bins_.fill(0.f);

    using namespace razumov::params;
    eqBypass_ = proc.getModuleBoolParam(slotId, eqBypass);
    for (int i = 0; i < kBands; ++i)
    {
        freq_[(size_t) i] = proc.getModuleFloatParam(slotId, kFreqIds[i]);
        gainDb_[(size_t) i] = proc.getModuleFloatParam(slotId, kGainIds[i]);
        q_[(size_t) i] = proc.getModuleFloatParam(slotId, kQIds[i]);
        type_[(size_t) i] = proc.getModuleFloatParam(slotId, kTypeIds[i]);
    }
    repaint();
}

juce::Rectangle<float> ReEqPanelComponent::getPlotArea() const noexcept
{
    auto r = getLocalBounds().toFloat().reduced(6.f, 6.f);
    r.removeFromBottom(22.f);
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

int ReEqPanelComponent::hitTestBand(juce::Point<float> pos) const noexcept
{
    const auto plot = getPlotArea();
    if (!plot.contains(pos))
        return -1;

    float bestD2 = 400.f;
    int best = -1;
    for (int b = 0; b < kBands; ++b)
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
                        repaint();
                    });
}

void ReEqPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(tkn::backgroundNode));
    g.fillRoundedRectangle(bounds.reduced(0.5f), 6.f);
    g.setColour(juce::Colour(tkn::borderModulePanel));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.f, 1.f);

    const auto plot = getPlotArea();
    const float nyq = (float) (sampleRate_ * 0.5);

    // Grid: 100, 1k, 10k Hz
    g.setColour(juce::Colour(tkn::textTertiary).withAlpha(0.35f));
    for (float hz : { 100.f, 1000.f, 10000.f })
    {
        const float x = hzToX(hz, plot);
        g.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
    }
    // 0 dB line
    const float y0 = dbToY(0.f, plot);
    g.setColour(juce::Colour(tkn::textSecondary).withAlpha(0.45f));
    g.drawHorizontalLine(juce::roundToInt(y0), plot.getX(), plot.getRight());

    // Spectrum (filled)
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
    g.setColour(juce::Colour(tkn::accentSignal).withAlpha(0.22f));
    g.fillPath(specPath);
    g.setColour(juce::Colour(tkn::accentSignal).withAlpha(0.55f));
    g.strokePath(specPath, juce::PathStrokeType(1.0f));

    // Magnitude response (flat 0 dB when EQ bypass)
    juce::Path magPath;
    const int steps = juce::jmin(320, juce::roundToInt(plot.getWidth()));
    if (eqBypass_)
    {
        const float y0 = dbToY(0.f, plot);
        magPath.startNewSubPath(plot.getX(), y0);
        magPath.lineTo(plot.getRight(), y0);
    }
    else
    {
        razumov::dsp::eq::Coeffs::Ptr coeffs[5];
        for (int b = 0; b < kBands; ++b)
        {
            const auto t = razumov::dsp::eq::EqTypeFromFloat(type_[(size_t) b]);
            coeffs[b] = razumov::dsp::eq::makeBandCoeffs(t, sampleRate_, freq_[(size_t) b], gainDb_[(size_t) b], q_[(size_t) b]);
        }

        for (int s = 0; s <= steps; ++s)
        {
            const float tx = plot.getX() + (float) s / (float) steps * plot.getWidth();
            const float hz = xToHz(tx, plot);
            if (hz > nyq * 0.999f)
                continue;
            const double hzD = (double) juce::jlimit(1.f, (float) nyq * 0.999f, hz);
            const float sumDb = razumov::dsp::eq::sumCascadeMagDbAtHz(coeffs, kBands, hzD, sampleRate_);
            const float y = dbToY(sumDb, plot);
            if (s == 0)
                magPath.startNewSubPath(tx, y);
            else
                magPath.lineTo(tx, y);
        }
    }
    g.setColour(juce::Colour(0xff1a2438).withAlpha(eqBypass_ ? 0.45f : 0.92f));
    g.strokePath(magPath, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colour(tkn::accentSelection).withAlpha(eqBypass_ ? 0.35f : 0.88f));
    g.strokePath(magPath, juce::PathStrokeType(1.2f));

    // Nodes
    for (int b = 0; b < kBands; ++b)
    {
        const float cx = hzToX(freq_[(size_t) b], plot);
        const float cy = dbToY(gainDb_[(size_t) b], plot);
        const auto col = kBandColours[b];
        const bool sel = (b == selectedBand_);
        g.setColour(col.withAlpha(0.28f));
        g.fillEllipse(cx - 9.f, cy - 9.f, 18.f, 18.f);
        g.setColour(col.withAlpha(sel ? 1.f : 0.88f));
        g.drawEllipse(cx - 9.f, cy - 9.f, 18.f, 18.f, sel ? 2.2f : 1.4f);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(juce::String(b + 1), juce::Rectangle<int>((int) cx - 8, (int) cy - 8, 16, 16), juce::Justification::centred);
    }

    // Caption
    g.setColour(juce::Colour(tkn::textCaption));
    g.setFont(juce::FontOptions(10.5f));
    juce::String cap = "Drag nodes: freq + gain. Wheel: Q. RMB: type.";
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
    if (e.mods.isPopupMenu())
    {
        const int hit = hitTestBand(pos);
        if (hit >= 0)
        {
            selectedBand_ = hit;
            showTypeMenuForBand(hit, e.getScreenPosition());
        }
        repaint();
        return;
    }

    const int hit = hitTestBand(pos);
    selectedBand_ = hit;
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
    repaint();
}
