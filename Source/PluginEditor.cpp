    using Joint = juce::PathStrokeType::JointStyle;
    using Cap = juce::PathStrokeType::EndCapStyle;

    g.setColour(juce::Colour(0x40000000));
    g.fillEllipse(centre.x - radius + 3, centre.y - radius + 5, radius * 2, radius * 2);

    juce::ColourGradient grad(juce::Colour(0xff3a4152), centre.x, centre.y - radius,
                             juce::Colour(0xff14171f), centre.x, centre.y + radius, false);
    g.setGradientFill(grad);
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2);

    g.setColour(juce::Colour(0xff1a1d26));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2, radius * 2, 1.5f);

    if (isEnabled)
    {
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, radius + 4, radius + 4, 0, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xff00d4ff));
        g.strokePath(arc, juce::PathStrokeType(3.5f, Joint::curved, Cap::rounded));
        g.setColour(juce::Colour(0x3300d4ff));
        g.strokePath(arc, juce::PathStrokeType(8.0f, Joint::curved, Cap::rounded));
    }
    else
    {
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, radius + 4, radius + 4, 0, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff333333));
        g.strokePath(arc, juce::PathStrokeType(3.5f, Joint::curved, Cap::rounded));
    }
