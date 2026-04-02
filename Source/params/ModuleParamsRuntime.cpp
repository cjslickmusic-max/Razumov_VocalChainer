#include "ModuleParamsRuntime.h"
#include "MacroRouting.h"
#include "ParamIDs.h"

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

namespace razumov::params
{
namespace detail
{

struct ModuleSlotBlock
{
    std::atomic<float> micBypass { 0.0f };
    std::atomic<float> micAmount { 1.0f };
    std::atomic<float> gainDb { 0.0f };
    std::atomic<float> lowpassHz { 20000.0f };
    std::atomic<float> deessCrossoverHz { 6000.0f };
    std::atomic<float> deessThresholdDb { -20.0f };
    std::atomic<float> deessRatio { 3.0f };
    std::atomic<float> optoThresholdDb { -12.0f };
    std::atomic<float> optoRatio { 3.0f };
    std::atomic<float> optoMakeupDb { 0.0f };
    std::atomic<float> fetThresholdDb { -12.0f };
    std::atomic<float> fetRatio { 4.0f };
    std::atomic<float> fetMakeupDb { 0.0f };
    std::atomic<float> vcaThresholdDb { -12.0f };
    std::atomic<float> vcaRatio { 2.5f };
    std::atomic<float> vcaMakeupDb { 0.0f };
    std::atomic<float> exciterDrive { 1.5f };
    std::atomic<float> exciterMix { 0.15f };
    std::atomic<float> spectralBypass { 0.0f };
    std::atomic<float> spectralMix { 0.75f };
    std::atomic<float> spectralThresholdDb { -24.0f };
    std::atomic<float> spectralRatio { 3.0f };
};

inline void copyModuleSlotBlockState(const ModuleSlotBlock& src, ModuleSlotBlock& dst)
{
    dst.micBypass.store(src.micBypass.load());
    dst.micAmount.store(src.micAmount.load());
    dst.gainDb.store(src.gainDb.load());
    dst.lowpassHz.store(src.lowpassHz.load());
    dst.deessCrossoverHz.store(src.deessCrossoverHz.load());
    dst.deessThresholdDb.store(src.deessThresholdDb.load());
    dst.deessRatio.store(src.deessRatio.load());
    dst.optoThresholdDb.store(src.optoThresholdDb.load());
    dst.optoRatio.store(src.optoRatio.load());
    dst.optoMakeupDb.store(src.optoMakeupDb.load());
    dst.fetThresholdDb.store(src.fetThresholdDb.load());
    dst.fetRatio.store(src.fetRatio.load());
    dst.fetMakeupDb.store(src.fetMakeupDb.load());
    dst.vcaThresholdDb.store(src.vcaThresholdDb.load());
    dst.vcaRatio.store(src.vcaRatio.load());
    dst.vcaMakeupDb.store(src.vcaMakeupDb.load());
    dst.exciterDrive.store(src.exciterDrive.load());
    dst.exciterMix.store(src.exciterMix.load());
    dst.spectralBypass.store(src.spectralBypass.load());
    dst.spectralMix.store(src.spectralMix.load());
    dst.spectralThresholdDb.store(src.spectralThresholdDb.load());
    dst.spectralRatio.store(src.spectralRatio.load());
}

} // namespace detail

namespace
{

void storeFromPhase3(detail::ModuleSlotBlock& b, const Phase3RealtimeParams& p)
{
    b.micBypass.store(p.micBypass ? 1.0f : 0.0f);
    b.micAmount.store(p.micAmount);
    b.gainDb.store(juce::Decibels::gainToDecibels(p.gainLinear, -200.0f));
    b.lowpassHz.store(p.lowpassHz);
    b.deessCrossoverHz.store(p.deessCrossoverHz);
    b.deessThresholdDb.store(p.deessThresholdDb);
    b.deessRatio.store(p.deessRatio);
    b.optoThresholdDb.store(p.optoThresholdDb);
    b.optoRatio.store(p.optoRatio);
    b.optoMakeupDb.store(p.optoMakeupDb);
    b.fetThresholdDb.store(p.fetThresholdDb);
    b.fetRatio.store(p.fetRatio);
    b.fetMakeupDb.store(p.fetMakeupDb);
    b.vcaThresholdDb.store(p.vcaThresholdDb);
    b.vcaRatio.store(p.vcaRatio);
    b.vcaMakeupDb.store(p.vcaMakeupDb);
    b.exciterDrive.store(p.exciterDrive);
    b.exciterMix.store(p.exciterMix);
    b.spectralBypass.store(p.spectralBypass ? 1.0f : 0.0f);
    b.spectralMix.store(p.spectralMix);
    b.spectralThresholdDb.store(p.spectralThresholdDb);
    b.spectralRatio.store(p.spectralRatio);
}

void loadToPhase3(const detail::ModuleSlotBlock& b, Phase3RealtimeParams& out)
{
    out.micBypass = b.micBypass.load() > 0.5f;
    out.micAmount = b.micAmount.load();
    out.gainLinear = juce::Decibels::decibelsToGain(b.gainDb.load());
    out.lowpassHz = b.lowpassHz.load();
    out.deessCrossoverHz = b.deessCrossoverHz.load();
    out.deessThresholdDb = b.deessThresholdDb.load();
    out.deessRatio = b.deessRatio.load();
    out.optoThresholdDb = b.optoThresholdDb.load();
    out.optoRatio = b.optoRatio.load();
    out.optoMakeupDb = b.optoMakeupDb.load();
    out.fetThresholdDb = b.fetThresholdDb.load();
    out.fetRatio = b.fetRatio.load();
    out.fetMakeupDb = b.fetMakeupDb.load();
    out.vcaThresholdDb = b.vcaThresholdDb.load();
    out.vcaRatio = b.vcaRatio.load();
    out.vcaMakeupDb = b.vcaMakeupDb.load();
    out.exciterDrive = b.exciterDrive.load();
    out.exciterMix = b.exciterMix.load();
    out.spectralBypass = b.spectralBypass.load() > 0.5f;
    out.spectralMix = b.spectralMix.load();
    out.spectralThresholdDb = b.spectralThresholdDb.load();
    out.spectralRatio = b.spectralRatio.load();
}

void initDefaults(detail::ModuleSlotBlock& b)
{
    Phase3RealtimeParams d;
    storeFromPhase3(b, d);
}

} // namespace

const juce::Identifier ModuleParamsRuntime::moduleParamsTreeType { "ModuleParams" };

ModuleParamsRuntime::ModuleParamsRuntime()
{
    static const char* const kDefaultNames[8] = { "Glue", "Air", "Sibil", "Presence", "Punch", "Body", "Smooth", "Density" };
    for (int i = 0; i < 8; ++i)
    {
        macroTargetSlot_[(size_t) i].store(0u, std::memory_order_relaxed);
        macroTargetParam_[(size_t) i].store((uint32_t) MacroTargetParam::None, std::memory_order_relaxed);
        macroDisplayNames_[(size_t) i] = kDefaultNames[(size_t) i];
    }
}

ModuleParamsRuntime::~ModuleParamsRuntime() = default;

void ModuleParamsRuntime::clear() noexcept
{
    slotIds_.clear();
    blocks_.clear();
    for (int i = 0; i < 8; ++i)
    {
        macroTargetSlot_[(size_t) i].store(0u, std::memory_order_relaxed);
        macroTargetParam_[(size_t) i].store((uint32_t) MacroTargetParam::None, std::memory_order_relaxed);
    }
}

int ModuleParamsRuntime::findSlotIndex(uint32_t slotId) const noexcept
{
    for (size_t i = 0; i < slotIds_.size(); ++i)
        if (slotIds_[i] == slotId)
            return (int) i;
    return -1;
}

detail::ModuleSlotBlock* ModuleParamsRuntime::findOrNull(uint32_t slotId) noexcept
{
    const int i = findSlotIndex(slotId);
    return i < 0 ? nullptr : blocks_[(size_t) i].get();
}

const detail::ModuleSlotBlock* ModuleParamsRuntime::findOrNull(uint32_t slotId) const noexcept
{
    const int i = findSlotIndex(slotId);
    return i < 0 ? nullptr : blocks_[(size_t) i].get();
}

void ModuleParamsRuntime::removeSlotAtIndex(size_t idx)
{
    if (idx >= slotIds_.size())
        return;
    slotIds_.erase(slotIds_.begin() + (ptrdiff_t) idx);
    blocks_.erase(blocks_.begin() + (ptrdiff_t) idx);
}

void ModuleParamsRuntime::ensureSlotWithDefaults(uint32_t slotId)
{
    if (findSlotIndex(slotId) >= 0)
        return;
    slotIds_.push_back(slotId);
    auto u = std::make_unique<detail::ModuleSlotBlock>();
    initDefaults(*u);
    blocks_.push_back(std::move(u));
}

void ModuleParamsRuntime::syncWithGraphSlotIds(const std::vector<uint32_t>& moduleSlotIds)
{
    for (uint32_t id : moduleSlotIds)
        ensureSlotWithDefaults(id);

    for (int i = (int) slotIds_.size() - 1; i >= 0; --i)
    {
        const uint32_t sid = slotIds_[(size_t) i];
        const bool keep = std::find(moduleSlotIds.begin(), moduleSlotIds.end(), sid) != moduleSlotIds.end();
        if (!keep)
            removeSlotAtIndex((size_t) i);
    }
}

void ModuleParamsRuntime::fillSlot(uint32_t slotId, const MacroAudioState& macros, Phase3RealtimeParams& out) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        out = Phase3RealtimeParams {};
    else
        loadToPhase3(*b, out);

