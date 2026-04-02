#pragma once

#include "MacroRouting.h"
#include "Phase3RealtimeParams.h"
#include <array>
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

    float getByIndex(int index) const noexcept
    {
        switch (index)
        {
            case 0:
                return glue01;
            case 1:
                return air01;
            case 2:
                return sibil01;
            case 3:
                return presence01;
            case 4:
                return punch01;
            case 5:
                return body01;
            case 6:
                return smooth01;
            case 7:
                return density01;
            default:
                return 0.5f;
        }
    }
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

    /** Copy per-slot DSP/UI values (used after duplicating a graph module). */
    void copySlotParamsFromTo(uint32_t fromSlotId, uint32_t toSlotId);

    juce::ValueTree toValueTree() const;
    void fromValueTree(const juce::ValueTree& v);

    static const juce::Identifier moduleParamsTreeType;

    void setMacroTarget(int macroIndex, uint32_t slotId, MacroTargetParam param) noexcept;
    void clearMacroTarget(int macroIndex) noexcept;
    uint32_t getMacroTargetSlot(int macroIndex) const noexcept;
    MacroTargetParam getMacroTargetParam(int macroIndex) const noexcept;
    /** -1 if no macro maps to this slot+param. */
    int findMacroIndexForTarget(uint32_t slotId, MacroTargetParam param) const noexcept;

    void setMacroDisplayName(int macroIndex, juce::String name);
    juce::String getMacroDisplayName(int macroIndex) const;

private:
    int findSlotIndex(uint32_t slotId) const noexcept;
    detail::ModuleSlotBlock* findOrNull(uint32_t slotId) noexcept;
    const detail::ModuleSlotBlock* findOrNull(uint32_t slotId) const noexcept;
    void ensureSlotWithDefaults(uint32_t slotId);
    void removeSlotAtIndex(size_t idx);

    std::vector<uint32_t> slotIds_;
    std::vector<std::unique_ptr<detail::ModuleSlotBlock>> blocks_;

    std::array<std::atomic<uint32_t>, 8> macroTargetSlot_ {};
    std::array<std::atomic<uint32_t>, 8> macroTargetParam_ {};
    std::array<juce::String, 8> macroDisplayNames_ {};
};

} // namespace razumov::params
