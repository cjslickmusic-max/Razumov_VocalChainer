cmake_minimum_required(VERSION 3.22)

project(RazumovVocalChain VERSION 0.9.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
endif()

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(
    RazumovVocalChain
    COMPANY_NAME "Razumov"
    BUNDLE_ID com.razumov.vocalchain
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Razu
    PLUGIN_CODE Rvch
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Razumov Vocal Chain"
    VERSION ${PROJECT_VERSION}
)

target_sources(
    RazumovVocalChain
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/ui/ChainStripComponent.cpp
        Source/presets/FactoryPresets.cpp
        Source/params/ParameterLayout.cpp
        Source/params/ModuleParamsRuntime.cpp
        Source/params/MicProfileChoices.cpp
        Source/dsp/graph/FilterNode.cpp
        Source/dsp/graph/LatencyNode.cpp
        Source/dsp/graph/FlexGraphDesc.cpp
        Source/dsp/graph/FlexGraphSerialization.cpp
        Source/dsp/graph/FlexGraphPlan.cpp
        Source/dsp/graph/AudioNodeFactory.cpp
        Source/dsp/graph/GraphPlanFactory.cpp
        Source/dsp/graph/GraphEngine.cpp
        Source/dsp/graph/MergeDelayPad.cpp
        Source/dsp/graph/MicCorrectionNode.cpp
        Source/dsp/graph/CompressorArchetypeNode.cpp
        Source/dsp/graph/DeesserNode.cpp
        Source/dsp/graph/ExciterNode.cpp
        Source/dsp/graph/SpectralCompressorNode.cpp
)

target_include_directories(
    RazumovVocalChain
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_compile_definitions(
    RazumovVocalChain
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(
    RazumovVocalChain
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_data_structures
)

add_executable(
    RazumovVocalChainTests
    Tests/TestMain.cpp
    Tests/GraphTests.cpp
    Tests/MergePdcTests.cpp
    Tests/DspDeterminismTests.cpp
    Source/dsp/graph/FilterNode.cpp
    Source/dsp/graph/LatencyNode.cpp
    Source/dsp/graph/FlexGraphDesc.cpp
    Source/dsp/graph/FlexGraphSerialization.cpp
    Source/dsp/graph/FlexGraphPlan.cpp
    Source/dsp/graph/AudioNodeFactory.cpp
    Source/dsp/graph/GraphPlanFactory.cpp
    Source/dsp/graph/GraphEngine.cpp
    Source/dsp/graph/MergeDelayPad.cpp
    Source/params/ModuleParamsRuntime.cpp
    Source/dsp/graph/MicCorrectionNode.cpp
    Source/dsp/graph/CompressorArchetypeNode.cpp
    Source/dsp/graph/DeesserNode.cpp
    Source/dsp/graph/ExciterNode.cpp
    Source/dsp/graph/SpectralCompressorNode.cpp
)

target_include_directories(
    RazumovVocalChainTests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
)

target_link_libraries(
    RazumovVocalChainTests
    PRIVATE
        juce::juce_dsp
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
)

enable_testing()
add_test(NAME GraphEngine COMMAND $<TARGET_FILE:RazumovVocalChainTests>)
