cmake_minimum_required(VERSION 3.22)
project(EC_AIR VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.9
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(EC_AIR
    COMPANY_NAME "EC AUDIO"
    COMPANY_WEBSITE "https://ecaudio.com"
    COMPANY_EMAIL "info@ecaudio.com"
    PLUGIN_MANUFACTURER_CODE ECAu
    PLUGIN_CODE AiAr
    FORMATS VST3 AU
    PRODUCT_NAME "EC AIR EXCITER"
    VERSION "1.0.0"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "Enhancement" "Exciter"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

juce_generate_juce_header(EC_AIR)

target_sources(EC_AIR PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(EC_AIR PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(EC_AIR PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
)
