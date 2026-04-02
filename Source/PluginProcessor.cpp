#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/graph/FlexGraphPlan.h"
#include "dsp/graph/FlexGraphDesc.h"
#include "dsp/graph/FlexGraphSerialization.h"
#include "dsp/graph/GraphPlanFactory.h"
#include "params/MacroRouting.h"
#include "params/ModuleParamsRuntime.h"
#include "params/ParamIDs.h"
#include "params/ParameterLayout.h"
#include "params/Phase3RealtimeParams.h"
#include "presets/FactoryPresets.h"

#include <juce_dsp/juce_dsp.h>
#include <map>

namespace
{

int getStartupChainIndex(juce::AudioProcessorValueTreeState& apvts)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(razumov::params::chainProfile)))
        return p->getIndex();
    return 0;
}

void collectLegacyParamMap(const juce::XmlElement& e, std::map<juce::String, float>& out)
{
    if (e.hasTagName("PARAM") && e.hasAttribute("id"))
        out[e.getStringAttribute("id")] = (float) e.getDoubleAttribute("value");
    for (auto* ch : e.getChildIterator())
        collectLegacyParamMap(*ch, out);
}

bool tryBuildPhase3FromLegacyMap(const std::map<juce::String, float>& m,
                                 razumov::params::Phase3RealtimeParams& out)
{
    using namespace razumov::params;
    const bool hasLegacy = m.find(micAmount) != m.end() || m.find(gainDb) != m.end()
                           || m.find(optoThresholdDb) != m.end();
    if (!hasLegacy)
        return false;

    razumov::params::Phase3RealtimeParams d {};
    auto getF = [&](const char* id, float def) {
        auto it = m.find(id);
        return it != m.end() ? it->second : def;
    };

    out = d;
    out.micBypass = getF(micBypass, 0.0f) > 0.5f;
    out.micAmount = getF(micAmount, d.micAmount);
    out.gainLinear = juce::Decibels::decibelsToGain(getF(gainDb, 0.0f));
    out.lowpassHz = getF(lowpassHz, d.lowpassHz);
    out.deessCrossoverHz = getF(deessCrossoverHz, d.deessCrossoverHz);
    out.deessThresholdDb = getF(deessThresholdDb, d.deessThresholdDb);
    out.deessRatio = getF(deessRatio, d.deessRatio);
    out.optoThresholdDb = getF(optoThresholdDb, d.optoThresholdDb);
    out.optoRatio = getF(optoRatio, d.optoRatio);
    out.optoMakeupDb = getF(optoMakeupDb, d.optoMakeupDb);
    out.fetThresholdDb = getF(fetThresholdDb, d.fetThresholdDb);
    out.fetRatio = getF(fetRatio, d.fetRatio);
    out.fetMakeupDb = getF(fetMakeupDb, d.fetMakeupDb);
    out.vcaThresholdDb = getF(vcaThresholdDb, d.vcaThresholdDb);
    out.vcaRatio = getF(vcaRatio, d.vcaRatio);
    out.vcaMakeupDb = getF(vcaMakeupDb, d.vcaMakeupDb);
    out.exciterDrive = getF(exciterDrive, d.exciterDrive);
    out.exciterMix = getF(exciterMix, d.exciterMix);
    out.spectralBypass = getF(spectralBypass, 0.0f) > 0.5f;
    out.spectralMix = getF(spectralMix, d.spectralMix);
    out.spectralThresholdDb = getF(spectralThresholdDb, d.spectralThresholdDb);
    out.spectralRatio = getF(spectralRatio, d.spectralRatio);
    return true;
}

razumov::params::MacroAudioState buildMacroStateFromApvts(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace razumov::params;
    MacroAudioState m;
    if (auto* raw = apvts.getRawParameterValue(macroGlue))
        m.glue01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroAir))
        m.air01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroSibilance))
        m.sibil01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroPresence))
        m.presence01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroPunch))
        m.punch01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroBody))
        m.body01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroSmooth))
        m.smooth01 = raw->load();
    if (auto* raw = apvts.getRawParameterValue(macroDensity))
        m.density01 = raw->load();
    return m;
}

