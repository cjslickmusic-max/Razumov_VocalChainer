#include "GraphEngine.h"

#include "CompressorArchetypeNode.h"
#include "DeesserNode.h"
#include "ExciterNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "MicCorrectionNode.h"
#include "SpectralCompressorNode.h"
#include "params/Phase3RealtimeParams.h"

#include <juce_core/juce_core.h>

namespace razumov::graph
{

void GraphNodeBindings::clear() noexcept
{
    mic = nullptr;
    gain = nullptr;
    filter = nullptr;
    deesser = nullptr;
    opto = nullptr;
    fet = nullptr;
    vca = nullptr;
    exciter = nullptr;
    spectral = nullptr;
}

void GraphEngine::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = sampleRate;
    maxBlockSize_ = juce::jmax(1, maxBlockSize);
    numChannels_ = juce::jmax(1, numChannels);

    maxDelayStorage_ = (int) sampleRate + maxBlockSize_ + 256;

    workL_.setSize(numChannels_, maxBlockSize_);
    workR_.setSize(numChannels_, maxBlockSize_);

    if (!padL_)
        padL_ = std::make_unique<MergeDelayPad>();
    if (!padR_)
        padR_ = std::make_unique<MergeDelayPad>();

    padL_->prepare(numChannels_, maxBlockSize_, maxDelayStorage_);
    padR_->prepare(numChannels_, maxBlockSize_, maxDelayStorage_);

    std::shared_ptr<GraphPlan> activated;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingPlan_)
            activePlan_ = std::move(pendingPlan_);

        activated = activePlan_;
    }

    if (activated != nullptr)
    {
        activated->prepare(sampleRate_, maxBlockSize_, numChannels_);
        activated->reset();
        reportedLatency_ = activated->computePluginLatencySamples();
        refreshParameterBindings();
    }
    else
    {
        reportedLatency_ = 0;
        bindings_.clear();
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
    bindings_.clear();
}

void GraphEngine::submitPlan(std::shared_ptr<GraphPlan> plan)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pendingPlan_ = std::move(plan);
}

void GraphEngine::swapAndPreparePendingPlan()
{
    std::shared_ptr<GraphPlan> local;
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
        bindings_.clear();
        if (onLatency_)
            onLatency_(0);
        return;
    }

    local->prepare(sampleRate_, maxBlockSize_, numChannels_);
    local->reset();
    reportedLatency_ = local->computePluginLatencySamples();
    refreshParameterBindings();

    if (onLatency_)
        onLatency_(reportedLatency_);
}

void GraphEngine::process(juce::AudioBuffer<float>& buffer)
{
    swapAndPreparePendingPlan();

    if (activePlan_ == nullptr)
        return;

    if (activePlan_->getSteps().empty())
        return;

    const int newLat = activePlan_->computePluginLatencySamples();
    if (newLat != reportedLatency_)
    {
        reportedLatency_ = newLat;
        if (onLatency_)
            onLatency_(reportedLatency_);
    }

    runSteps(buffer);
}

void GraphEngine::bindFromNode(AudioNode& node)
{
    switch (node.getKind())
    {
        case AudioNodeKind::MicCorrection:
            if (bindings_.mic == nullptr)
                bindings_.mic = static_cast<MicCorrectionNode*>(&node);
            break;
        case AudioNodeKind::Gain:
            if (bindings_.gain == nullptr)
                bindings_.gain = static_cast<GainNode*>(&node);
            break;
        case AudioNodeKind::Filter:
            if (bindings_.filter == nullptr)
                bindings_.filter = static_cast<FilterNode*>(&node);
            break;
        case AudioNodeKind::Deesser:
            if (bindings_.deesser == nullptr)
                bindings_.deesser = static_cast<DeesserNode*>(&node);
            break;
        case AudioNodeKind::OptoCompressor:
            if (bindings_.opto == nullptr)
                bindings_.opto = static_cast<CompressorArchetypeNode*>(&node);
            break;
        case AudioNodeKind::FetCompressor:
            if (bindings_.fet == nullptr)
                bindings_.fet = static_cast<CompressorArchetypeNode*>(&node);
            break;
        case AudioNodeKind::VcaCompressor:
            if (bindings_.vca == nullptr)
                bindings_.vca = static_cast<CompressorArchetypeNode*>(&node);
            break;
        case AudioNodeKind::Exciter:
            if (bindings_.exciter == nullptr)
                bindings_.exciter = static_cast<ExciterNode*>(&node);
            break;
        case AudioNodeKind::SpectralCompressor:
            if (bindings_.spectral == nullptr)
                bindings_.spectral = static_cast<SpectralCompressorNode*>(&node);
            break;
        default:
            break;
    }
}