    for (int i = 0; i < 8; ++i)
    {
        const uint32_t ts = macroTargetSlot_[(size_t) i].load(std::memory_order_acquire);
        if (ts == 0 || ts != slotId)
            continue;
        const auto pk = (MacroTargetParam) macroTargetParam_[(size_t) i].load(std::memory_order_acquire);
        if (pk == MacroTargetParam::None)
            continue;
        applyMacroFullRangeToPhase3(out, pk, macros.getByIndex(i));
    }
}

float ModuleParamsRuntime::getFloat(uint32_t slotId, const juce::String& paramId) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return 0.0f;

    using namespace razumov::params;
    if (paramId == micAmount)
        return b->micAmount.load();
    if (paramId == gainDb)
        return b->gainDb.load();
    if (paramId == lowpassHz)
        return b->lowpassHz.load();
    if (paramId == deessCrossoverHz)
        return b->deessCrossoverHz.load();
    if (paramId == deessThresholdDb)
        return b->deessThresholdDb.load();
    if (paramId == deessRatio)
        return b->deessRatio.load();
    if (paramId == optoThresholdDb)
        return b->optoThresholdDb.load();
    if (paramId == optoRatio)
        return b->optoRatio.load();
    if (paramId == optoMakeupDb)
        return b->optoMakeupDb.load();
    if (paramId == fetThresholdDb)
        return b->fetThresholdDb.load();
    if (paramId == fetRatio)
        return b->fetRatio.load();
    if (paramId == fetMakeupDb)
        return b->fetMakeupDb.load();
    if (paramId == vcaThresholdDb)
        return b->vcaThresholdDb.load();
    if (paramId == vcaRatio)
        return b->vcaRatio.load();
    if (paramId == vcaMakeupDb)
        return b->vcaMakeupDb.load();
    if (paramId == exciterDrive)
        return b->exciterDrive.load();
    if (paramId == exciterMix)
        return b->exciterMix.load();
    if (paramId == spectralMix)
        return b->spectralMix.load();
    if (paramId == spectralThresholdDb)
        return b->spectralThresholdDb.load();
    if (paramId == spectralRatio)
        return b->spectralRatio.load();
    return 0.0f;
}

