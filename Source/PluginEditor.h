#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class EC_AIRAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    EC_AIRAudioProcessorEditor(EC_AIRAudioProcessor&);
    ~EC_AIRAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    EC_AIRAudioProcessor& audioProcessor;

    juce::Slider midAirSlider;
    juce::Slider highAirSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;
    juce::TextButton powerButton;
    juce::TextButton presetAirButton;
    juce::TextButton presetVocalButton;
    juce::TextButton presetMasterButton;

    juce::Label midAirLabel;
    juce::Label highAirLabel;
    juce::Label mixLabel;
    juce::Label outputLabel;
    juce::Label titleLabel;
    juce::Label valueLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAirAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highAirAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;

    void applyPreset(float mid, float high, float mix, float out);
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EC_AIRAudioProcessorEditor)
};
