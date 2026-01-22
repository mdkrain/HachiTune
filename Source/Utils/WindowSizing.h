#pragma once

#include "../JuceHeader.h"

namespace WindowSizing {

struct Constraints {
  int minWidth = 960;
  int minHeight = 600;
  float initialMaxFraction = 0.92f;
  int initialMargin = 24;
};

constexpr int kDefaultWidth = 1400;
constexpr int kDefaultHeight = 900;

const juce::Displays::Display *getPrimaryDisplay();
const juce::Displays::Display *getDisplayForComponent(
    const juce::Component *component);

juce::Rectangle<int> getInitialBounds(int desiredWidth, int desiredHeight,
                                      const juce::Displays::Display &display,
                                      const Constraints &constraints);

juce::Point<int> getClampedSize(int desiredWidth, int desiredHeight,
                                const juce::Displays::Display &display,
                                const Constraints &constraints);

juce::Rectangle<int> getMaxBounds(const juce::Displays::Display &display);

} // namespace WindowSizing
