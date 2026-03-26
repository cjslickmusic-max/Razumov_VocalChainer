#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace razumov::ui
{

/**
 * Визуализация фиксированной цепочки (read-only): карточки и стрелки в духе Razumov ShaperX «normal» chain view.
 * Не редактирует граф — только отражает выбранный chainProfile (Full / Compact / FET-forward).
 */
class ChainStripComponent : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ChainStripComponent(juce::AudioProcessorValueTreeState& apvts);
    ~ChainStripComponent() override;

    void paint(juce::Graphics&) override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    int getChainProfileIndex() const;

    juce::AudioProcessorValueTreeState& apvts_;
};

} // namespace razumov::ui
