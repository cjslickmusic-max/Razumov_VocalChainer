#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

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

    RazumovVocalChainAudioProcessor& processor;
    razumov::ui::ChainStripComponent chainStrip;

    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::Label chainLabel;
    juce::ComboBox chainCombo;

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

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> micBypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> spectralBypassAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> micAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowpassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessCrossAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> optoThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> optoRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> optoMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fetThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fetRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fetMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vcaMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> exciterDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> exciterMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spectralMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spectralThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spectralRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> chainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroGlueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAirAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroSibilanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroPresenceAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessorEditor)
};
