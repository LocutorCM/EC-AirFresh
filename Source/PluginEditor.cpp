#include "PluginEditor.h"

EC_AIRAudioProcessorEditor::EC_AIRAudioProcessorEditor(EC_AIRAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 600);
    setLookAndFeel(&look);

    auto setupKnob = [this](juce::Slider& s, juce::Label& label, const char* text)
    {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setLookAndFeel(&look);
        addAndMakeVisible(s);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xff2a7ab5));
        label.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        addAndMakeVisible(label);
    };

    setupKnob(midAir, midLabel, "Mid Air");
    setupKnob(highAir, highLabel, "High Air");
    setupKnob(mix, mixLabel, "MIX");
    setupKnob(output, outLabel, "OUTPUT");

    midAttach = std::make_unique<Attach>(p.apvts, "midAir", midAir);
    highAttach = std::make_unique<Attach>(p.apvts, "highAir", highAir);
    mixAttach = std::make_unique<Attach>(p.apvts, "mix", mix);
    outAttach = std::make_unique<Attach>(p.apvts, "output", output);

    // POWER - si esta apagado, no suena el efecto
    power.setLookAndFeel(&look);
    power.setButtonText("");
    addAndMakeVisible(power);
    powerAttach = std::make_unique<ButtonAttach>(p.apvts, "power", power);

    // BYPASS - REAL: apaga POWER, y si lo vuelves a picar lo prende
    bypass.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffeef6fb));
    bypass.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff78909c));
    bypass.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd32f2f));
    bypass.setClickingTogglesState(true);
    bypass.onClick = [this]()
    {
        auto* param = audioProcessor.apvts.getParameter("power");
        if (param != nullptr)
        {
            // Si BYPASS esta ON -> power = 0, si esta OFF -> power = 1
            float newPower = bypass.getToggleState() ? 0.0f : 1.0f;
            param->setValueNotifyingHost(newPower);
            // Sincroniza el switch de POWER con el boton
            power.setToggleState(newPower > 0.5f, juce::dontSendNotification);
        }
    };
    addAndMakeVisible(bypass);

    // PRESET - REAL
    presetBox.addItem("Warm & Airy", 1);
    presetBox.addItem("Bright Vocals", 2);
    presetBox.addItem("Master Sparkle", 3);
    presetBox.addItem("Subtle Air", 4);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    presetBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xff78909c));
    presetBox.onChange = [this]() { applyPreset(presetBox.getSelectedId()); };
    addAndMakeVisible(presetBox);

    auto setupValue = [this](juce::Label& l)
    {
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colour(0xff2a7ab5));
        l.setFont(juce::FontOptions(16.0f));
        addAndMakeVisible(l);
    };
    setupValue(midValue);
    setupValue(highValue);
    setupValue(mixValue);
    setupValue(outValue);

    addAndMakeVisible(waveform);

    updateLabels();
    startTimerHz(30); // Actualiza los numeritos 30 veces por segundo
}

EC_AIRAudioProcessorEditor::~EC_AIRAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void EC_AIRAudioProcessorEditor::timerCallback()
{
    updateLabels();

    // Mantiene BYPASS sincronizado si mueves POWER a mano
    auto* powerParam = audioProcessor.apvts.getRawParameterValue("power");
    if (powerParam != nullptr)
    {
        bool isOn = *powerParam > 0.5f;
        bypass.setToggleState(!isOn, juce::dontSendNotification);
        bypass.setButtonText(!isOn ? "BYPASSED" : "BYPASS");
        bypass.setColour(juce::TextButton::buttonColourId,
                         !isOn ? juce::Colour(0xffffebee) : juce::Colour(0xffeef6fb));
    }
}

void EC_AIRAudioProcessorEditor::updateLabels()
{
    auto getVal = [this](const char* id) -> float
    {
        auto* v = audioProcessor.apvts.getRawParameterValue(id);
        return v != nullptr ? *v : 0.0f;
    };

    // Aqui asumimos: midAir/highAir/output en dB, mix en 0-100
    // Si en tu Processor son 0.0-1.0, cambialos abajo
    float mid = getVal("midAir");
    float high = getVal("highAir");
    float mx = getVal("mix");
    float out = getVal("output");

    // Si tus params son normalizados 0-1, descomenta estas 3 lineas:
    // mid = mid * 12.0f; // 0 a +12dB
    // high = high * 12.0f;
    // out = (out * 24.0f) - 12.0f; // -12 a +12dB

    midValue.setText(juce::String(mid, 1) + " dB", juce::dontSendNotification);
    highValue.setText(juce::String(high, 1) + " dB", juce::dontSendNotification);
    mixValue.setText(juce::String((int)mx) + "%", juce::dontSendNotification);
    outValue.setText(juce::String(out, 1) + " dB", juce::dontSendNotification);
}

