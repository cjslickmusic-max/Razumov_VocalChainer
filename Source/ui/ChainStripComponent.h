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
class ChainStripComponent : public juce::Component
{
public:
    explicit ChainStripComponent(RazumovVocalChainAudioProcessor& processor);
    ~ChainStripComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    void syncFromProcessor();
    void setSelectedSlotId(uint32_t id) noexcept;
    uint32_t getSelectedSlotId() const noexcept { return selectedSlotId_; }

    std::function<void(uint32_t slotId)> onSlotSelected;
    std::function<void(uint32_t slotId)> onRequestAddSerialAfter;
    std::function<void(uint32_t slotId)> onRequestParallelBranch;
    /** Swap order of two root-level modules (strip UI); host validates. */
    std::function<void(uint32_t slotIdA, uint32_t slotIdB)> onRequestSwapRootModules;

private:
    void rebuildLayout();
    static juce::Rectangle<float> bypassPillBounds(juce::Rectangle<float> card) noexcept;
    uint32_t hitTestCardSlotAt(juce::Point<float> p) const noexcept;

    RazumovVocalChainAudioProcessor& processor_;
    ChainStripLayout layout_;
    uint32_t selectedSlotId_ { 0 };

    bool dragging_ { false };
    uint32_t dragSlotId_ { 0 };
    juce::Point<float> dragStart_ {};
};

} // namespace razumov::ui