float getMacro01FromApvts(juce::AudioProcessorValueTreeState& apvts, int macroIndex)
{
    using namespace razumov::params;
    const char* id = nullptr;
    switch (macroIndex)
    {
        case 0:
            id = macroGlue;
            break;
        case 1:
            id = macroAir;
            break;
        case 2:
            id = macroSibilance;
            break;
        case 3:
            id = macroPresence;
            break;
        case 4:
            id = macroPunch;
            break;
        case 5:
            id = macroBody;
            break;
        case 6:
            id = macroSmooth;
            break;
        case 7:
            id = macroDensity;
            break;
        default:
            return 0.5f;
    }
    if (auto* raw = apvts.getRawParameterValue(id))
        return raw->load();
    return 0.5f;
}

} // namespace

RazumovVocalChainAudioProcessor::RazumovVocalChainAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMS", razumov::params::createParameterLayout())
{
    graphEngine_.setLatencyCallback([this](int latency) { setLatencySamples(latency); });
    graphDesc_ = razumov::graph::GraphPlanFactory::makeStartupDescForIndex(getStartupChainIndex(apvts), 44100.0);
    nextSlotCounter_ = juce::jmax(1u, razumov::graph::maxSlotIdInSegment(graphDesc_) + 1);
    syncModuleParamsWithGraph();
}

RazumovVocalChainAudioProcessor::~RazumovVocalChainAudioProcessor() = default;

void RazumovVocalChainAudioProcessor::syncModuleParamsWithGraph()
{
    moduleParams_.syncWithGraphSlotIds(razumov::graph::collectModuleSlotIds(graphDesc_));
}

void RazumovVocalChainAudioProcessor::submitGraphPlanFromCurrentDesc()
{
    auto plan = razumov::graph::GraphPlanFactory::makePlanFromDesc(graphDesc_);
    graphEngine_.submitPlan(std::shared_ptr<razumov::graph::FlexGraphPlan>(std::move(plan)));
}

void RazumovVocalChainAudioProcessor::persistEmbeddedStateToApvts()
{
    auto root = apvts.copyState();
    auto oldG = root.getChildWithName(razumov::graph::flexGraphValueTreeType);
    if (oldG.isValid())
        root.removeChild(oldG, nullptr);
    auto oldM = root.getChildWithName(razumov::params::ModuleParamsRuntime::moduleParamsTreeType);
    if (oldM.isValid())
        root.removeChild(oldM, nullptr);
    root.addChild(razumov::graph::flexSegmentDescToValueTree(graphDesc_, nextSlotCounter_), -1, nullptr);
    root.addChild(moduleParams_.toValueTree(), -1, nullptr);
    apvts.replaceState(root);
}

void RazumovVocalChainAudioProcessor::syncGraphDescFromApvtsState()
{
    const auto g = apvts.state.getChildWithName(razumov::graph::flexGraphValueTreeType);
    if (!g.isValid())
        return;

    razumov::graph::FlexSegmentDesc tmp;
    uint32_t fileNext = 1;
    if (!razumov::graph::valueTreeToFlexSegmentDesc(g, tmp, &fileNext))
        return;

    graphDesc_ = std::move(tmp);

    if (razumov::graph::flexGraphNeedsSlotIdAssignment(graphDesc_))
    {
        uint32_t n = 1;
        razumov::graph::assignUniqueSlotIds(graphDesc_, n);
        nextSlotCounter_ = n;
    }
    else
    {
        nextSlotCounter_ = juce::jmax(fileNext, razumov::graph::maxSlotIdInSegment(graphDesc_) + 1);
        if (nextSlotCounter_ < 2)
            nextSlotCounter_ = 1;
    }
}

