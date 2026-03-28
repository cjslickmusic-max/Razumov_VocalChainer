#include "GraphEngine.h"

#include "CompressorArchetypeNode.h"
#include "DeesserNode.h"
#include "ExciterNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "MicCorrectionNode.h"
#include "SpectralCompressorNode.h"
#include "params/ModuleParamsRuntime.h"
#include "params/Phase3RealtimeParams.h"

#include <juce_core/juce_core.h>

namespace razumov::graph
{
namespace
{

void applyPhase3ToNode(AudioNode& node, const razumov::params::Phase3RealtimeParams& p)
{
    switch (node.getKind())
    {
        case AudioNodeKind::MicCorrection:
        {
            auto& n = static_cast<MicCorrectionNode&>(node);
            n.setBypass(p.micBypass);
            n.setAmount(p.micAmount);
            break;
        }
        case AudioNodeKind::Gain:
            static_cast<GainNode&>(node).setLinearGain(p.gainLinear);
            break;
        case AudioNodeKind::Filter:
            static_cast<FilterNode&>(node).setCutoffHz(p.lowpassHz);
            break;
        case AudioNodeKind::Deesser:
        {
            auto& n = static_cast<DeesserNode&>(node);
            n.setCrossoverHz(p.deessCrossoverHz);
            n.setThresholdDb(p.deessThresholdDb);
            n.setRatio(p.deessRatio);
            break;
        }
        case AudioNodeKind::OptoCompressor:
        {
            auto& n = static_cast<CompressorArchetypeNode&>(node);
            n.setThresholdDb(p.optoThresholdDb);
            n.setRatio(p.optoRatio);
            n.setMakeupDb(p.optoMakeupDb);
            break;
        }
        case AudioNodeKind::FetCompressor:
        {
            auto& n = static_cast<CompressorArchetypeNode&>(node);
            n.setThresholdDb(p.fetThresholdDb);
            n.setRatio(p.fetRatio);
            n.setMakeupDb(p.fetMakeupDb);
            break;
        }
        case AudioNodeKind::VcaCompressor:
        {
            auto& n = static_cast<CompressorArchetypeNode&>(node);
            n.setThresholdDb(p.vcaThresholdDb);
            n.setRatio(p.vcaRatio);
            n.setMakeupDb(p.vcaMakeupDb);
            break;
        }
        case AudioNodeKind::Exciter:
        {
            auto& n = static_cast<ExciterNode&>(node);
            n.setDrive(p.exciterDrive);
            n.setMix(p.exciterMix);
            break;
        }
        case AudioNodeKind::SpectralCompressor:
        {
            auto& n = static_cast<SpectralCompressorNode&>(node);
            n.setBypass(p.spectralBypass);
            n.setMix(p.spectralMix);
            n.setThresholdDb(p.spectralThresholdDb);
            n.setRatio(p.spectralRatio);
            break;
        }
        default:
            break;
    }
}

void walkApplyPhase3Slot(FlexSlot& slot,
                         const razumov::params::MacroAudioState& macros,
                         const razumov::params::ModuleParamsRuntime& runtime)
{
    if (slot.type == FlexSlot::Type::Module)
    {
        if (slot.bypassed || slot.node == nullptr)
            return;
        razumov::params::Phase3RealtimeParams p;
        runtime.fillSlot(slot.slotId, macros, p);
        applyPhase3ToNode(*slot.node, p);
        return;
    }

    for (auto& br : slot.branches)
        for (auto& child : br)
            walkApplyPhase3Slot(child, macros, runtime);
}

void walkApplyPhase3Segment(FlexSegment& seg,
                            const razumov::params::MacroAudioState& macros,
                            const razumov::params::ModuleParamsRuntime& runtime)
{
    for (auto& slot : seg)
        walkApplyPhase3Slot(slot, macros, runtime);
}

} // namespace

void GraphEngine::ensureBranchPool(int breadth)
{
    const int b = juce::jmax(2, breadth);
    maxSplitBreadthForPads_ = b;

    branchBuffers_.resize((size_t) b);
    for (auto& bbuf : branchBuffers_)
        bbuf.setSize(numChannels_, maxBlockSize_, false, false, true);

    const int nest = activePlan_ != nullptr ? activePlan_->computeMaxSplitNestingDepth() : 1;
    const int padCount = b * juce::jmax(1, nest);

    mergePads_.resize((size_t) padCount);
    for (int i = 0; i < padCount; ++i)
    {
        if (mergePads_[(size_t) i] == nullptr)
            mergePads_[(size_t) i] = std::make_unique<MergeDelayPad>();

        mergePads_[(size_t) i]->prepare(numChannels_, maxBlockSize_, maxDelayStorage_);
    }
}

void GraphEngine::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = sampleRate;
    maxBlockSize_ = juce::jmax(1, maxBlockSize);
    numChannels_ = juce::jmax(1, numChannels);

