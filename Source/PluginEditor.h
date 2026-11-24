#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ECAirEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
    ECAirEditor(ECAirProcessor&);
    ~ECAirEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
private:
    ECAirProcessor& processor;
    juce::Slider midKnob, highKnob, mixKnob, outputKnob;
    juce::ToggleButton powerBtn, bypassBtn;
    juce::ComboBox presetBox;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAtt;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ECAirEditor)
};
