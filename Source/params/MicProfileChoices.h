#pragma once

#include <juce_core/juce_core.h>

namespace razumov::params
{

/** Список для AudioParameterChoice: Off + имена из resources/mic_dataset (пока без загрузки в DSP). */
juce::StringArray buildMicProfileChoiceStrings();

} // namespace razumov::params