void RazumovVocalChainAudioProcessor::ensureGraphAfterStateLoad()
{
    const auto g = apvts.state.getChildWithName(razumov::graph::flexGraphValueTreeType);
    if (!g.isValid() || graphDesc_.empty())
    {
        graphDesc_ = razumov::graph::GraphPlanFactory::makeStartupDescForIndex(getStartupChainIndex(apvts), lastSampleRate_);
        nextSlotCounter_ = juce::jmax(1u, razumov::graph::maxSlotIdInSegment(graphDesc_) + 1);
        persistEmbeddedStateToApvts();
    }
}

void RazumovVocalChainAudioProcessor::applyChainProfileTemplate()
{
    graphDesc_ = razumov::graph::GraphPlanFactory::makeStartupDescForIndex(getStartupChainIndex(apvts), lastSampleRate_);
    nextSlotCounter_ = juce::jmax(1u, razumov::graph::maxSlotIdInSegment(graphDesc_) + 1);
    syncModuleParamsWithGraph();
    persistEmbeddedStateToApvts();
    submitGraphPlanFromCurrentDesc();
}

void RazumovVocalChainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lastSampleRate_ = sampleRate;
    syncGraphDescFromApvtsState();
    if (graphDesc_.empty())
        ensureGraphAfterStateLoad();

    syncModuleParamsWithGraph();

    auto plan = razumov::graph::GraphPlanFactory::makePlanFromDesc(graphDesc_);
    graphEngine_.submitPlan(std::shared_ptr<razumov::graph::FlexGraphPlan>(std::move(plan)));
    graphEngine_.prepare(sampleRate, samplesPerBlock, 2);
}

void RazumovVocalChainAudioProcessor::releaseResources()
{
    graphEngine_.releaseResources();
}

bool RazumovVocalChainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn.isDisabled() || mainIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RazumovVocalChainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const auto macros = buildMacroStateFromApvts(apvts);
    graphEngine_.process(buffer, macros, moduleParams_);
}

float RazumovVocalChainAudioProcessor::getModuleFloatParam(uint32_t slotId, const juce::String& paramId) const
{
    return moduleParams_.getFloat(slotId, paramId);
}

void RazumovVocalChainAudioProcessor::setModuleFloatParam(uint32_t slotId, const juce::String& paramId, float value)
{
    moduleParams_.setFloat(slotId, paramId, value);
}

bool RazumovVocalChainAudioProcessor::getModuleBoolParam(uint32_t slotId, const juce::String& paramId) const
{
    return moduleParams_.getBool(slotId, paramId);
}

void RazumovVocalChainAudioProcessor::setModuleBoolParam(uint32_t slotId, const juce::String& paramId, bool value)
{
    moduleParams_.setBool(slotId, paramId, value);
}

juce::AudioProcessorEditor* RazumovVocalChainAudioProcessor::createEditor()
{
    return new RazumovVocalChainAudioProcessorEditor(*this);
}

int RazumovVocalChainAudioProcessor::getNumPrograms()
{
    return razumov::presets::getNumFactoryPresets();
}

int RazumovVocalChainAudioProcessor::getCurrentProgram()
{
    return currentProgram_;
}

void RazumovVocalChainAudioProcessor::setCurrentProgram(int index)
{
    applyFactoryPreset(index);
}

const juce::String RazumovVocalChainAudioProcessor::getProgramName(int index)
{
    return juce::String(razumov::presets::getFactoryPresetName(index));
}

void RazumovVocalChainAudioProcessor::applyFactoryPreset(int index)
{
    const int n = razumov::presets::getNumFactoryPresets();
    currentProgram_ = juce::jlimit(0, juce::jmax(0, n - 1), index);
    razumov::presets::applyFactoryPresetGlobals(currentProgram_, apvts);
    const auto p = razumov::presets::buildFactoryPresetModulePhase3(currentProgram_);
    moduleParams_.seedAllSlotsWithSameParams(p);
}

