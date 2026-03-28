#pragma once

#include "ChainStripLayout.h"
#include "dsp/graph/FlexGraphDesc.h"
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class RazumovVocalChainAudioProcessor;

namespace razumov::ui
{

/** Полоса цепочки: дерево графа, параллель ветками, fork/join; клик по карточке -> slotId. */
class ChainStripComponent : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ChainStripComponent(RazumovVocalChainAudioProcessor& processor);
    ~ChainStripComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    void syncFromProcessor();
    void setSelectedSlotId(uint32_t id) noexcept;
    uint32_t getSelectedSlotId() const noexcept { return selectedSlotId_; }

    std::function<void(uint32_t slotId)> onSlotSelected;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void rebuildLayout();

    RazumovVocalChainAudioProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;

    ChainStripLayout layout_;
    uint32_t selectedSlotId_ { 0 };
};

} // namespace razumov::ui
