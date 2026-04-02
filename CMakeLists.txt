cmake_minimum_required(VERSION 3.22)

project(RazumovVocalChain VERSION 0.9.22 LANGUAGES C CXX)

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

juce_add_binary_data(
    RazumovVocalChainBinaryData
    SOURCES
        resources/ui/brushed_panel_placeholder.png
        resources/ui/editor_corner_accent.svg
        resources/ui/editor_corner_accent_r.svg
)

juce_add_plugin(
    RazumovVocalChain
    COMPANY_NAME "Razumov"
    BUNDLE_ID com.razumov.vocalchainer
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Razu
    PLUGIN_CODE Rvch
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Razumov VocalChainer"
    VERSION ${PROJECT_VERSION}
)

target_sources(
    RazumovVocalChain
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/ui/ChainStripComponent.cpp
        Source/ui/ChainStripLayout.cpp
        Source/ui/EditorVisualAssets.cpp
        Source/presets/FactoryPresets.cpp
        Source/params/ParameterLayout.cpp
        Source/params/MacroRouting.cpp
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
        Source/dsp/graph/SpectrumTap.cpp
        Source/dsp/graph/ParametricEqNode.cpp
        Source/dsp/graph/SpectrumAnalyzerNode.cpp
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
        RazumovVocalChainBinaryData
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_data_structures
)

# Always refresh the same repo path after VST3 build (REAPER: one extra VST3 folder, no manual copy).
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/Artifacts")
set(RAZUMOV_CANONICAL_VST3 "${CMAKE_SOURCE_DIR}/Artifacts/Razumov VocalChainer.vst3")
add_custom_command(
    TARGET RazumovVocalChain_VST3
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${RAZUMOV_CANONICAL_VST3}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_BUNDLE_DIR:RazumovVocalChain_VST3>" "${RAZUMOV_CANONICAL_VST3}"
    COMMENT "Copy VST3 to Artifacts/ (canonical)"
    VERBATIM
)

set(
    RAZUMOV_GRAPH_DSP_TEST_SOURCES
    Source/dsp/graph/FilterNode.cpp
    Source/dsp/graph/LatencyNode.cpp
    Source/dsp/graph/FlexGraphDesc.cpp
    Source/dsp/graph/FlexGraphSerialization.cpp
    Source/dsp/graph/FlexGraphPlan.cpp
    Source/dsp/graph/AudioNodeFactory.cpp
    Source/dsp/graph/GraphPlanFactory.cpp
    Source/dsp/graph/GraphEngine.cpp
    Source/dsp/graph/MergeDelayPad.cpp
    Source/params/MacroRouting.cpp
    Source/params/ModuleParamsRuntime.cpp
    Source/dsp/graph/MicCorrectionNode.cpp
    Source/dsp/graph/CompressorArchetypeNode.cpp
    Source/dsp/graph/DeesserNode.cpp
    Source/dsp/graph/ExciterNode.cpp
    Source/dsp/graph/SpectralCompressorNode.cpp
    Source/dsp/graph/SpectrumTap.cpp
    Source/dsp/graph/ParametricEqNode.cpp
    Source/dsp/graph/SpectrumAnalyzerNode.cpp
)

add_executable(
    RazumovVocalChainTests
    Tests/TestMain.cpp
    Tests/GraphTests.cpp
    Tests/MergePdcTests.cpp
    Tests/PhaseAlignTests.cpp
    Tests/FlexGraphSerializationTests.cpp
    Tests/DspDeterminismTests.cpp
    ${RAZUMOV_GRAPH_DSP_TEST_SOURCES}
)

add_executable(
    RazumovVocalChainStressTests
    Tests/StressMain.cpp
    Tests/GraphStressTests.cpp
    ${RAZUMOV_GRAPH_DSP_TEST_SOURCES}
)

target_include_directories(
    RazumovVocalChainTests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/Tests"
)

target_include_directories(
    RazumovVocalChainStressTests
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

target_link_libraries(
    RazumovVocalChainStressTests
    PRIVATE
        juce::juce_dsp
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
)

enable_testing()
add_test(NAME GraphEngine COMMAND $<TARGET_FILE:RazumovVocalChainTests>)
add_test(NAME GraphStress COMMAND $<TARGET_FILE:RazumovVocalChainStressTests>)
set_tests_properties(GraphStress PROPERTIES LABELS "long;stress" TIMEOUT 600)
