#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class RazumovVocalChainAudioProcessor;

namespace razumov::ui
{

/**
 * Визуализация цепочки (read-only): карточки из FlexSegmentDesc процессора.
 * Обновляется при смене chainProfile (тот же APVTS listener).
 */
class ChainStripComponent : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ChainStripComponent(RazumovVocalChainAudioProcessor& processor);
    ~ChainStripComponent() override;

    void paint(juce::Graphics&) override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    RazumovVocalChainAudioProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;
};

} // namespace razumov::ui
