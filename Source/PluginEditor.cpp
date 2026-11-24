#include "PluginProcessor.h"
#include "PluginEditor.h"

EC_AIRAudioProcessorEditor::EC_AIRAudioProcessorEditor(EC_AIRAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(620, 340);

    setupSlider(midAirSlider, midAirLabel, "MID AIR");
    setupSlider(highAirSlider, highAirLabel, "HIGH AIR");
    setupSlider(mixSlider, mixLabel, "MIX");
    setupSlider(outputSlider, outputLabel, "OUTPUT");

    midAirAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "midAir", midAirSlider);
    highAirAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "highAir", highAirSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "output", outputSlider);

    powerButton.setButtonText("POWER");
    powerButton.setClickingTogglesState(true);
    powerButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2ecc71));
    powerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
    powerButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    powerButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(powerButton);
    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "power", powerButton);

    auto setupPreset = [this](juce::TextButton& btn, const juce::String& name)
    {
        btn.setButtonText(name);
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(btn);
    };
    setupPreset(presetAirButton, "AIR");
    setupPreset(presetVocalButton, "VOCAL");
    setupPreset(presetMasterButton, "MASTER");

    presetAirButton.onClick = [this] { applyPreset(2.4f, 4.1f, 65.0f, -0.5f); };
    presetVocalButton.onClick = [this] { applyPreset(3.5f, 5.0f, 55.0f, -1.0f); };
    presetMasterButton.onClick = [this] { applyPreset(1.2f, 2.0f, 40.0f, -0.2f); };

    titleLabel.setText("EC AIR EXCITER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    valueLabel.setText("", juce::dontSendNotification);
    valueLabel.setFont(juce::Font(13.0f));
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
    valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueLabel);

    startTimerHz(15);
}

EC_AIRAudioProcessorEditor::~EC_AIRAudioProcessorEditor()
{
    stopTimer();
}

void EC_AIRAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00d4ff));
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void EC_AIRAudioProcessorEditor::applyPreset(float mid, float high, float mix, float out)
{
    auto* midP = audioProcessor.apvts.getParameter("midAir");
    auto* highP = audioProcessor.apvts.getParameter("highAir");
    auto* mixP = audioProcessor.apvts.getParameter("mix");
    auto* outP = audioProcessor.apvts.getParameter("output");
    if (midP != nullptr) midP->setValueNotifyingHost(midP->convertTo0to1(mid));
    if (highP != nullptr) highP->setValueNotifyingHost(highP->convertTo0to1(high));
    if (mixP != nullptr) mixP->setValueNotifyingHost(mixP->convertTo0to1(mix));
    if (outP != nullptr) outP->setValueNotifyingHost(outP->convertTo0to1(out));
}

void EC_AIRAudioProcessorEditor::timerCallback()
{
    auto getVal = [this](const char* id) -> float
    {
        auto* v = audioProcessor.apvts.getRawParameterValue(id);
        return v != nullptr ? v->load() : 0.0f;
    };

    float mid = getVal("midAir");
    float high = getVal("highAir");
    float mix = getVal("mix");
    float out = getVal("output");

    valueLabel.setText("MID " + juce::String(mid, 1) + " dB  |  HIGH " + juce::String(high, 1) + " dB  |  MIX " + juce::String((int) mix) + "%  |  OUT " + juce::String(out, 1) + " dB", juce::dontSendNotification);

    auto* powerParam = audioProcessor.apvts.getRawParameterValue("power");
    if (powerParam != nullptr)
    {
        bool isOn = powerParam->load() > 0.5f;
        midAirSlider.setEnabled(isOn);
        highAirSlider.setEnabled(isOn);
        mixSlider.setEnabled(isOn);
        outputSlider.setEnabled(isOn);
    }
}

void EC_AIRAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111111));
    g.setColour(juce::Colour(0xff00d4ff));
    g.drawLine(0, 48, (float) getWidth(), 48, 1.0f);
}

void EC_AIRAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds(area.removeFromTop(48));
    auto bottom = area.removeFromBottom(52);
    valueLabel.setBounds(bottom.removeFromTop(24));
    auto presetRow = bottom.removeFromBottom(28);
    presetAirButton.setBounds(presetRow.removeFromLeft(90).reduced(4));
    presetVocalButton.setBounds(presetRow.removeFromLeft(90).reduced(4));
    presetMasterButton.setBounds(presetRow.removeFromLeft(90).reduced(4));
    powerButton.setBounds(presetRow.removeFromRight(110).reduced(4));

    auto knobs = area.reduced(10);
    int w = knobs.getWidth() / 4;
    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int x)
    {
        s.setBounds(x, knobs.getY() + 10, w - 10, 150);
        l.setBounds(x, knobs.getY() + 160, w - 10, 20);
    };
    placeKnob(midAirSlider, midAirLabel, knobs.getX());
    placeKnob(highAirSlider, highAirLabel, knobs.getX() + w);
    placeKnob(mixSlider, mixLabel, knobs.getX() + w * 2);
    placeKnob(outputSlider, outputLabel, knobs.getX() + w * 3);
}
