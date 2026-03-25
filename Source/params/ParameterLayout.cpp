#include "ParameterLayout.h"
#include "ParamIDs.h"

namespace razumov::params
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { gainDb, 1 },
        "Gain",
        NormalisableRange<float> { -24.0f, 12.0f, 0.1f },
        0.0f,
        AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { lowpassHz, 1 },
        "Lowpass",
        NormalisableRange<float> { 400.0f, 20000.0f, 1.0f, 0.35f },
        20000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    return { params.begin(), params.end() };
}

} // namespace razumov::params
