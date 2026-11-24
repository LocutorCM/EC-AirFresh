#include "PluginEditor.h"

ECAirEditor::ECAirEditor(ECAirProcessor& p) : AudioProcessorEditor(&p), processor(p) {
    setSize(900, 620);
    startTimerHz(30);
    auto styleBigKnob = [&](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF4FC3F7));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xFF29B6F6));
        addAndMakeVisible(s);
    };
    auto styleSmallKnob = [&](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF90A4AE));
        addAndMakeVisible(s);
    };
    styleBigKnob(midKnob); styleBigKnob(highKnob);
    styleSmallKnob(mixKnob); styleSmallKnob(outputKnob);
    powerBtn.setButtonText(""); powerBtn.setToggleState(true, juce::dontSendNotification);
    powerBtn.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF29B6F6));
    bypassBtn.setButtonText("BYPASS");
    bypassBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFECEFF1));
    addAndMakeVisible(powerBtn); addAndMakeVisible(bypassBtn);
    presetBox.addItemList({"Warm & Airy","Vocal Air","Master Shine","Drum Air","Solo High Air"}, 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xFF37474F));
    addAndMakeVisible(presetBox);
    sliderAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "MID_AIR", midKnob));
    sliderAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "HIGH_AIR", highKnob));
    sliderAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "MIX", mixKnob));
    sliderAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "OUTPUT", outputKnob));
    buttonAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, "POWER", powerBtn));
    buttonAtt.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, "BYPASS", bypassBtn));
    presetAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, "PRESET", presetBox);
    presetBox.onChange = [&] {
        auto id = presetBox.getSelectedId();
        auto* mid = processor.apvts.getRawParameterValue("MID_AIR");
        auto* high = processor.apvts.getRawParameterValue("HIGH_AIR");
        auto* mix = processor.apvts.getRawParameterValue("MIX");
        if (id == 1) { *mid = 45.0f; *high = 68.0f; *mix = 65.0f; }
        if (id == 2) { *mid = 60.0f; *high = 55.0f; *mix = 70.0f; }
        if (id == 3) { *mid = 35.0f; *high = 45.0f; *mix = 50.0f; }
        if (id == 4) { *mid = 70.0f; *high = 40.0f; *mix = 60.0f; }
        if (id == 5) { *mid = 0.0f; *high = 80.0f; *mix = 80.0f; }
    };
}
ECAirEditor::~ECAirEditor() { stopTimer(); }

void ECAirEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF455A64));
    g.setColour(juce::Colour(0xFFF0F5FA));
    g.fillRoundedRectangle(15, 15, 870, 590, 20.0f);
    g.setColour(juce::Colour(0xFF4A90C2));
    g.setFont(juce::Font(26.0f, juce::Font::bold));
    g.drawText("EC AUDIO", 65, 30, 200, 30, juce::Justification::left);
    g.setColour(juce::Colour(0xFF5B9BD5));
    g.setFont(juce::Font(36.0f, juce::Font::bold));
    g.drawText("EC AIR EXCITER", 250, 70, 400, 40, juce::Justification::centred);
    g.setColour(juce::Colour(0xFF78909C));
    g.setFont(17.0f);
    g.drawText("HARMONIC EXCITER - ADD AIR & CLARITY", 250, 110, 400, 20, juce::Justification::centred);
    g.setColour(juce::Colour(0xFF2E7AB5));
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("Mid Air", 190, 400, 150, 30, juce::Justification::centred);
    g.drawText("High Air", 560, 400, 150, 30, juce::Justification::centred);
    g.setColour(juce::Colour(0xFF4A90C2));
    g.setFont(17.0f);
    g.drawText("+2.4 dB", 190, 430, 150, 20, juce::Justification::centred);
    g.drawText("+4.1 dB", 560, 430, 150, 20, juce::Justification::centred);
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillRoundedRectangle(45, 470, 810, 75, 10.0f);
    g.setColour(juce::Colour(0xFF90A4AE));
    g.drawRoundedRectangle(45, 470, 810, 75, 10.0f, 1.0f);
    g.setColour(juce::Colour(0xFF546E7A));
    g.setFont(13.0f);
    g.drawText("POWER", 180, 495, 60, 20, juce::Justification::right);
    g.drawText("MIX", 340, 478, 60, 20, juce::Justification::centred);
    g.drawText("OUTPUT", 520, 478, 80, 20, juce::Justification::centred);
    g.setColour(juce::Colour(0xFF2E7AB5));
    g.drawText("65%", 340, 540, 60, 20, juce::Justification::centred);
    g.drawText("-0.5 dB", 520, 540, 80, 20, juce::Justification::centred);
    g.setColour(juce::Colour(0xFF78909C));
    g.setFont(13.0f);
    g.drawText("INPUT", 45, 560, 60, 20, juce::Justification::left);
    g.drawText("OUTPUT", 790, 560, 65, 20, juce::Justification::right);
    g.setColour(juce::Colour(0xFF4FC3F7).withAlpha(0.5f));
    g.drawLine(110, 570, 790, 570, 1.0f);
    g.setColour(juce::Colour(0xFF78909C));
    g.setFont(12.0f);
    g.drawText("VERSION 1.0.0 - 48kHz - STEREO - macOS", 45, 585, 350, 15, juce::Justification::left);
    g.drawText("SAFE - OVERSAMPLE 2x", 620, 585, 235, 15, juce::Justification::right);
    g.setColour(juce::Colour(0xFF78909C));
    g.setFont(14.0f);
    g.drawText("Preset: Warm & Airy", 610, 32, 160, 22, juce::Justification::right);
}

void ECAirEditor::resized() {
    presetBox.setBounds(610, 28, 220, 32);
    midKnob.setBounds(180, 170, 170, 170);
    highKnob.setBounds(550, 170, 170, 170);
    powerBtn.setBounds(245, 495, 40, 24);
    bypassBtn.setBounds(300, 493, 80, 28);
    mixKnob.setBounds(325, 495, 50, 50);
    outputKnob.setBounds(515, 495, 50, 50);
}
void ECAirEditor::timerCallback() { repaint(); }