void EC_AIRAudioProcessorEditor::applyPreset(int index)
{
    auto setParam = [this](const char* id, float value)
    {
        auto* param = audioProcessor.apvts.getParameter(id);
        if (param != nullptr)
        {
            // Convierte valor real a 0-1 segun el rango del param
            auto range = param->getNormalisableRange();
            param->setValueNotifyingHost(range.convertTo0to1(value));
        }
    };

    // AJUSTA ESTOS VALORES A TUS RANGOS REALES
    // Formato: midAir dB, highAir dB, mix %, output dB
    switch (index)
    {
        case 1: // Warm & Airy
            setParam("midAir", 2.4f); setParam("highAir", 4.1f);
            setParam("mix", 65.0f); setParam("output", -0.5f);
            break;
        case 2: // Bright Vocals
            setParam("midAir", 3.5f); setParam("highAir", 6.0f);
            setParam("mix", 75.0f); setParam("output", -1.0f);
            break;
        case 3: // Master Sparkle
            setParam("midAir", 1.5f); setParam("highAir", 2.8f);
            setParam("mix", 45.0f); setParam("output", -0.2f);
            break;
        case 4: // Subtle Air
            setParam("midAir", 0.8f); setParam("highAir", 1.2f);
            setParam("mix", 30.0f); setParam("output", 0.0f);
            break;
        default: break;
    }
    // Asegura que POWER quede prendido al cambiar preset
    auto* powerParam = audioProcessor.apvts.getParameter("power");
    if (powerParam != nullptr) powerParam->setValueNotifyingHost(1.0f);
    updateLabels();
}

void EC_AIRAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bg(juce::Colour(0xffffffff), 0, 0,
                            juce::Colour(0xffdbeefb), 0, (float)getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();
    g.setColour(juce::Colour(0xffb0d4ea));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 18.0f, 1.0f);

    g.setColour(juce::Colour(0xff2a7ab5));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("EC AUDIO", 40, 20, 200, 30, juce::Justification::left);

    g.setColour(juce::Colour(0xff2a7ab5));
    g.setFont(juce::FontOptions(42.0f, juce::Font::bold));
    g.drawText("EC AIR EXCITER", 0, 70, getWidth(), 50, juce::Justification::centred);

    g.setColour(juce::Colour(0xff78909c));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("HARMONIC EXCITER — ADD AIR & CLARITY", 0, 118, getWidth(), 25, juce::Justification::centred);

    g.setColour(juce::Colour(0xfff2f8fc));
    g.fillRoundedRectangle(40, 400, getWidth() - 80, 90, 12.0f);
    g.setColour(juce::Colour(0xffcfe8f5));
    g.drawRoundedRectangle(juce::Rectangle<float>(40, 400, getWidth() - 80, 90), 12.0f, 1.0f);

    g.setColour(juce::Colour(0xff78909c));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("POWER", 140, 428, 70, 20, juce::Justification::right);
    g.drawText("INPUT", 40, 540, 100, 20, juce::Justification::left);
    g.drawText("OUTPUT", getWidth() - 140, 540, 100, 20, juce::Justification::right);
    g.drawText("VERSION 1.0  •  48kHz  •  STEREO", 40, 565, 300, 20, juce::Justification::left);
    g.drawText("SAFE  •  OVERSAMPLE 2x", getWidth() - 340, 565, 300, 20, juce::Justification::right);
}

void EC_AIRAudioProcessorEditor::resized()
{
    midAir.setBounds(120, 165, 200, 200);
    highAir.setBounds(480, 165, 200, 200);
    midLabel.setBounds(120, 365, 200, 30);
    highLabel.setBounds(480, 365, 200, 30);
    midValue.setBounds(120, 395, 200, 20);
    highValue.setBounds(480, 395, 200, 20);

    presetBox.setBounds(540, 22, 200, 26);

    power.setBounds(215, 428, 55, 28);
    bypass.setBounds(285, 428, 80, 28);
    mix.setBounds(400, 415, 65, 65);
    output.setBounds(560, 415, 65, 65);
    mixLabel.setBounds(400, 398, 65, 20);
    outLabel.setBounds(560, 398, 65, 20);
    mixValue.setBounds(400, 478, 65, 18);
    outValue.setBounds(560, 478, 65, 18);

    waveform.setBounds(40, 500, getWidth() - 80, 38);
}
