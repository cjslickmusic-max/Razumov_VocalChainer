#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class RazumovVocalChainAudioProcessor;

class RazumovVocalChainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor&);
    ~RazumovVocalChainAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RazumovVocalChainAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessorEditor)
};
