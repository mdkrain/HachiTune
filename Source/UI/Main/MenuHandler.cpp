#include "MenuHandler.h"

MenuHandler::MenuHandler() = default;

juce::StringArray MenuHandler::getMenuBarNames() {
    if (pluginMode)
        return {TR("menu.edit"), TR("menu.view"), TR("menu.settings")};
    return {TR("menu.file"), TR("menu.edit"), TR("menu.view"), TR("menu.settings")};
}

juce::PopupMenu MenuHandler::getMenuForIndex(int menuIndex, const juce::String& /*menuName*/) {
    juce::PopupMenu menu;

    if (pluginMode) {
        if (menuIndex == 0) {
            // Edit menu
            bool canUndo = undoManager && undoManager->canUndo();
            bool canRedo = undoManager && undoManager->canRedo();
            menu.addItem(MenuUndo, TR("menu.undo"), canUndo);
            menu.addItem(MenuRedo, TR("menu.redo"), canRedo);
        } else if (menuIndex == 1) {
            // View menu
            menu.addItem(MenuShowDeltaPitch, TR("menu.show_delta_pitch"), true, showDeltaPitch);
            menu.addItem(MenuShowBasePitch, TR("menu.show_base_pitch"), true, showBasePitch);
        } else if (menuIndex == 2) {
            // Settings menu
            menu.addItem(MenuSettings, TR("menu.settings"));
        }
    } else {
        if (menuIndex == 0) {
            // File menu
            menu.addItem(MenuOpen, TR("menu.open"));
            menu.addItem(MenuSave, TR("menu.save"));
            menu.addSeparator();
            menu.addItem(MenuExport, TR("menu.export"));
            menu.addItem(MenuExportMidi, TR("menu.export_midi"));
            menu.addSeparator();
            menu.addItem(MenuQuit, TR("menu.quit"));
        } else if (menuIndex == 1) {
            // Edit menu
            bool canUndo = undoManager && undoManager->canUndo();
            bool canRedo = undoManager && undoManager->canRedo();
            menu.addItem(MenuUndo, TR("menu.undo"), canUndo);
            menu.addItem(MenuRedo, TR("menu.redo"), canRedo);
        } else if (menuIndex == 2) {
            // View menu
            menu.addItem(MenuShowDeltaPitch, TR("menu.show_delta_pitch"), true, showDeltaPitch);
            menu.addItem(MenuShowBasePitch, TR("menu.show_base_pitch"), true, showBasePitch);
        } else if (menuIndex == 3) {
            // Settings menu
            menu.addItem(MenuSettings, TR("menu.settings"));
        }
    }

    return menu;
}

void MenuHandler::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/) {
    switch (menuItemID) {
        case MenuOpen:
            if (onOpenFile) onOpenFile();
            break;
        case MenuSave:
            if (onSaveProject) onSaveProject();
            break;
        case MenuExport:
            if (onExportFile) onExportFile();
            break;
        case MenuExportMidi:
            if (onExportMidi) onExportMidi();
            break;
        case MenuQuit:
            if (onQuit) onQuit();
            break;
        case MenuUndo:
            if (onUndo) onUndo();
            break;
        case MenuRedo:
            if (onRedo) onRedo();
            break;
        case MenuSettings:
            if (onShowSettings) onShowSettings();
            break;
        case MenuShowDeltaPitch:
            showDeltaPitch = !showDeltaPitch;
            if (onShowDeltaPitchChanged) onShowDeltaPitchChanged(showDeltaPitch);
            menuItemsChanged();
            break;
        case MenuShowBasePitch:
            showBasePitch = !showBasePitch;
            if (onShowBasePitchChanged) onShowBasePitchChanged(showBasePitch);
            menuItemsChanged();
            break;
        default:
            break;
    }
}