uint32_t RazumovVocalChainAudioProcessor::resolveSerialInsertReferenceSlot(uint32_t referenceSlotId) const
{
    if (auto mic = razumov::graph::getRootModuleSlotIdAtIndex(graphDesc_, 0))
    {
        if (referenceSlotId == *mic)
        {
            if (auto room = razumov::graph::getRootModuleSlotIdAtIndex(graphDesc_, 1))
                return *room;
        }
    }
    return referenceSlotId;
}

bool RazumovVocalChainAudioProcessor::canInsertParallelSplitAfterSlot(uint32_t referenceSlotId) const
{
    return !razumov::graph::isProtectedFrontRootModuleSlot(graphDesc_, referenceSlotId);
}

void RazumovVocalChainAudioProcessor::commitGraphMutation()
{
    syncModuleParamsWithGraph();
    persistEmbeddedStateToApvts();
    submitGraphPlanFromCurrentDesc();
}

void RazumovVocalChainAudioProcessor::setSlotBypassForId(uint32_t slotId, bool bypassed)
{
    if (razumov::graph::setSlotBypassById(graphDesc_, slotId, bypassed))
        commitGraphMutation();
}

void RazumovVocalChainAudioProcessor::removeGraphSlotById(uint32_t slotId)
{
    if (razumov::graph::isProtectedFrontRootModuleSlot(graphDesc_, slotId))
        return;
    if (razumov::graph::removeSlotById(graphDesc_, slotId))
        commitGraphMutation();
}

void RazumovVocalChainAudioProcessor::moveRootSlotContainingId(uint32_t slotId, int delta)
{
    const int idx = razumov::graph::findRootSlotIndexContainingId(graphDesc_, slotId);
    if (idx < 0)
        return;
    const int ni = idx + delta;
    if (ni < 0 || ni >= (int) graphDesc_.size())
        return;
    if (idx <= 1 || ni <= 1)
        return;
    std::swap(graphDesc_[(size_t) idx], graphDesc_[(size_t) ni]);
    commitGraphMutation();
}

void RazumovVocalChainAudioProcessor::swapDirectRootModules(uint32_t slotIdA, uint32_t slotIdB)
{
    if (razumov::graph::trySwapDirectRootModuleSlots(graphDesc_, slotIdA, slotIdB))
        commitGraphMutation();
}

bool RazumovVocalChainAudioProcessor::canDuplicateRootModuleSlot(uint32_t slotId) const
{
    const int idx = razumov::graph::findRootSlotIndexContainingId(graphDesc_, slotId);
    if (idx < 0)
        return false;
    const auto& s = graphDesc_[(size_t) idx];
    if (s.descType != razumov::graph::FlexSlotDescType::Module || s.slotId != slotId)
        return false;
    return !razumov::graph::isProtectedFrontRootModuleSlot(graphDesc_, slotId);
}

void RazumovVocalChainAudioProcessor::duplicateRootModuleAfter(uint32_t slotId)
{
    if (!canDuplicateRootModuleSlot(slotId))
        return;
    const int idx = razumov::graph::findRootSlotIndexContainingId(graphDesc_, slotId);
    if (idx < 0)
        return;
    const auto& src = graphDesc_[(size_t) idx];
    const uint32_t oldId = src.slotId;
    razumov::graph::FlexSlotDesc copy = src;
    uint32_t n = nextSlotCounter_;
    razumov::graph::assignSlotIdsForSubtree(copy, n);
    nextSlotCounter_ = n;
    const uint32_t newId = copy.slotId;
    graphDesc_.insert(graphDesc_.begin() + idx + 1, std::move(copy));
    syncModuleParamsWithGraph();
    moduleParams_.copySlotParamsFromTo(oldId, newId);
    persistEmbeddedStateToApvts();
    submitGraphPlanFromCurrentDesc();
}

