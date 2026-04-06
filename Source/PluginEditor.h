#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <memory>

#include "dsp/graph/AudioNode.h"
#include "dsp/graph/ISpectrumSource.h"
#include "ui/ChainStripComponent.h"
#include "ui/ReEqPanelComponent.h"

class RazumovVocalChainAudioProcessor;

struct VocalChainerLookAndFeel;

struct ModuleSectionBackdrop : juce::Component
{
    void paint(juce::Graphics&) override;
};

class RazumovVocalChainAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              public juce::Timer,
                                              public juce::Label::Listener
{
public:
    explicit RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor&);
    ~RazumovVocalChainAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    static void styleRotary(juce::Slider&, int textBoxW, int textBoxH);

    void populateComboBoxes();
    void showAddModuleMenuForSlot(uint32_t referenceSlotId);
    void showParallelModuleMenuForSlot(uint32_t referenceSlotId);
    void showChainContextMenu(razumov::ui::ChainContextTarget target, uint32_t slotId, juce::Point<int> screenPos);
    void refreshModulePanelVisibility();
    void reloadModuleParamsFromProcessor();
    void syncChainStripAfterGraphEdit();
    void layoutMacroHeroRow(juce::Rectangle<int> area);
    /** eqPanelHeight: height of ReEq panel when visible (used for total content height). */
    int computeModuleContentHeight(int eqPanelHeight) const noexcept;
    void layoutModuleViewport(int viewportWidth, int viewportHeight);
    void refreshRotaryStyles();
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    void onMacroLearnSliderPressed(int macroIndex);
    void assignMacroFromLearn(const char* paramId);
    void updateMacroLearnVisuals();
    void refreshMacroLabelsFromProcessor();
    void refreshEqAuxSliders();
    void wireMacroValueCallbacks();

    int scaled(int base) const noexcept { return juce::roundToInt((float) base * uiScale_); }

    RazumovVocalChainAudioProcessor& processor;
    razumov::ui::ChainStripComponent chainStrip;

    juce::ComboBox micProfileCombo;
    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::Label macroSectionLabel;

    juce::Label moduleTitleLabel;
    juce::Label moduleHintLabel;

    juce::Viewport viewport;
    juce::Component content;
    ModuleSectionBackdrop moduleSectionBackdrop;

    struct MacroLearnSlider : public juce::Slider
    {
        void setLearnContext(RazumovVocalChainAudioProcessorEditor& ed, int macroIndex) noexcept;
        void mouseDown(const juce::MouseEvent& e) override;

    private:
        RazumovVocalChainAudioProcessorEditor* parent_ { nullptr };
        int macroIndex_ { 0 };
    };

    struct SpectrumPanel : public juce::Component
    {
        void updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId);
        void paint(juce::Graphics& g) override;

    private:
        std::array<float, (size_t) razumov::graph::ISpectrumSource::kSpectrumBins> bins_{};
    };

    /** Opto/FET/VCA: горизонтальный GR 0...24 dB. */
    struct GrMeterBar : public juce::Component
    {
        void setDb(float db) noexcept;
        void paint(juce::Graphics& g) override;

    private:
        float db_ { 0.f };
    };

    /** Spectral compressor: полоса сайдчейна, порог (желтый), уровень (cyan), GR. */
    struct SpectralCompPanel : public juce::Component
    {
        void updateFrom(RazumovVocalChainAudioProcessor& proc, uint32_t slotId);
        void paint(juce::Graphics& g) override;

    private:
        std::array<float, 256> in_{};
        std::array<float, 256> red_{};
        bool hasData_ { false };
        float centerHz_ { 2000.f };
        float q_ { 1.2f };
        float thrDb_ { -24.f };
        float envDb_ { -120.f };
        double sampleRate_ { 48000.0 };
    };

    struct ModuleTargetSlider : public juce::Slider
    {
        void setTargetContext(RazumovVocalChainAudioProcessorEditor& ed, const char* paramId) noexcept;
        void mouseDown(const juce::MouseEvent& e) override;

    private:
        RazumovVocalChainAudioProcessorEditor* parent_ { nullptr };
        const char* paramId_ { nullptr };
    };

    MacroLearnSlider macroGlueSlider;
    MacroLearnSlider macroAirSlider;
    MacroLearnSlider macroSibilanceSlider;
    MacroLearnSlider macroPresenceSlider;
    MacroLearnSlider macroPunchSlider;
    MacroLearnSlider macroBodySlider;
    MacroLearnSlider macroSmoothSlider;
    MacroLearnSlider macroDensitySlider;
    juce::Label macroGlueLabel;
    juce::Label macroAirLabel;
    juce::Label macroSibilanceLabel;
    juce::Label macroPresenceLabel;
    juce::Label macroPunchLabel;
    juce::Label macroBodyLabel;
    juce::Label macroSmoothLabel;
    juce::Label macroDensityLabel;

    juce::ToggleButton micBypassBtn;
    juce::ToggleButton spectralBypassBtn;

    ModuleTargetSlider micAmountSlider;
    ModuleTargetSlider gainSlider;
    ModuleTargetSlider lowpassSlider;
    ModuleTargetSlider deessCrossSlider;
    ModuleTargetSlider deessThreshSlider;
    ModuleTargetSlider deessRatioSlider;
    ModuleTargetSlider optoThreshSlider;
    ModuleTargetSlider optoRatioSlider;
    ModuleTargetSlider optoMakeupSlider;
    ModuleTargetSlider fetThreshSlider;
    ModuleTargetSlider fetRatioSlider;
    ModuleTargetSlider fetMakeupSlider;
    ModuleTargetSlider vcaThreshSlider;
    ModuleTargetSlider vcaRatioSlider;
    ModuleTargetSlider vcaMakeupSlider;
    ModuleTargetSlider exciterDriveSlider;
    ModuleTargetSlider exciterMixSlider;
    ModuleTargetSlider spectralMixSlider;
    ModuleTargetSlider spectralThreshSlider;
    ModuleTargetSlider spectralRatioSlider;
    ModuleTargetSlider spectralScFreqSlider;
    ModuleTargetSlider spectralScQSlider;
    ModuleTargetSlider spectralAttackSlider;
    ModuleTargetSlider spectralReleaseSlider;

    SpectrumPanel spectrumPanel;
    GrMeterBar grMeterBar;
    SpectralCompPanel spectralCompPanel;
    juce::ToggleButton eqBypassToggle;
    ReEqPanelComponent reEqPanel;
    ModuleTargetSlider eqSelFreqSlider;
    ModuleTargetSlider eqSelGainSlider;
    ModuleTargetSlider eqSelQSlider;
    ModuleTargetSlider eqSelSlopeSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> micProfileAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroGlueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAirAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroSibilanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroPresenceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroPunchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroBodyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroSmoothAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroDensityAttachment;

    std::unique_ptr<VocalChainerLookAndFeel> laf;

    float uiScale_ { 1.f };
    uint32_t selectedSlotId_ { 0 };

    /** OpenGL MSAA for smoother strokes (same pattern as Razumov ShaperX editor). */
    juce::OpenGLContext openGLContext_;
    int macroLearnPendingIndex_ { -1 };
    bool syncingMacroFromModule_ { false };

    /** Section backgrounds (updated in resized; used in paint for hierarchy). */
    juce::Rectangle<int> sectionMacro_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessorEditor)
};
