#include "GraphPlan.h"

#include <algorithm>

namespace razumov::graph
{

int chainLatencySamples(const std::vector<std::unique_ptr<AudioNode>>& chain) noexcept
{
    int sum = 0;
    for (const auto& n : chain)
        if (n != nullptr)
            sum += n->getLatencySamples();

    return sum;
}

void GraphPlan::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    for (auto& step : steps_)
    {
        if (auto* serial = std::get_if<SerialStep>(&step))
        {
            for (auto& node : serial->nodes)
                if (node != nullptr)
                    node->prepare(sampleRate, maxBlockSize, numChannels);
        }
        else if (auto* par = std::get_if<ParallelStep>(&step))
        {
            for (auto& node : par->left)
                if (node != nullptr)
                    node->prepare(sampleRate, maxBlockSize, numChannels);

            for (auto& node : par->right)
                if (node != nullptr)
                    node->prepare(sampleRate, maxBlockSize, numChannels);
        }
    }
}

void GraphPlan::reset()
{
    for (auto& step : steps_)
    {
        if (auto* serial = std::get_if<SerialStep>(&step))
        {
            for (auto& node : serial->nodes)
                if (node != nullptr)
                    node->reset();
        }
        else if (auto* par = std::get_if<ParallelStep>(&step))
        {
            for (auto& node : par->left)
                if (node != nullptr)
                    node->reset();

            for (auto& node : par->right)
                if (node != nullptr)
                    node->reset();
        }
    }
}

int GraphPlan::computePluginLatencySamples() const
{
    int total = 0;

    for (const auto& step : steps_)
    {
        if (const auto* serial = std::get_if<SerialStep>(&step))
        {
            for (const auto& node : serial->nodes)
                if (node != nullptr)
                    total += node->getLatencySamples();
        }
        else if (const auto* par = std::get_if<ParallelStep>(&step))
        {
            const int l = chainLatencySamples(par->left);
            const int r = chainLatencySamples(par->right);
            total += std::max(l, r);
        }
    }

    return total;
}

} // namespace razumov::graph