void ModuleParamsRuntime::setFloat(uint32_t slotId, const juce::String& paramId, float value)
{
    detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return;

    using namespace razumov::params;
    if (paramId == micAmount)
        b->micAmount.store(value);
    else if (paramId == gainDb)
        b->gainDb.store(value);
    else if (paramId == lowpassHz)
        b->lowpassHz.store(value);
    else if (paramId == deessCrossoverHz)
        b->deessCrossoverHz.store(value);
    else if (paramId == deessThresholdDb)
        b->deessThresholdDb.store(value);
    else if (paramId == deessRatio)
        b->deessRatio.store(value);
    else if (paramId == optoThresholdDb)
        b->optoThresholdDb.store(value);
    else if (paramId == optoRatio)
        b->optoRatio.store(value);
    else if (paramId == optoMakeupDb)
        b->optoMakeupDb.store(value);
    else if (paramId == fetThresholdDb)
        b->fetThresholdDb.store(value);
    else if (paramId == fetRatio)
        b->fetRatio.store(value);
    else if (paramId == fetMakeupDb)
        b->fetMakeupDb.store(value);
    else if (paramId == vcaThresholdDb)
        b->vcaThresholdDb.store(value);
    else if (paramId == vcaRatio)
        b->vcaRatio.store(value);
    else if (paramId == vcaMakeupDb)
        b->vcaMakeupDb.store(value);
    else if (paramId == exciterDrive)
        b->exciterDrive.store(value);
    else if (paramId == exciterMix)
        b->exciterMix.store(value);
    else if (paramId == spectralMix)
        b->spectralMix.store(value);
    else if (paramId == spectralThresholdDb)
        b->spectralThresholdDb.store(value);
    else if (paramId == spectralRatio)
        b->spectralRatio.store(value);
}

