#pragma once

#include "dsp/graph/FlexGraphDesc.h"
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

namespace razumov::ui
{

/** Карточка на полосе цепочки (модуль, split, merge). */
struct ChainStripLayoutCard
{
    juce::Rectangle<float> bounds;
    uint32_t slotId { 0 };
    bool bypassed { false };
    juce::String label;
    bool selectable { true };
    bool isMergeNode { false };
    bool showSerialPlus { false };
    bool showParallelPlus { false };
    juce::Rectangle<float> serialPlusBounds;
    juce::Rectangle<float> parallelPlusBounds;

    bool showDeleteButton { false };
    juce::Rectangle<float> deleteButtonBounds;
};

/** Соединительная линия (серийная стрелка или fork/join). */
struct ChainStripLayoutWire
{
    juce::Point<float> a;
    juce::Point<float> b;
};

struct ChainStripLayout
{
    std::vector<ChainStripLayoutCard> cards;
    std::vector<ChainStripLayoutWire> wires;
    /** Parallel merge junctions on the main row (for Sum marker). */
    std::vector<juce::Point<float>> mergePoints;
    float totalWidth { 0 };
    float totalHeight { 0 };
};

/**
 * Раскладка по дереву FlexSegmentDesc: параллельные ветки рядом, merge на основной линии,
 * провода fork/join. При нехватке ширины масштабируется по X (вся схема).
 */
ChainStripLayout computeChainStripLayout(const razumov::graph::FlexSegmentDesc& root,
                                         float availableWidth,
                                         float stripHeight) noexcept;

} // namespace razumov::ui
