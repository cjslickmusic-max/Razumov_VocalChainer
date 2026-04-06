#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "dsp/graph/ISpectrumSource.h"

class RazumovVocalChainAudioProcessor;

/** Spectrum + per-band + sum magnitude + draggable nodes (up to 10 bands; double-click plot to add). */
class ReEqPanelComponent : public juce::Component
{
public:
    void setProcessor(RazumovVocalChainAudioProcessor* p) noexcept { processor_ = p; }

    void setOnSelectionChanged(std::function<void()> cb) { onSelectionChanged_ = std::move(cb); }

    int getSelectedBand() const noexcept { return selectedBand_; }

    /** Notify editor (e.g. show/hide Slope knob when filter type changes). */
    void notifyAuxRefreshNeeded() noexcept;

    void updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

private:
    static constexpr int kBands = 10;
    static constexpr int kBins = razumov::graph::ISpectrumSource::kSpectrumBins;

    juce::Rectangle<float> getPlotArea() const noexcept;
    float hzToX(float hz, const juce::Rectangle<float>& plot) const noexcept;
    float xToHz(float x, const juce::Rectangle<float>& plot) const noexcept;
    float dbToY(float db, const juce::Rectangle<float>& plot) const noexcept;
    float yToDb(float y, const juce::Rectangle<float>& plot) const noexcept;
    /** Analyzer overlay: norm 0...1 maps to ~0...-120 dB (same plot rect as EQ curve). */
    float analyzerDbToY(float dbAnalyser, const juce::Rectangle<float>& plot) const noexcept;

    int hitTestBand(juce::Point<float> pos) const noexcept;
    void showTypeMenuForBand(int bandIndex, juce::Point<int> screenPos);
    void pushBandParamsFromMouse(juce::Point<float> localPos, bool writeFreqGain);
    void notifySelectionChanged(int previousSelection) noexcept;

    uint64_t computeResponseCacheHash(const juce::Rectangle<float>& plot) const noexcept;
    void rebuildResponsePaths(const juce::Rectangle<float>& plot, float nyq);

    RazumovVocalChainAudioProcessor* processor_ { nullptr };
    uint32_t slotId_ { 0 };
    double sampleRate_ { 48000.0 };
    std::array<float, (size_t) kBins> binsIn_{};
    std::array<float, (size_t) kBins> binsOut_{};
    /** UI-thread smoothed spectrum (attack/release); post-EQ is primary visual. */
    std::array<float, (size_t) kBins> spectrumDisplayIn_{};
    std::array<float, (size_t) kBins> spectrumDisplayOut_{};
    /** Peak decay on post-EQ spectrum only. */
    std::array<float, (size_t) kBins> spectrumTrailOut_{};

    /** Piecewise-constant (histogram) top: no linear interp between bins -> sharper harmonics. */
    void fillSpectrumHistogramPath(juce::Path& p, const juce::Rectangle<float>& plot, const float* spectrumData) const noexcept;
    std::array<float, (size_t) kBands> freq_{};
    std::array<float, (size_t) kBands> gainDb_{};
    std::array<float, (size_t) kBands> q_{};
    std::array<float, (size_t) kBands> type_{};
    /** LP/HP: 0...96 dB/oct (cascade). */
    std::array<float, (size_t) kBands> slope_{};
    bool eqBypass_ { false };
    int activeBandCount_ { 0 };

    int selectedBand_ { -1 };
    int dragBand_ { -1 };

    std::function<void()> onSelectionChanged_;

    uint64_t lastResponseCacheHash_ { 0 };
    juce::Path cachedSumPath_;
    std::array<juce::Path, (size_t) kBands> cachedBandPaths_{};
};
