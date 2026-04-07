#pragma once

#include "ChainStripLayout.h"
#include "dsp/graph/FlexGraphDesc.h"
#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class RazumovVocalChainAudioProcessor;

namespace razumov::ui
{

/** Что под курсором при вызове контекстного меню (ПКМ). */
enum class ChainContextTarget : uint8_t
{
    ModuleCard = 0,
    SerialPlus,
    ParallelPlus,
    ParallelFromHere
};

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
    std::function<void(uint32_t slotId)> onRequestRemoveSlot;

    /** Right-click: add/remove/duplicate live in the menu instead of a toolbar row. */
    std::function<void(ChainContextTarget target, uint32_t slotId, juce::Point<int> screenPosition)> onChainContextMenu;

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
    juce::Point<float> dragLast_ {};
    uint32_t deleteHoverSlotId_ { 0 };
};

} // namespace razumov::ui
