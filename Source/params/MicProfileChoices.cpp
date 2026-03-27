#include "MicProfileChoices.h"

namespace razumov::params
{
namespace
{
/** Базовые имена файлов *.csv в resources/mic_dataset (порядок стабилен для индексов пресетов). */
constexpr const char* kCsvStems[] = {
    "AKG_C12",
    "AKG_C214",
    "AKG_C414",
    "AKG_P120",
    "AT2020",
    "ATR2100x",
    "Aston_Origin",
    "BM800",
    "Behringer_XM8500",
    "Coles_4038",
    "EV_RE20",
    "Fifine_K669",
    "Lewitt_440",
    "MXL_990",
    "Neumann_TLM102",
    "Neumann_TLM103",
    "Neumann_U67",
    "Neumann_U87",
    "Rode_NT1A",
    "Rode_NT2A",
    "Royer_R121",
    "Samson_Q2U",
    "Sennheiser_MK4",
    "Shure_KSM44",
    "Shure_SM57",
    "Shure_SM58",
    "Shure_SM7B",
    "Sony_C800G",
    "Superlux_PRA_D1",
    "Telefunken_U47",
};

} // namespace

juce::StringArray buildMicProfileChoiceStrings()
{
    juce::StringArray a;
    a.add("Off");
    for (auto stem : kCsvStems)
    {
        juce::String s(stem);
        s = s.replaceCharacter('_', ' ');
        a.add(s);
    }
    return a;
}

} // namespace razumov::params
