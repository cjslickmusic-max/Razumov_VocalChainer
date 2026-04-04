#pragma once

#include <array>
#include <cstdint>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

class RazumovVocalChainAudioProcessor;

/** Spectrum + combined magnitude response + draggable EQ nodes (5 bands). */
class ReEqPanelComponent : public juce::Component
{
public:
    void setProcessor(RazumovVocalChainAudioProcessor* p) noexcept { processor_ = p; }

    void updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

private:
    static constexpr int kBands = 5;
    static constexpr int kBins = 256;

    juce::Rectangle<float> getPlotArea() const noexcept;
    float hzToX(float hz, const juce::Rectangle<float>& plot) const noexcept;
    float xToHz(float x, const juce::Rectangle<float>& plot) const noexcept;
    float dbToY(float db, const juce::Rectangle<float>& plot) const noexcept;
    float yToDb(float y, const juce::Rectangle<float>& plot) const noexcept;

    int hitTestBand(juce::Point<float> posInComponent) const noexcept;
    void showTypeMenuForBand(int bandIndex, juce::Point<int> screenPos);
    void pushBandParamsFromMouse(juce::Point<float> localPos, bool writeFreqGain);

    uint64_t hashEqParams() const noexcept;
    void rebuildMagPathIfNeeded(const juce::Rectangle<float>& plot, float nyq);

    RazumovVocalChainAudioProcessor* processor_ { nullptr };
    uint32_t slotId_ { 0 };
    double sampleRate_ { 48000.0 };
    std::array<float, (size_t) kBins> bins_{};
    std::array<float, (size_t) kBands> freq_{};
    std::array<float, (size_t) kBands> gainDb_{};
    std::array<float, (size_t) kBands> q_{};
    std::array<float, (size_t) kBands> type_{};
    bool eqBypass_ { false };

    int selectedBand_ { -1 };
    int dragBand_ { -1 };

    /** Avoid heap churn + redundant magnitude math when params/plot unchanged (UI thread only). */
    std::array<juce::dsp::IIR::Coefficients<float>::Ptr, (size_t) kBands> coeffCache_{};
    uint64_t lastCoeffHash_ { 0 };
    juce::Rectangle<float> lastMagPlot_{};
    juce::Path magPathCache_{};
    bool magPathBypass_ { false };
};