void RazumovVocalChainAudioProcessor::insertPaletteModuleAfterSlot(uint32_t referenceSlotId,
                                                                  razumov::graph::AudioNodeKind kind)
{
    const uint32_t ref = resolveSerialInsertReferenceSlot(referenceSlotId);
    razumov::graph::FlexSlotDesc ns = razumov::graph::GraphPlanFactory::makeModulePaletteSlot(kind);
    uint32_t n = nextSlotCounter_;
    razumov::graph::assignSlotIdsForSubtree(ns, n);
    nextSlotCounter_ = n;

    const int ri = razumov::graph::findRootSlotIndexContainingId(graphDesc_, ref);
    if (ri < 0)
        graphDesc_.push_back(std::move(ns));
    else
        graphDesc_.insert(graphDesc_.begin() + ri + 1, std::move(ns));

    commitGraphMutation();
}

void RazumovVocalChainAudioProcessor::insertParallelModuleAfterSlot(uint32_t referenceSlotId,
                                                                    razumov::graph::AudioNodeKind kind)
{
    if (!canInsertParallelSplitAfterSlot(referenceSlotId))
        return;
    razumov::graph::FlexSlotDesc sp = razumov::graph::GraphPlanFactory::makeSplitDryBranchAndParallelModule(kind);
    uint32_t n = nextSlotCounter_;
    razumov::graph::assignSlotIdsForSubtree(sp, n);
    nextSlotCounter_ = n;

    const int ri = razumov::graph::findRootSlotIndexContainingId(graphDesc_, referenceSlotId);
    if (ri < 0)
        graphDesc_.push_back(std::move(sp));
    else
        graphDesc_.insert(graphDesc_.begin() + ri + 1, std::move(sp));

    commitGraphMutation();
}

void RazumovVocalChainAudioProcessor::insertSplitAfterSlot(uint32_t referenceSlotId, int numBranches)
{
    if (!canInsertParallelSplitAfterSlot(referenceSlotId))
        return;
    razumov::graph::FlexSlotDesc sp = razumov::graph::GraphPlanFactory::makeSplitWithUnityBranches(numBranches);
    uint32_t n = nextSlotCounter_;
    razumov::graph::assignSlotIdsForSubtree(sp, n);
    nextSlotCounter_ = n;

    const int ri = razumov::graph::findRootSlotIndexContainingId(graphDesc_, referenceSlotId);
    if (ri < 0)
        graphDesc_.push_back(std::move(sp));
    else
        graphDesc_.insert(graphDesc_.begin() + ri + 1, std::move(sp));

    commitGraphMutation();
}

juce::StringArray RazumovVocalChainAudioProcessor::getChainStripLabelArray() const
{
    return razumov::graph::segmentDescToChainStripLabels(graphDesc_);
}

std::vector<razumov::graph::ChainStripItem> RazumovVocalChainAudioProcessor::getChainStripItems() const
{
    return razumov::graph::segmentDescToChainStripItems(graphDesc_);
}

void RazumovVocalChainAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void RazumovVocalChainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    persistEmbeddedStateToApvts();
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void RazumovVocalChainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        std::map<juce::String, float> legacyMap;
        collectLegacyParamMap(*xml, legacyMap);

        if (xml->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
            graphDesc_.clear();
            syncGraphDescFromApvtsState();
            ensureGraphAfterStateLoad();

            moduleParams_.clear();
            syncModuleParamsWithGraph();

            const auto mp = apvts.state.getChildWithName(razumov::params::ModuleParamsRuntime::moduleParamsTreeType);
            if (mp.isValid())
                moduleParams_.fromValueTree(mp);
            else
            {
                razumov::params::Phase3RealtimeParams legacy {};
                if (tryBuildPhase3FromLegacyMap(legacyMap, legacy))
                    moduleParams_.seedAllSlotsWithSameParams(legacy);
            }

            submitGraphPlanFromCurrentDesc();
        }
    }
}

void RazumovVocalChainAudioProcessor::assignMacroToModuleParam(int macroIndex, uint32_t slotId,
                                                               razumov::params::MacroTargetParam param)
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    if (slotId == 0 || param == razumov::params::MacroTargetParam::None)
        return;

    for (int i = 0; i < 8; ++i)
    {
        if (i == macroIndex)
            continue;
        if (moduleParams_.getMacroTargetSlot(i) == slotId && moduleParams_.getMacroTargetParam(i) == param)
            moduleParams_.clearMacroTarget(i);
    }

    moduleParams_.setMacroTarget(macroIndex, slotId, param);
    persistEmbeddedStateToApvts();
}

