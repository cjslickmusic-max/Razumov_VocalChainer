#pragma once

#include "Phase3RealtimeParams.h"
#include <juce_data_structures/juce_data_structures.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace razumov::params
{
namespace detail
{
class ModuleSlotBlock;
}

/** Макросы 0...1 (как в APVTS), читаются в начале processBlock. */
struct MacroAudioState
{
    float glue01 { 0.5f };
    float air01 { 0.5f };
    float sibil01 { 0.5f };
    float presence01 { 0.5f };
    float punch01 { 0.5f };
    float body01 { 0.5f };
    float smooth01 { 0.5f };
    float density01 { 0.5f };
};

/** Сырые значения на слот (как бывшие параметры APVTS), потокобезопасно для audio thread. */
class ModuleParamsRuntime
{
public:
    ModuleParamsRuntime();
    ~ModuleParamsRuntime();

    void clear() noexcept;

    /** Синхронизация с графом: новые слоты — defaults, лишние slotId — удалить. */
    void syncWithGraphSlotIds(const std::vector<uint32_t>& moduleSlotIds);

    void fillSlot(uint32_t slotId, const MacroAudioState& macros, Phase3RealtimeParams& out) const;

    float getFloat(uint32_t slotId, const juce::String& paramId) const;
    void setFloat(uint32_t slotId, const juce::String& paramId, float value);

    bool getBool(uint32_t slotId, const juce::String& paramId) const;
    void setBool(uint32_t slotId, const juce::String& paramId, bool value);

    void seedAllSlotsWithSameParams(const Phase3RealtimeParams& raw);

    juce::ValueTree toValueTree() const;
    void fromValueTree(const juce::ValueTree& v);

    static const juce::Identifier moduleParamsTreeType;

private:
    int findSlotIndex(uint32_t slotId) const noexcept;
    detail::ModuleSlotBlock* findOrNull(uint32_t slotId) noexcept;
    const detail::ModuleSlotBlock* findOrNull(uint32_t slotId) const noexcept;
    void ensureSlotWithDefaults(uint32_t slotId);
    void removeSlotAtIndex(size_t idx);

    std::vector<uint32_t> slotIds_;
    std::vector<std::unique_ptr<detail::ModuleSlotBlock>> blocks_;
};

void applyMacroOffsetsToPhase3(Phase3RealtimeParams& p, const MacroAudioState& macros) noexcept;

} // namespace razumov::params