void GraphEngine::refreshParameterBindings()
{
    bindings_.clear();

    if (activePlan_ == nullptr)
        return;

    auto scanSerial = [this](SerialStep& serial) {
        for (auto& node : serial.nodes)
        {
            if (node != nullptr)
                bindFromNode(*node);
        }
    };

    for (auto& step : activePlan_->getSteps())
    {
        if (auto* serial = std::get_if<SerialStep>(&step))
        {
            scanSerial(*serial);
        }
        else if (auto* par = std::get_if<ParallelStep>(&step))
        {
            for (auto& node : par->left)
                if (node != nullptr)
                    bindFromNode(*node);

            for (auto& node : par->right)
                if (node != nullptr)
                    bindFromNode(*node);
        }
    }
}

void GraphEngine::applyPhase3Parameters(const razumov::params::Phase3RealtimeParams& p)
{
    if (bindings_.mic != nullptr)
    {
        bindings_.mic->setBypass(p.micBypass);
        bindings_.mic->setAmount(p.micAmount);
    }

    if (bindings_.gain != nullptr)
        bindings_.gain->setLinearGain(p.gainLinear);

    if (bindings_.filter != nullptr)
        bindings_.filter->setCutoffHz(p.lowpassHz);

    if (bindings_.deesser != nullptr)
    {
        bindings_.deesser->setCrossoverHz(p.deessCrossoverHz);
        bindings_.deesser->setThresholdDb(p.deessThresholdDb);
        bindings_.deesser->setRatio(p.deessRatio);
    }

    if (bindings_.opto != nullptr)
    {
        bindings_.opto->setThresholdDb(p.optoThresholdDb);
        bindings_.opto->setRatio(p.optoRatio);
        bindings_.opto->setMakeupDb(p.optoMakeupDb);
    }

    if (bindings_.fet != nullptr)
    {
        bindings_.fet->setThresholdDb(p.fetThresholdDb);
        bindings_.fet->setRatio(p.fetRatio);
        bindings_.fet->setMakeupDb(p.fetMakeupDb);
    }

    if (bindings_.vca != nullptr)
    {
        bindings_.vca->setThresholdDb(p.vcaThresholdDb);
        bindings_.vca->setRatio(p.vcaRatio);
        bindings_.vca->setMakeupDb(p.vcaMakeupDb);
    }

    if (bindings_.exciter != nullptr)
    {
        bindings_.exciter->setDrive(p.exciterDrive);
        bindings_.exciter->setMix(p.exciterMix);
    }

    if (bindings_.spectral != nullptr)
    {
        bindings_.spectral->setBypass(p.spectralBypass);
        bindings_.spectral->setMix(p.spectralMix);
        bindings_.spectral->setThresholdDb(p.spectralThresholdDb);
        bindings_.spectral->setRatio(p.spectralRatio);
    }
}

void GraphEngine::runSteps(juce::AudioBuffer<float>& buffer)
{
    for (auto& step : activePlan_->getSteps())
    {
        if (auto* serial = std::get_if<SerialStep>(&step))
        {
            for (auto& node : serial->nodes)
                if (node != nullptr)
                    node->process(buffer);
        }
        else if (auto* par = std::get_if<ParallelStep>(&step))
        {
            processParallel(*par, buffer);
        }
    }
}

void GraphEngine::processParallel(ParallelStep& par, juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int ch = juce::jmin(buffer.getNumChannels(), workL_.getNumChannels(), workR_.getNumChannels());

    workL_.setSize(ch, n, false, false, true);
    workR_.setSize(ch, n, false, false, true);

    for (int c = 0; c < ch; ++c)
    {
        workL_.copyFrom(c, 0, buffer, c, 0, n);
        workR_.copyFrom(c, 0, buffer, c, 0, n);
    }

    for (auto& node : par.left)
        if (node != nullptr)
            node->process(workL_);

    for (auto& node : par.right)
        if (node != nullptr)
            node->process(workR_);

    const int latL = chainLatencySamples(par.left);
    const int latR = chainLatencySamples(par.right);
    const int mx = juce::jmax(latL, latR);
    const int padL = mx - latL;
    const int padR = mx - latR;

    padL_->setDelaySamples(padL);
    padR_->setDelaySamples(padR);
    padL_->process(workL_);
    padR_->process(workR_);

    buffer.clear();
    for (int c = 0; c < ch; ++c)
    {
        buffer.addFrom(c, 0, workL_, c, 0, n);
        buffer.addFrom(c, 0, workR_, c, 0, n);
    }
}

} // namespace razumov::graph
