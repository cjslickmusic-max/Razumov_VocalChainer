#pragma once

#include "FlexGraphDesc.h"
#include "AudioNode.h"
#include <juce_core/juce_core.h>
#include <memory>
#include <vector>

namespace razumov::graph
{

/** Слот исполнения: модуль или параллель (N >= 2 веток), рекурсивно. */
struct FlexSlot
{
    enum class Type : uint8_t
    {
        Module,
        Split
    };

    Type type { Type::Module };
    uint32_t slotId { 0 };
    bool bypassed { false };
    std::unique_ptr<AudioNode> node;
    std::vector<std::vector<FlexSlot>> branches;
};

using FlexSegment = std::vector<FlexSlot>;

class FlexGraphPlan
{
public:
    FlexGraphPlan() = default;

    explicit FlexGraphPlan(FlexSegment root, int maxSplitBreadth) noexcept
        : root_(std::move(root))
        , maxSplitBreadth_(juce::jmax(2, maxSplitBreadth))
    {
    }

    const FlexSegment& getRoot() const noexcept { return root_; }
    FlexSegment& getRoot() noexcept { return root_; }

    int getMaxSplitBreadth() const noexcept { return maxSplitBreadth_; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    /** Serial: сумма задержек слотов; Split: max по веткам; суммируется по корню. */
    int computePluginLatencySamples() const noexcept;

    static FlexGraphPlan buildFromDesc(const FlexSegmentDesc& desc);

private:
    static FlexSlot buildSlotFromDesc(const FlexSlotDesc& d);
    static int computeMaxSplitBreadthRuntime(const FlexSegment& seg) noexcept;

    FlexSegment root_;
    int maxSplitBreadth_ { 2 };
};

int segmentLatencySamples(const FlexSegment& seg) noexcept;
int slotLatencySamples(const FlexSlot& slot) noexcept;

} // namespace razumov::graph
