cmake_minimum_required(VERSION 3.22)
project(LynxTuner VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LynxTuner
    COMPANY_NAME "Caliel666"
    BUNDLE_ID "com.caliel666.lynxtuner"
    PLUGIN_MANUFACTURER_CODE Ka1S
    PLUGIN_CODE Lynx
    PRODUCT_NAME "LYNX Tuner"
    DESCRIPTION "Real-time vocal pitch correction"
    FORMATS VST3
    VST3_CATEGORIES Fx Tools
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(LynxTuner)

target_sources(LynxTuner PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/LynxTune.cpp
    Source/DSP/VocalPitch.cpp)

target_include_directories(LynxTuner PRIVATE Source)
target_compile_definitions(LynxTuner PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(LynxTuner PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

if(MSVC)
    target_compile_options(LynxTuner PRIVATE /W4 /permissive-)
else()
    target_compile_options(LynxTuner PRIVATE -Wall -Wextra -Wpedantic)
endif()
