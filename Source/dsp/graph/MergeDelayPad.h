#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace razumov::graph
{

/** Целочисленная задержка для выравнивания веток перед суммированием (merge). */
class MergeDelayPad
{
public:
    void prepare(int numChannels, int maxBlock, int maxDelay);
    void setDelaySamples(int delaySamples);
    void process(juce::AudioBuffer<float>& buffer);

private:
    int delaySamples_ { 0 };
    int lineSize_ { 0 };
    int writePos_ { 0 };
    juce::AudioBuffer<float> line_;
};

} // namespace razumov::graph
