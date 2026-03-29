#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

#include "dsp/graph/AudioNode.h"
#include "ui/ChainStripComponent.h"

class RazumovVocalChainAudioProcessor;

struct VocalChainerLookAndFeel;

/** Область под будущую графику микрофона; клик по превью открывает список профиля. */
struct MicProfilePanel : juce::Component
{
    std::function<void()> onPreviewClicked;
    void resized() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::Rectangle<int> previewBounds_;
};

struct ModuleSectionBackdrop : juce::Component
{
    void paint(juce::Graphics&) override;
};

class RazumovVocalChainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor&);
    ~RazumovVocalChainAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static void styleRotary(juce::Slider&);

    void populateComboBoxes();
    void refreshModulePanelVisibility();
    void reloadModuleParamsFromProcessor();
    void syncChainStripAfterGraphEdit();
    void layoutGlobalSection(juce::Rectangle<int> area);
    void layoutModuleViewport(int viewportWidth);

    RazumovVocalChainAudioProcessor& processor;
    razumov::ui::ChainStripComponent chainStrip;

    MicProfilePanel micProfilePanel;
    juce::ComboBox micProfileCombo;
    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::Label chainLabel;
    juce::ComboBox chainCombo;
    juce::Label macroSectionLabel;

    juce::TextButton bypassSlotBtn { {}, "Bypass" };
    juce::TextButton removeSlotBtn { {}, "Remove" };
    juce::TextButton moveLeftBtn { {}, "<" };
    juce::TextButton moveRightBtn { {}, ">" };
    juce::TextButton addModuleBtn { {}, "Add..." };

    juce::Label moduleTitleLabel;
    juce::Label moduleHintLabel;

    juce::Viewport viewport;
    juce::Component content;
    ModuleSectionBackdrop moduleSectionBackdrop;

    juce::Slider macroGlueSlider;
    juce::Slider macroAirSlider;
    juce::Slider macroSibilanceSlider;
    juce::Slider macroPresenceSlider;
    juce::Slider macroPunchSlider;
    juce::Slider macroBodySlider;
    juce::Slider macroSmoothSlider;
    juce::Slider macroDensitySlider;
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

    juce::Slider micAmountSlider;
    juce::Slider gainSlider;
    juce::Slider lowpassSlider;
    juce::Slider deessCrossSlider;
    juce::Slider deessThreshSlider;
    juce::Slider deessRatioSlider;
    juce::Slider optoThreshSlider;
    juce::Slider optoRatioSlider;
    juce::Slider optoMakeupSlider;
    juce::Slider fetThreshSlider;
    juce::Slider fetRatioSlider;
    juce::Slider fetMakeupSlider;
    juce::Slider vcaThreshSlider;
    juce::Slider vcaRatioSlider;
    juce::Slider vcaMakeupSlider;
    juce::Slider exciterDriveSlider;
    juce::Slider exciterMixSlider;
    juce::Slider spectralMixSlider;
    juce::Slider spectralThreshSlider;
    juce::Slider spectralRatioSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> micProfileAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> chainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroGlueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAirAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroSibilanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroPresenceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroPunchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroBodyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroSmoothAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroDensityAttachment;

    std::unique_ptr<VocalChainerLookAndFeel> laf;

    uint32_t selectedSlotId_ { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessorEditor)
};