    maxDelayStorage_ = (int) sampleRate + maxBlockSize_ + 256;

    std::shared_ptr<FlexGraphPlan> activated;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingPlan_)
            activePlan_ = std::move(pendingPlan_);

        activated = activePlan_;
    }

    if (activated != nullptr)
    {
        ensureBranchPool(activated->getMaxSplitBreadth());
        activated->prepare(sampleRate_, maxBlockSize_, numChannels_);
        activated->reset();
        reportedLatency_ = activated->computePluginLatencySamples();
    }
    else
    {
        ensureBranchPool(2);
        reportedLatency_ = 0;
    }

    if (onLatency_)
        onLatency_(reportedLatency_);
}

void GraphEngine::releaseResources()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (activePlan_ != nullptr)
        activePlan_->reset();

    pendingPlan_.reset();
}

void GraphEngine::submitPlan(std::shared_ptr<FlexGraphPlan> plan)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pendingPlan_ = std::move(plan);
}

void GraphEngine::swapAndPreparePendingPlan()
{
    std::shared_ptr<FlexGraphPlan> local;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingPlan_ == nullptr)
            return;

        activePlan_ = std::move(pendingPlan_);
        local = activePlan_;
    }

    if (local == nullptr)
    {
        reportedLatency_ = 0;
        if (onLatency_)
            onLatency_(0);
        return;
    }

    ensureBranchPool(local->getMaxSplitBreadth());
    local->prepare(sampleRate_, maxBlockSize_, numChannels_);
    local->reset();
    reportedLatency_ = local->computePluginLatencySamples();

    if (onLatency_)
        onLatency_(reportedLatency_);
}

void GraphEngine::process(juce::AudioBuffer<float>& buffer,
                          const razumov::params::MacroAudioState& macros,
                          const razumov::params::ModuleParamsRuntime& moduleParams)
{
    swapAndPreparePendingPlan();

    if (activePlan_ == nullptr)
        return;

    auto& root = activePlan_->getRoot();
    if (root.empty())
        return;

    walkApplyPhase3Segment(root, macros, moduleParams);

    const int newLat = activePlan_->computePluginLatencySamples();
    if (newLat != reportedLatency_)
    {
        reportedLatency_ = newLat;
        if (onLatency_)
            onLatency_(reportedLatency_);
    }

    processSegment(root, buffer, 0);
}

void GraphEngine::processSegment(FlexSegment& seg, juce::AudioBuffer<float>& buffer, int splitDepth)
{
    for (auto& slot : seg)
    {
        if (slot.type == FlexSlot::Type::Module)
        {
            if (slot.bypassed || slot.node == nullptr)
                continue;
            slot.node->process(buffer);
        }
        else
        {
            processSplit(slot, buffer, splitDepth);
        }
    }
}

void GraphEngine::processSplit(FlexSlot& slot, juce::AudioBuffer<float>& buffer, int splitDepth)
{
    if (slot.bypassed)
        return;

    const int numBranches = (int) slot.branches.size();
    if (numBranches < 2)
        return;

    jassert(numBranches <= (int) branchBuffers_.size());

    const int n = buffer.getNumSamples();
    const int ch = juce::jmin(buffer.getNumChannels(), numChannels_);

    std::vector<int> latPerBranch((size_t) numBranches);
    for (int i = 0; i < numBranches; ++i)
        latPerBranch[(size_t) i] = segmentLatencySamples(slot.branches[(size_t) i]);

    int mx = 0;
    for (int l : latPerBranch)
        mx = juce::jmax(mx, l);

    const int padBase = splitDepth * maxSplitBreadthForPads_;

    for (int i = 0; i < numBranches; ++i)
    {
        auto& work = branchBuffers_[(size_t) i];
        work.setSize(ch, n, false, false, true);

        for (int c = 0; c < ch; ++c)
            work.copyFrom(c, 0, buffer, c, 0, n);

        processSegment(slot.branches[(size_t) i], work, splitDepth + 1);

        const int pad = mx - latPerBranch[(size_t) i];
        const int padIdx = padBase + i;
        jassert(padIdx >= 0 && padIdx < (int) mergePads_.size());
        mergePads_[(size_t) padIdx]->setDelaySamples(pad);
        mergePads_[(size_t) padIdx]->process(work);
    }

    buffer.clear();
    for (int i = 0; i < numBranches; ++i)
    {
        for (int c = 0; c < ch; ++c)
            buffer.addFrom(c, 0, branchBuffers_[(size_t) i], c, 0, n);
    }
}

} // namespace razumov::graph
