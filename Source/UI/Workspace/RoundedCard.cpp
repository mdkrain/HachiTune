#include "RoundedCard.h"

RoundedCard::RoundedCard()
{
    setOpaque(false);
}

void RoundedCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background gradient for subtle depth
    auto topColor = backgroundColour.brighter(0.08f);
    auto bottomColor = backgroundColour.darker(0.06f);
    juce::ColourGradient bgGradient(topColor, bounds.getX(), bounds.getY(),
                                    bottomColor, bounds.getX(),
                                    bounds.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, cornerRadius);

    // Inner highlight
    g.setColour(APP_COLOR_BORDER_SUBTLE.withAlpha(0.7f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), cornerRadius - 1.0f, 1.0f);

    // Outer border
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void RoundedCard::paintOverChildren(juce::Graphics& g)
{
    // Create a rounded rectangle path for clipping effect
    auto bounds = getLocalBounds().toFloat();
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);

    // Draw border on top to create clean rounded edge
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
}

void RoundedCard::resized()
{
    if (contentComponent != nullptr)
    {
        contentComponent->setBounds(getLocalBounds().reduced(padding));
    }
}

void RoundedCard::setContentComponent(juce::Component* content)
{
    if (contentComponent != nullptr)
        removeChildComponent(contentComponent);

    contentComponent = content;

    if (contentComponent != nullptr)
    {
        addAndMakeVisible(contentComponent);
        resized();
    }
}
