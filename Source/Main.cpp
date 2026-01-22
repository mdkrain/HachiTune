// Main.cpp - Cross-platform entry point (macOS uses native menu inside
// MainComponent)

#include "JuceHeader.h"
#include "UI/MainComponent.h"
#include "UI/StyledComponents.h"
#include "Utils/AppLogger.h"
#include "Utils/Constants.h"
#include "Utils/Localization.h"
#include "Utils/PlatformUtils.h"
#include "Utils/WindowSizing.h"

#if JUCE_WINDOWS
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

class HachiTuneApplication : public juce::JUCEApplication {
public:
  HachiTuneApplication() {}

  const juce::String getApplicationName() override { return "HachiTune"; }

  const juce::String getApplicationVersion() override { return "1.0.0"; }

  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String &commandLine) override {
    juce::ignoreUnused(commandLine);
    AppLogger::init();
    LOG("========== APP STARTING ==========");
    LOG("Initializing fonts...");
    AppFont::initialize();
    LOG("Loading localization...");
    Localization::loadFromSettings();
    LOG("Localization loaded, creating MainWindow...");
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
    LOG("MainWindow created and visible");
  }

  void shutdown() override {
    mainWindow = nullptr;
    AppFont::shutdown(); // Release font resources before JUCE shuts down
  }

  void systemRequestedQuit() override { quit(); }

  void anotherInstanceStarted(const juce::String &commandLine) override {
    juce::ignoreUnused(commandLine);
  }

  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Colour(APP_COLOR_BACKGROUND),
                         DocumentWindow::allButtons,
                         false) // Don't add to desktop yet
    {
      LOG("MainWindow: constructor start");

      // Ensure window is opaque - this must be set before any
      // transparency-related operations
      setOpaque(true);

      LOG("MainWindow: creating MainComponent...");
      // Set content first, ensuring it's also opaque
      auto *content = new MainComponent();
      LOG("MainWindow: MainComponent created");
      content->setOpaque(true);
      setContentOwned(content, true);

      // Now set native title bar after content is set
      setUsingNativeTitleBar(true);

      setResizable(true, true);

      // Ensure window is still opaque before adding to desktop
      // (some operations might affect opacity state)
      setOpaque(true);

      LOG("MainWindow: adding to desktop...");
      // Now add to desktop after all properties are set
      addToDesktop();

      auto *display = WindowSizing::getDisplayForComponent(this);
      auto constraints = WindowSizing::Constraints();
      auto desiredSize = content->getSavedWindowSize();
      if (desiredSize.x <= 0 || desiredSize.y <= 0)
        desiredSize = {WindowSizing::kDefaultWidth, WindowSizing::kDefaultHeight};

      if (display != nullptr) {
        auto initialBounds = WindowSizing::getInitialBounds(
            desiredSize.x, desiredSize.y, *display, constraints);
        auto maxBounds = WindowSizing::getMaxBounds(*display);
        setBounds(initialBounds);
        setResizeLimits(constraints.minWidth, constraints.minHeight,
                        maxBounds.getWidth(), maxBounds.getHeight());
      } else {
        setSize(desiredSize.x, desiredSize.y);
        centreWithSize(getWidth(), getHeight());
      }

      LOG("MainWindow: initial size " + juce::String(getWidth()) + "x" +
          juce::String(getHeight()));
      setVisible(true);
      LOG("MainWindow: setVisible(true) done");

#if JUCE_WINDOWS
      // Enable dark mode for title bar
      if (auto *peer = getPeer()) {
        if (auto hwnd = (HWND)peer->getNativeHandle()) {
          // Enable immersive dark mode for title bar
          constexpr DWORD darkMode = 1;
          DwmSetWindowAttribute(hwnd, 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/,
                                &darkMode, sizeof(darkMode));

          // Enable rounded corners on Windows 11+
          DWORD preference = 2; // DWMWCP_ROUND
          DwmSetWindowAttribute(hwnd, 33 /*DWMWA_WINDOW_CORNER_PREFERENCE*/,
                                &preference, sizeof(preference));
        }
      }
#elif JUCE_MAC
      // Enable dark mode for macOS window
      if (auto *peer = getPeer())
        PlatformUtils::setDarkAppearance(peer->getNativeHandle());
#endif
    }

    void closeButtonPressed() override {
      JUCEApplication::getInstance()->systemRequestedQuit();
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(HachiTuneApplication)
