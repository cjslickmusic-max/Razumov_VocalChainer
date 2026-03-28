#include "ModuleParamsRuntime.h"
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

} // namespace detail

namespace
{

float bipolarMacro01(float normalized01) noexcept
{
    return (normalized01 - 0.5f) * 2.0f;
}

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

ModuleParamsRuntime::ModuleParamsRuntime() = default;

ModuleParamsRuntime::~ModuleParamsRuntime() = default;

void ModuleParamsRuntime::clear() noexcept
{
    slotIds_.clear();
    blocks_.clear();
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

    applyMacroOffsetsToPhase3(out, macros);
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
}

void applyMacroOffsetsToPhase3(Phase3RealtimeParams& p, const MacroAudioState& macros) noexcept
{
    const float bGlue = bipolarMacro01(macros.glue01);
    p.optoMakeupDb = juce::jlimit(0.0f, 24.0f, p.optoMakeupDb + bGlue * 4.0f);
    p.fetMakeupDb = juce::jlimit(0.0f, 24.0f, p.fetMakeupDb + bGlue * 3.0f);
    p.vcaMakeupDb = juce::jlimit(0.0f, 24.0f, p.vcaMakeupDb + bGlue * 3.0f);

    const float bAir = bipolarMacro01(macros.air01);
    p.lowpassHz = juce::jlimit(400.0f, 20000.0f, p.lowpassHz + bAir * 4500.0f);
    p.exciterMix = juce::jlimit(0.0f, 1.0f, p.exciterMix + bAir * 0.12f);

    const float bSib = bipolarMacro01(macros.sibil01);
    p.deessThresholdDb = juce::jlimit(-60.0f, 0.0f, p.deessThresholdDb + bSib * -10.0f);

    const float bPr = bipolarMacro01(macros.presence01);
    p.exciterDrive = juce::jlimit(0.1f, 8.0f, p.exciterDrive + bPr * 2.0f);
    p.gainLinear *= juce::Decibels::decibelsToGain(bPr * 2.0f);
}

} // namespace razumov::params
