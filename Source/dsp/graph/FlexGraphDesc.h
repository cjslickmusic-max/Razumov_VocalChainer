#pragma once

#include "AudioNode.h"
#include <cstdint>
#include <optional>
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

/** Элемент полосы цепочки (клик → выбор слота). */
struct ChainStripItem
{
    uint32_t slotId { 0 };
    juce::String label;
    bool bypassed { false };
};

/** Максимум веток у любого Split в дереве (для пула буферов / MergeDelayPad). */
int computeMaxSplitBreadth(const FlexSegmentDesc& root) noexcept;

/** Плоские подписи для полосы цепочки (ASCII). */
juce::StringArray segmentDescToChainStripLabels(const FlexSegmentDesc& root);

/** Плоский DFS: модули и split-узлы в порядке обхода. */
std::vector<ChainStripItem> segmentDescToChainStripItems(const FlexSegmentDesc& root);

/** Редактирование дерева описания (message thread). */
bool setSlotBypassById(FlexSegmentDesc& root, uint32_t slotId, bool bypassed) noexcept;
bool removeSlotById(FlexSegmentDesc& root, uint32_t slotId) noexcept;
/** Только для module; split / не найден — nullopt. */
std::optional<AudioNodeKind> queryModuleKindForSlotId(const FlexSegmentDesc& root, uint32_t slotId) noexcept;

/** true, если slotId указывает на Split (в т.ч. во вложенном сегменте). */
bool queryIsParallelSplitSlot(const FlexSegmentDesc& root, uint32_t slotId) noexcept;

int findRootSlotIndexContainingId(const FlexSegmentDesc& root, uint32_t slotId) noexcept;

} // namespace razumov::graph
