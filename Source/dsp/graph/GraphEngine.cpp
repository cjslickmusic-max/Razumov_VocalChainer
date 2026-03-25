#include "GraphEngine.h"

#include "FilterNode.h"
#include "GainNode.h"

#include <juce_core/juce_core.h>

namespace razumov::graph
{

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
        gainBinding_ = nullptr;
        filterBinding_ = nullptr;
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
        gainBinding_ = nullptr;
        filterBinding_ = nullptr;
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

    runSteps(buffer);
}

void GraphEngine::refreshParameterBindings()
{
    gainBinding_ = nullptr;
    filterBinding_ = nullptr;

    if (activePlan_ == nullptr)
        return;

    auto scanSerial = [this](SerialStep& serial) {
        for (auto& node : serial.nodes)
        {
            if (node == nullptr)
                continue;

            if (gainBinding_ == nullptr)
                gainBinding_ = node->asGain();

            if (filterBinding_ == nullptr)
                filterBinding_ = node->asFilter();
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
            {
                if (node == nullptr)
                    continue;
                if (gainBinding_ == nullptr)
                    gainBinding_ = node->asGain();
                if (filterBinding_ == nullptr)
                    filterBinding_ = node->asFilter();
            }

            for (auto& node : par->right)
            {
                if (node == nullptr)
                    continue;
                if (gainBinding_ == nullptr)
                    gainBinding_ = node->asGain();
                if (filterBinding_ == nullptr)
                    filterBinding_ = node->asFilter();
            }
        }
    }
}

void GraphEngine::applyLiveParameters(float gainLinear, float lowpassHz)
{
    if (gainBinding_ != nullptr)
        gainBinding_->setLinearGain(gainLinear);

    if (filterBinding_ != nullptr)
        filterBinding_->setCutoffHz(lowpassHz);
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