bool ModuleParamsRuntime::getBool(uint32_t slotId, const juce::String& paramId) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return false;

    using namespace razumov::params;
    if (paramId == micBypass)
        return b->micBypass.load() > 0.5f;
    if (paramId == spectralBypass)
        return b->spectralBypass.load() > 0.5f;
    return false;
}

void ModuleParamsRuntime::setBool(uint32_t slotId, const juce::String& paramId, bool value)
{
    detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return;

    using namespace razumov::params;
    const float v = value ? 1.0f : 0.0f;
    if (paramId == micBypass)
        b->micBypass.store(v);
    else if (paramId == spectralBypass)
        b->spectralBypass.store(v);
}

void ModuleParamsRuntime::seedAllSlotsWithSameParams(const Phase3RealtimeParams& p)
{
    for (size_t i = 0; i < blocks_.size(); ++i)
        if (blocks_[i] != nullptr)
            storeFromPhase3(*blocks_[i], p);
}

void ModuleParamsRuntime::copySlotParamsFromTo(uint32_t fromSlotId, uint32_t toSlotId)
{
    const detail::ModuleSlotBlock* src = findOrNull(fromSlotId);
    if (src == nullptr)
        return;
    ensureSlotWithDefaults(toSlotId);
    detail::ModuleSlotBlock* dst = findOrNull(toSlotId);
    if (dst == nullptr)
        return;
    detail::copyModuleSlotBlockState(*src, *dst);
}

