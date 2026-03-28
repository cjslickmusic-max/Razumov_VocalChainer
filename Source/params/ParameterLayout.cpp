#include "ParameterLayout.h"
#include "MicProfileChoices.h"
#include "ParamIDs.h"

namespace razumov::params
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID { micProfile, 1 },
        "Mic profile",
        buildMicProfileChoiceStrings(),
        0));

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID { chainProfile, 1 },
        "Chain",
        StringArray { "Full (Opto->FET->VCA)", "Compact (Opto only)", "FET-forward (FET->Opto->VCA)" },
        0));

    auto addMacro = [&](const char* id, const char* name) {
        params.push_back(std::make_unique<AudioParameterFloat>(
            ParameterID { id, 1 },
            name,
            NormalisableRange<float> { 0.0f, 1.0f, 0.01f },
            0.5f,
            AudioParameterFloatAttributes().withLabel("macro")));
    };

    addMacro(macroGlue, "Macro Glue");
    addMacro(macroAir, "Macro Air");
    addMacro(macroSibilance, "Macro Sibilance");
    addMacro(macroPresence, "Macro Presence");
    addMacro(macroPunch, "Macro Punch");
    addMacro(macroBody, "Macro Body");
    addMacro(macroSmooth, "Macro Smooth");
    addMacro(macroDensity, "Macro Density");

    return { params.begin(), params.end() };
}

} // namespace razumov::params
