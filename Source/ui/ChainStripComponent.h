#pragma once

#include "dsp/graph/FlexGraphDesc.h"
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class RazumovVocalChainAudioProcessor;

namespace razumov::ui
{

/** Полоса цепочки: клик по карточке выбирает slotId; подсветка выбранного. */
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

    std::vector<razumov::graph::ChainStripItem> items_;
    std::vector<juce::Rectangle<int>> hitRects_;
    uint32_t selectedSlotId_ { 0 };
};

} // namespace razumov::ui