juce::ValueTree ModuleParamsRuntime::toValueTree() const
{
    juce::ValueTree root(moduleParamsTreeType);
    for (size_t i = 0; i < slotIds_.size(); ++i)
    {
        const uint32_t sid = slotIds_[i];
        const auto* b = blocks_[i].get();
        if (b == nullptr)
            continue;

        juce::ValueTree slot("Slot");
        using namespace razumov::params;
        slot.setProperty("slotId", (int) sid, nullptr);
        slot.setProperty(micBypass, b->micBypass.load(), nullptr);
        slot.setProperty(micAmount, b->micAmount.load(), nullptr);
        slot.setProperty(gainDb, b->gainDb.load(), nullptr);
        slot.setProperty(lowpassHz, b->lowpassHz.load(), nullptr);
        slot.setProperty(deessCrossoverHz, b->deessCrossoverHz.load(), nullptr);
        slot.setProperty(deessThresholdDb, b->deessThresholdDb.load(), nullptr);
        slot.setProperty(deessRatio, b->deessRatio.load(), nullptr);
        slot.setProperty(optoThresholdDb, b->optoThresholdDb.load(), nullptr);
        slot.setProperty(optoRatio, b->optoRatio.load(), nullptr);
        slot.setProperty(optoMakeupDb, b->optoMakeupDb.load(), nullptr);
        slot.setProperty(fetThresholdDb, b->fetThresholdDb.load(), nullptr);
        slot.setProperty(fetRatio, b->fetRatio.load(), nullptr);
        slot.setProperty(fetMakeupDb, b->fetMakeupDb.load(), nullptr);
        slot.setProperty(vcaThresholdDb, b->vcaThresholdDb.load(), nullptr);
        slot.setProperty(vcaRatio, b->vcaRatio.load(), nullptr);
        slot.setProperty(vcaMakeupDb, b->vcaMakeupDb.load(), nullptr);
        slot.setProperty(exciterDrive, b->exciterDrive.load(), nullptr);
        slot.setProperty(exciterMix, b->exciterMix.load(), nullptr);
        slot.setProperty(spectralBypass, b->spectralBypass.load(), nullptr);
        slot.setProperty(spectralMix, b->spectralMix.load(), nullptr);
        slot.setProperty(spectralThresholdDb, b->spectralThresholdDb.load(), nullptr);
        slot.setProperty(spectralRatio, b->spectralRatio.load(), nullptr);
        root.appendChild(slot, nullptr);
    }

    juce::ValueTree mr("MacroRouting");
    for (int i = 0; i < 8; ++i)
    {
        juce::ValueTree row("Macro");
        row.setProperty("i", i, nullptr);
        row.setProperty("slot", (int) macroTargetSlot_[(size_t) i].load(std::memory_order_relaxed), nullptr);
        row.setProperty("param", (int) macroTargetParam_[(size_t) i].load(std::memory_order_relaxed), nullptr);
        row.setProperty("name", macroDisplayNames_[(size_t) i], nullptr);
        mr.appendChild(row, nullptr);
    }
    root.appendChild(mr, nullptr);
    return root;
}