void RazumovVocalChainAudioProcessor::clearMacroTarget(int macroIndex)
{
    moduleParams_.clearMacroTarget(macroIndex);
    persistEmbeddedStateToApvts();
}

uint32_t RazumovVocalChainAudioProcessor::getMacroTargetSlot(int macroIndex) const noexcept
{
    return moduleParams_.getMacroTargetSlot(macroIndex);
}

razumov::params::MacroTargetParam RazumovVocalChainAudioProcessor::getMacroTargetParam(int macroIndex) const noexcept
{
    return moduleParams_.getMacroTargetParam(macroIndex);
}

int RazumovVocalChainAudioProcessor::findMacroIndexForSlotParam(uint32_t slotId, const juce::String& paramId) const noexcept
{
    const auto mp = razumov::params::macroTargetParamFromParamId(paramId);
    if (mp == razumov::params::MacroTargetParam::None)
        return -1;
    return moduleParams_.findMacroIndexForTarget(slotId, mp);
}

void RazumovVocalChainAudioProcessor::setMacroDisplayName(int macroIndex, const juce::String& name)
{
    moduleParams_.setMacroDisplayName(macroIndex, name);
    persistEmbeddedStateToApvts();
}

juce::String RazumovVocalChainAudioProcessor::getMacroDisplayName(int macroIndex) const
{
    return moduleParams_.getMacroDisplayName(macroIndex);
}

bool RazumovVocalChainAudioProcessor::copySpectrumForSlot(uint32_t slotId, float* dst256) const
{
    return graphEngine_.copySpectrumForSlot(slotId, dst256);
}

void RazumovVocalChainAudioProcessor::pushMacroIntoAssignedModuleParam(int macroIndex)
{
    const uint32_t sid = moduleParams_.getMacroTargetSlot(macroIndex);
    const auto mp = moduleParams_.getMacroTargetParam(macroIndex);
    if (sid == 0 || mp == razumov::params::MacroTargetParam::None)
        return;
    const char* pid = razumov::params::macroTargetParamToParamId(mp);
    if (pid == nullptr)
        return;
    const float m01 = getMacro01FromApvts(apvts, macroIndex);
    const float v = razumov::params::macro01ToParamValue(mp, m01);
    setModuleFloatParam(sid, juce::String(pid), v);
}

void RazumovVocalChainAudioProcessor::syncMacroApvtsFromAssignedModule(int macroIndex)
{
    const uint32_t sid = moduleParams_.getMacroTargetSlot(macroIndex);
    const auto mp = moduleParams_.getMacroTargetParam(macroIndex);
    if (sid == 0 || mp == razumov::params::MacroTargetParam::None)
        return;
    const char* pid = razumov::params::macroTargetParamToParamId(mp);
    if (pid == nullptr)
        return;
    const float v = getModuleFloatParam(sid, juce::String(pid));
    const float m01 = razumov::params::paramValueToMacro01(mp, v);
    const char* apId = nullptr;
    switch (macroIndex)
    {
        case 0:
            apId = razumov::params::macroGlue;
            break;
        case 1:
            apId = razumov::params::macroAir;
            break;
        case 2:
            apId = razumov::params::macroSibilance;
            break;
        case 3:
            apId = razumov::params::macroPresence;
            break;
        case 4:
            apId = razumov::params::macroPunch;
            break;
        case 5:
            apId = razumov::params::macroBody;
            break;
        case 6:
            apId = razumov::params::macroSmooth;
            break;
        case 7:
            apId = razumov::params::macroDensity;
            break;
        default:
            return;
    }
    if (auto* p = apvts.getParameter(apId))
        p->setValueNotifyingHost(m01);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RazumovVocalChainAudioProcessor();
}
