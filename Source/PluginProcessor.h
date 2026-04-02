#pragma once

#include "dsp/graph/FlexGraphDesc.h"
#include "dsp/graph/GraphEngine.h"
#include "params/MacroRouting.h"
#include "params/ModuleParamsRuntime.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

class RazumovVocalChainAudioProcessor : public juce::AudioProcessor
{
public:
    RazumovVocalChainAudioProcessor();
    ~RazumovVocalChainAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const noexcept { return apvts; }

    /** Параметры DSP по slotId (per-instance). */
    float getModuleFloatParam(uint32_t slotId, const juce::String& paramId) const;
    void setModuleFloatParam(uint32_t slotId, const juce::String& paramId, float value);
    bool getModuleBoolParam(uint32_t slotId, const juce::String& paramId) const;
    void setModuleBoolParam(uint32_t slotId, const juce::String& paramId, bool value);

    juce::StringArray getChainStripLabelArray() const;
    std::vector<razumov::graph::ChainStripItem> getChainStripItems() const;

    razumov::graph::FlexSegmentDesc& getGraphDesc() noexcept { return graphDesc_; }
    const razumov::graph::FlexSegmentDesc& getGraphDesc() const noexcept { return graphDesc_; }

    /** Сохранить FlexGraph + ModuleParams в APVTS и отправить план в движок (message thread). */
    void commitGraphMutation();

    void applyFactoryPreset(int index);

    /** Serial insert after Mic maps to after Room so Mic/Room stay adjacent. */
    uint32_t resolveSerialInsertReferenceSlot(uint32_t referenceSlotId) const;
    bool canInsertParallelSplitAfterSlot(uint32_t referenceSlotId) const;

    void setSlotBypassForId(uint32_t slotId, bool bypassed);
    void removeGraphSlotById(uint32_t slotId);
    void moveRootSlotContainingId(uint32_t slotId, int delta);
    /** Swap two direct root-level modules (not inside split); Mic/Room fixed. */
    void swapDirectRootModules(uint32_t slotIdA, uint32_t slotIdB);
    void insertPaletteModuleAfterSlot(uint32_t referenceSlotId, razumov::graph::AudioNodeKind kind);
    /** Split x2: dry 0.5 + (0.5 -> module); UI picks module instead of abstract "Split x2". */
    void insertParallelModuleAfterSlot(uint32_t referenceSlotId, razumov::graph::AudioNodeKind kind);
    void insertSplitAfterSlot(uint32_t referenceSlotId, int numBranches);

    /** Direct root module only (not Mic/Room, not inside split). */
    bool canDuplicateRootModuleSlot(uint32_t slotId) const;
    void duplicateRootModuleAfter(uint32_t slotId);

    void assignMacroToModuleParam(int macroIndex, uint32_t slotId, razumov::params::MacroTargetParam param);
    void clearMacroTarget(int macroIndex);
    uint32_t getMacroTargetSlot(int macroIndex) const noexcept;
    razumov::params::MacroTargetParam getMacroTargetParam(int macroIndex) const noexcept;
    int findMacroIndexForSlotParam(uint32_t slotId, const juce::String& paramId) const noexcept;
    void setMacroDisplayName(int macroIndex, const juce::String& name);
    juce::String getMacroDisplayName(int macroIndex) const;
    void pushMacroIntoAssignedModuleParam(int macroIndex);
    void syncMacroApvtsFromAssignedModule(int macroIndex);

private:
    void submitGraphPlanFromCurrentDesc();
    void applyChainProfileTemplate();
    void syncGraphDescFromApvtsState();
    void persistEmbeddedStateToApvts();
    void ensureGraphAfterStateLoad();
    void syncModuleParamsWithGraph();

    juce::AudioProcessorValueTreeState apvts;
    razumov::params::ModuleParamsRuntime moduleParams_;
    razumov::graph::GraphEngine graphEngine_;
    razumov::graph::FlexSegmentDesc graphDesc_;
    uint32_t nextSlotCounter_ { 1 };
    double lastSampleRate_ { 44100.0 };
    int currentProgram_ { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RazumovVocalChainAudioProcessor)
};