void ModuleParamsRuntime::fromValueTree(const juce::ValueTree& v)
{
    if (!v.isValid() || v.getType() != moduleParamsTreeType)
        return;

    for (int i = 0; i < v.getNumChildren(); ++i)
    {
        auto slot = v.getChild(i);
        if (!slot.isValid() || slot.getType().toString() != "Slot")
            continue;
        const int sid = (int) slot.getProperty("slotId", 0);
        if (sid <= 0)
            continue;
        ensureSlotWithDefaults((uint32_t) sid);
        detail::ModuleSlotBlock* b = findOrNull((uint32_t) sid);
        if (b == nullptr)
            continue;

        using namespace razumov::params;
        auto gf = [&](const char* id, float defV) {
            return (float) slot.getProperty(id, defV);
        };
        b->micBypass.store(gf(micBypass, 0.0f));
        b->micAmount.store(gf(micAmount, 1.0f));
        b->gainDb.store(gf(gainDb, 0.0f));
        b->lowpassHz.store(gf(lowpassHz, 20000.0f));
        b->deessCrossoverHz.store(gf(deessCrossoverHz, 6000.0f));
        b->deessThresholdDb.store(gf(deessThresholdDb, -20.0f));
        b->deessRatio.store(gf(deessRatio, 3.0f));
        b->optoThresholdDb.store(gf(optoThresholdDb, -12.0f));
        b->optoRatio.store(gf(optoRatio, 3.0f));
        b->optoMakeupDb.store(gf(optoMakeupDb, 0.0f));
        b->fetThresholdDb.store(gf(fetThresholdDb, -12.0f));
        b->fetRatio.store(gf(fetRatio, 4.0f));
        b->fetMakeupDb.store(gf(fetMakeupDb, 0.0f));
        b->vcaThresholdDb.store(gf(vcaThresholdDb, -12.0f));
        b->vcaRatio.store(gf(vcaRatio, 2.5f));
        b->vcaMakeupDb.store(gf(vcaMakeupDb, 0.0f));
        b->exciterDrive.store(gf(exciterDrive, 1.5f));
        b->exciterMix.store(gf(exciterMix, 0.15f));
        b->spectralBypass.store(gf(spectralBypass, 0.0f));
        b->spectralMix.store(gf(spectralMix, 0.75f));
        b->spectralThresholdDb.store(gf(spectralThresholdDb, -24.0f));
        b->spectralRatio.store(gf(spectralRatio, 3.0f));
    }

    juce::ValueTree mr = v.getChildWithName("MacroRouting");
    if (mr.isValid())
    {
        for (int c = 0; c < mr.getNumChildren(); ++c)
        {
            auto row = mr.getChild(c);
            if (!row.isValid() || row.getType().toString() != "Macro")
                continue;
            const int idx = (int) row.getProperty("i", -1);
            if (idx < 0 || idx > 7)
                continue;
            macroTargetSlot_[(size_t) idx].store((uint32_t)(int) row.getProperty("slot", 0), std::memory_order_relaxed);
            macroTargetParam_[(size_t) idx].store((uint32_t)(int) row.getProperty("param", 0), std::memory_order_relaxed);
            const juce::String nm = row.getProperty("name").toString();
            if (nm.isNotEmpty())
                macroDisplayNames_[(size_t) idx] = nm;
        }
    }
}

void ModuleParamsRuntime::setMacroTarget(int macroIndex, uint32_t slotId, MacroTargetParam param) noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    macroTargetSlot_[(size_t) macroIndex].store(slotId, std::memory_order_release);
    macroTargetParam_[(size_t) macroIndex].store((uint32_t) param, std::memory_order_release);
}

void ModuleParamsRuntime::clearMacroTarget(int macroIndex) noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    macroTargetSlot_[(size_t) macroIndex].store(0u, std::memory_order_release);
    macroTargetParam_[(size_t) macroIndex].store((uint32_t) MacroTargetParam::None, std::memory_order_release);
}

uint32_t ModuleParamsRuntime::getMacroTargetSlot(int macroIndex) const noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return 0;
    return macroTargetSlot_[(size_t) macroIndex].load(std::memory_order_acquire);
}

MacroTargetParam ModuleParamsRuntime::getMacroTargetParam(int macroIndex) const noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return MacroTargetParam::None;
    return (MacroTargetParam) macroTargetParam_[(size_t) macroIndex].load(std::memory_order_acquire);
}

int ModuleParamsRuntime::findMacroIndexForTarget(uint32_t slotId, MacroTargetParam param) const noexcept
{
    if (slotId == 0 || param == MacroTargetParam::None)
        return -1;
    for (int i = 0; i < 8; ++i)
    {
        if (macroTargetSlot_[(size_t) i].load(std::memory_order_acquire) != slotId)
            continue;
        if ((MacroTargetParam) macroTargetParam_[(size_t) i].load(std::memory_order_acquire) == param)
            return i;
    }
    return -1;
}

void ModuleParamsRuntime::setMacroDisplayName(int macroIndex, juce::String name)
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    name = name.trim();
    if (name.isEmpty())
        return;
    macroDisplayNames_[(size_t) macroIndex] = name;
}

juce::String ModuleParamsRuntime::getMacroDisplayName(int macroIndex) const
{
    if (macroIndex < 0 || macroIndex > 7)
        return {};
    return macroDisplayNames_[(size_t) macroIndex];
}

} // namespace razumov::params
