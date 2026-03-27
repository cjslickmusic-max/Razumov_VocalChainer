#pragma once

#include "AudioNode.h"
#include <cstdint>
#include <vector>

#include <juce_core/juce_core.h>

namespace razumov::graph
{

/** POD-описание слота для UI/сериализации и сборки FlexGraphPlan (без владения DSP). */
enum class FlexSlotDescType : uint8_t
{
    Module = 0,
    Split
};

struct FlexSlotDesc
{
    FlexSlotDescType descType { FlexSlotDescType::Module };
    uint32_t slotId { 0 };
    bool bypassed { false };
    juce::String uiLabel;

    AudioNodeKind kind { AudioNodeKind::Gain };
    float gainLinear { 1.0f };
    float filterCutoffHz { 20000.0f };
    int latencySamples { 0 };

    std::vector<std::vector<FlexSlotDesc>> branches;
};

using FlexSegmentDesc = std::vector<FlexSlotDesc>;

/** Максимум веток у любого Split в дереве (для пула буферов / MergeDelayPad). */
int computeMaxSplitBreadth(const FlexSegmentDesc& root) noexcept;

/** Плоские подписи для полосы цепочки (ASCII). */
juce::StringArray segmentDescToChainStripLabels(const FlexSegmentDesc& root);

} // namespace razumov::graph
