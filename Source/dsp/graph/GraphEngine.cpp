#include "GraphEngine.h"

#include "CompressorArchetypeNode.h"
#include "DeesserNode.h"
#include "ExciterNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "MicCorrectionNode.h"
#include "SpectralCompressorNode.h"
#include "ParametricEqNode.h"
#include "params/ModuleParamsRuntime.h"
#include "params/Phase3RealtimeParams.h"

#include "FlexGraphDesc.h"

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
        case AudioNodeKind::ParametricEq:
            static_cast<ParametricEqNode&>(node).applyPhase3(p);
            break;
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

bool GraphEngine::walkCopySpectrum(const FlexSegment& seg, uint32_t slotId, float* dst256) noexcept
{
    if (dst256 == nullptr)
        return false;
    for (const auto& s : seg)
    {
        if (s.type == FlexSlot::Type::Module)
        {
            if (s.slotId == slotId && s.node != nullptr)
            {
                if (auto* sp = s.node->asSpectrumSource())
                {
                    sp->copySpectrum256(dst256);
                    return true;
                }
                return false;
            }
        }
        else
        {
            for (const auto& br : s.branches)
                if (walkCopySpectrum(br, slotId, dst256))
                    return true;
        }
    }
    return false;
}

bool GraphEngine::copySpectrumForSlot(uint32_t slotId, float* dst256) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (activePlan_ == nullptr)
        return false;
    return walkCopySpectrum(activePlan_->getRoot(), slotId, dst256);
}

bool GraphEngine::walkGainReduction(const FlexSegment& seg, uint32_t slotId, float& outDb) noexcept
{
    for (const auto& s : seg)
    {
        if (s.type == FlexSlot::Type::Module)
        {
            if (s.slotId == slotId && s.node != nullptr)
            {
                outDb = s.node->getGainReductionDbForUi();
                return true;
            }
        }
        else
        {
            for (const auto& br : s.branches)
                if (walkGainReduction(br, slotId, outDb))
                    return true;
        }
    }
    return false;
}

float GraphEngine::getGainReductionDbForSlot(uint32_t slotId) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (activePlan_ == nullptr)
        return 0.f;
    float db = 0.f;
    if (walkGainReduction(activePlan_->getRoot(), slotId, db))
        return db;
    return 0.f;
}

bool GraphEngine::walkSpectralCompressionDisplay(const FlexSegment& seg,
                                                 uint32_t slotId,
                                                 float* in256,
                                                 float* red256) noexcept
{
    if (in256 == nullptr || red256 == nullptr)
        return false;
    for (const auto& s : seg)
    {
        if (s.type == FlexSlot::Type::Module)
        {
            if (s.slotId == slotId && s.node != nullptr)
                return s.node->copySpectralCompressionDisplay256(in256, red256);
        }
        else
        {
            for (const auto& br : s.branches)
                if (walkSpectralCompressionDisplay(br, slotId, in256, red256))
                    return true;
        }
    }
    return false;
}

bool GraphEngine::copySpectralCompressionDisplayForSlot(uint32_t slotId, float* in256, float* red256) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (activePlan_ == nullptr)
        return false;
    return walkSpectralCompressionDisplay(activePlan_->getRoot(), slotId, in256, red256);
}

void GraphEngine::ensureBranchPool(int breadth)
{
    const int b = juce::jmax(2, breadth);
    maxSplitBreadthForPads_ = b;

    branchBuffers_.resize((size_t) b);
    for (auto& bbuf : branchBuffers_)
        bbuf.setSize(numChannels_, maxBlockSize_, false, false, true);

    latPerBranchScratch_.assign((size_t) b, 0);

    const int nest = activePlan_ != nullptr ? activePlan_->computeMaxSplitNestingDepth() : 1;
    const int padCount = b * juce::jmax(1, nest);

    phaseAlignPads_.resize((size_t) padCount);
    mergePads_.resize((size_t) padCount);
    for (int i = 0; i < padCount; ++i)
    {
        if (phaseAlignPads_[(size_t) i] == nullptr)
            phaseAlignPads_[(size_t) i] = std::make_unique<MergeDelayPad>();

        if (mergePads_[(size_t) i] == nullptr)
            mergePads_[(size_t) i] = std::make_unique<MergeDelayPad>();

        phaseAlignPads_[(size_t) i]->prepare(numChannels_, maxBlockSize_, maxDelayStorage_);
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

    jassert(numBranches <= (int) latPerBranchScratch_.size());
    for (int i = 0; i < numBranches; ++i)
    {
        const int ph = juce::jlimit(
            0,
            kMaxBranchPhaseAlignSamples,
            (i < (int) slot.branchPhaseAlignSamples.size()) ? slot.branchPhaseAlignSamples[(size_t) i] : 0);
        latPerBranchScratch_[(size_t) i] = segmentLatencySamples(slot.branches[(size_t) i]) + ph;
    }

    int mx = 0;
    for (int i = 0; i < numBranches; ++i)
        mx = juce::jmax(mx, latPerBranchScratch_[(size_t) i]);

    const int padBase = splitDepth * maxSplitBreadthForPads_;

    for (int i = 0; i < numBranches; ++i)
    {
        auto& work = branchBuffers_[(size_t) i];
        work.setSize(ch, n, false, false, true);

        for (int c = 0; c < ch; ++c)
            work.copyFrom(c, 0, buffer, c, 0, n);

        processSegment(slot.branches[(size_t) i], work, splitDepth + 1);

        const int padIdx = padBase + i;
        jassert(padIdx >= 0 && padIdx < (int) mergePads_.size());
        const int ph = juce::jlimit(
            0,
            kMaxBranchPhaseAlignSamples,
            (i < (int) slot.branchPhaseAlignSamples.size()) ? slot.branchPhaseAlignSamples[(size_t) i] : 0);
        const int segLat = segmentLatencySamples(slot.branches[(size_t) i]);
        const int pdcPad = mx - segLat - ph;
        jassert(pdcPad >= 0);

        phaseAlignPads_[(size_t) padIdx]->setDelaySamples(ph);
        phaseAlignPads_[(size_t) padIdx]->process(work);

        mergePads_[(size_t) padIdx]->setDelaySamples(pdcPad);
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
