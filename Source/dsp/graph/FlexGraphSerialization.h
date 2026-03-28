#pragma once

#include "FlexGraphDesc.h"
#include <juce_data_structures/juce_data_structures.h>

namespace razumov::graph
{

inline constexpr const char* flexGraphValueTreeType = "FlexGraph";
inline constexpr const char* flexSlotValueTreeType = "Slot";
inline constexpr const char* flexBranchValueTreeType = "Branch";

/** Сериализация сегмента в дочерний ValueTree (без родителя PARAMS). nextSlotCounter — следующий свободный id для новых слотов. */
juce::ValueTree flexSegmentDescToValueTree(const FlexSegmentDesc& seg, uint32_t nextSlotCounter);

/** Разбор; при ошибке возвращает false. nextSlotCounterOut — из файла (может быть nullptr). */
bool valueTreeToFlexSegmentDesc(const juce::ValueTree& tree, FlexSegmentDesc& out, uint32_t* nextSlotCounterOut) noexcept;

/** Максимальный slotId в дереве (0 если пусто). */
uint32_t maxSlotIdInSegment(const FlexSegmentDesc& seg) noexcept;

/** true, если хотя бы один slotId == 0 (старые/битые данные). */
bool flexGraphNeedsSlotIdAssignment(const FlexSegmentDesc& seg) noexcept;

/** Назначить уникальные ненулевые id всем слотам ( preorder, счётчик с 1 ). */
void assignUniqueSlotIds(FlexSegmentDesc& seg, uint32_t& nextId) noexcept;

/** Назначить id поддереву (новый split / модуль с нулевыми id). */
void assignSlotIdsForSubtree(FlexSlotDesc& slot, uint32_t& nextId) noexcept;

} // namespace razumov::graph
