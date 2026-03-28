#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

#include "dsp/graph/AudioNode.h"
#include "ui/ChainStripComponent.h"

class RazumovVocalChainAudioProcessor;

class RazumovVocalChainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor&);
    ~RazumovVocalChainAudioProcessorEditor() override = default;

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

    juce::Label micProfileLabel;
    juce::ComboBox micProfileCombo;
    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::Label chainLabel;
    juce::ComboBox chainCombo;

    juce::TextButton bypassSlotBtn { {}, "Bypass" };
    juce::TextButton removeSlotBtn { {}, "Remove" };
    juce::TextButton moveLeftBtn { {}, "<" };
    juce::TextButton moveRightBtn { {}, ">" };
    juce::TextButton addModuleBtn { {}, "Add..." };

    juce::Label moduleTitleLabel;
    juce::Label moduleHintLabel;

    juce::Viewport viewport;
    juce::Component content;

    juce::Slider macroGlueSlider;
    juce::Slider macroAirSlider;
    juce::Slider macroSibilanceSlider;
    juce::Slider macroPresenceSlider;
    juce::Label macroGlueLabel;
    juce::Label macroAirLabel;
    juce::Label macroSibilanceLabel;
    juce::Label macroPresenceLabel;

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

    uint32_t selectedSlotId_ { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessorEditor)
};
