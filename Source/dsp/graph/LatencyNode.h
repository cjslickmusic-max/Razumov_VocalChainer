#pragma once

#include "AudioNode.h"

namespace razumov::graph
{

/** Целочисленная задержка для тестов merge / latency (кольцевой буфер). */
class LatencyNode final : public AudioNode
{
public:
    explicit LatencyNode(int latencySamples) : latency_(juce::jmax(0, latencySamples)) {}

    int getLatencySamples() const noexcept override { return latency_; }

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::Latency; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    int latency_ { 0 };
    int lineSize_ { 0 };
    int writePos_ { 0 };
    juce::AudioBuffer<float> line_;
};

} // namespace razumov::graph
