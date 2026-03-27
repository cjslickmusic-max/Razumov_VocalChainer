#include "ParameterLayout.h"
#include "MicProfileChoices.h"
#include "ParamIDs.h"

namespace razumov::params
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back(std::make_unique<AudioParameterBool>(ParameterID { micBypass, 1 }, "Mic bypass", false));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { micAmount, 1 },
        "Mic amount",
        NormalisableRange<float> { 0.0f, 1.0f, 0.01f },
        1.0f,
        AudioParameterFloatAttributes().withLabel("amt")));

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

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { deessCrossoverHz, 1 },
        "De-ess Xover",
        NormalisableRange<float> { 3000.0f, 10000.0f, 1.0f },
        6000.0f,
        AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { deessThresholdDb, 1 },
        "De-ess thr",
        NormalisableRange<float> { -60.0f, 0.0f, 0.1f },
        -20.0f,
        AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { deessRatio, 1 },
        "De-ess ratio",
        NormalisableRange<float> { 1.0f, 10.0f, 0.1f },
        3.0f));

    auto addCompParams = [&](const char* idThresh, const char* idRatio, const char* idMakeup, const char* name,
                             float defThresh, float defRatio) {
        juce::String prefix(name);
        params.push_back(std::make_unique<AudioParameterFloat>(
            ParameterID { idThresh, 1 },
            prefix + " thr",
            NormalisableRange<float> { -60.0f, 0.0f, 0.1f },
            defThresh,
            AudioParameterFloatAttributes().withLabel("dB")));
        params.push_back(std::make_unique<AudioParameterFloat>(
            ParameterID { idRatio, 1 },
            prefix + " ratio",
            NormalisableRange<float> { 1.0f, 20.0f, 0.1f },
            defRatio));
        params.push_back(std::make_unique<AudioParameterFloat>(
            ParameterID { idMakeup, 1 },
            prefix + " makeup",
            NormalisableRange<float> { 0.0f, 24.0f, 0.1f },
            0.0f,
            AudioParameterFloatAttributes().withLabel("dB")));
    };

    addCompParams(optoThresholdDb, optoRatio, optoMakeupDb, "Opto", -12.0f, 3.0f);
    addCompParams(fetThresholdDb, fetRatio, fetMakeupDb, "FET", -12.0f, 4.0f);
    addCompParams(vcaThresholdDb, vcaRatio, vcaMakeupDb, "VCA", -12.0f, 2.5f);

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { exciterDrive, 1 },
        "Exciter drive",
        NormalisableRange<float> { 0.1f, 8.0f, 0.01f },
        1.5f));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { exciterMix, 1 },
        "Exciter mix",
        NormalisableRange<float> { 0.0f, 1.0f, 0.01f },
        0.15f));

    params.push_back(std::make_unique<AudioParameterBool>(ParameterID { spectralBypass, 1 }, "Spectral bypass", false));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { spectralMix, 1 },
        "Spectral mix",
        NormalisableRange<float> { 0.0f, 1.0f, 0.01f },
        0.75f,
        AudioParameterFloatAttributes().withLabel("wet")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { spectralThresholdDb, 1 },
        "Spectral thr",
        NormalisableRange<float> { -60.0f, 0.0f, 0.1f },
        -24.0f,
        AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID { spectralRatio, 1 },
        "Spectral ratio",
        NormalisableRange<float> { 1.0f, 20.0f, 0.1f },
        3.0f));

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID { micProfile, 1 },
        "Mic profile",
        buildMicProfileChoiceStrings(),
        0));

    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID { chainProfile, 1 },
        "Chain",
        StringArray { "Full (Opto→FET→VCA)", "Compact (Opto only)", "FET-forward (FET→Opto→VCA)" },
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

    return { params.begin(), params.end() };
}

} // namespace razumov::params
