#include "JuceHeader.h"
#include "MainComponent.hpp"




//==============================================================================
class MonocleApplication : public JUCEApplication
{
public:




    //==========================================================================
    class MainMenuBarModel : public MenuBarModel
    {
    public:
        MainMenuBarModel();
        StringArray getMenuBarNames() override;
        PopupMenu getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName) override;
        void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override;
    };




    //==========================================================================
    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (String name);
        void closeButtonPressed() override;
        void toggleOpenGL();
        OpenGLContext context;
    };




    //==========================================================================
    MonocleApplication()
    {
    }

    static MonocleApplication& getApp()
    {
        return *dynamic_cast<MonocleApplication*> (JUCEApplication::getInstance());
    }

    MenuBarModel& getMenuModel()
    {
        return *menuModel;
    }

    StringArray getMenuNames()
    {
        return { "File", "Edit", "View", "Window", "Document", "Tools", "Help" };
    }

    PopupMenu createMenu (const String& menuName)
    {
        PopupMenu menu;

        if      (menuName == "File")        createFileMenu (menu);
        else if (menuName == "Edit")        createEditMenu (menu);
        else if (menuName == "View")        {}
        else if (menuName == "Window")      createWindowMenu (menu);
        else if (menuName == "Document")    {}
        else if (menuName == "Tools")       {}
        else if (menuName == "Help")        {}
        else                                jassertfalse;
        return menu;
    }

    void createFileMenu (PopupMenu& menu)
    {
        menu.addCommandItem (&commandManager, CommandIDs::openDocument);
    }

    void createEditMenu (PopupMenu& menu)
    {
        menu.addCommandItem (&commandManager, CommandIDs::kernelCreateRule);
    }

    void createWindowMenu (PopupMenu& menu)
    {
        menu.addCommandItem (&commandManager, CommandIDs::windowToggleNavPages);
        menu.addCommandItem (&commandManager, CommandIDs::windowToggleBackdrop);
        menu.addCommandItem (&commandManager, CommandIDs::windowToggleOpenGL);
    }

    void configureLookAndFeel()
    {
        auto& laf = Desktop::getInstance().getDefaultLookAndFeel();
        laf.setColour (TextEditor::backgroundColourId, Colours::white);
        laf.setColour (TextEditor::textColourId, Colours::black);
        laf.setColour (TextEditor::highlightColourId, Colours::lightblue);
        laf.setColour (TextEditor::highlightedTextColourId, Colours::black);
        laf.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        laf.setColour (TextEditor::focusedOutlineColourId, Colours::lightblue);
        laf.setColour (Label::ColourIds::textColourId, Colours::black);
        laf.setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);
        laf.setColour (Label::ColourIds::backgroundWhenEditingColourId, Colours::white);
        laf.setColour (ListBox::backgroundColourId, Colours::white);
    }




    //==========================================================================
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise (const String& commandLine) override
    {
        configureLookAndFeel();
        menuModel = std::make_unique<MainMenuBarModel>();
        commandManager.registerAllCommandsForTarget (this);
        MenuBarModel::setMacMainMenu (menuModel.get(), nullptr);
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        MenuBarModel::setMacMainMenu (nullptr, nullptr);
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }




    //==========================================================================
    void getAllCommands (Array<CommandID>& commands) override
    {
        JUCEApplication::getAllCommands (commands);

        const CommandID ids[] = {
            CommandIDs::openDocument,
            CommandIDs::windowToggleOpenGL,
        };
        commands.addArray (ids, numElementsInArray (ids));
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case CommandIDs::openDocument:
                result.setInfo ("Open...", "", CommandCategories::general,
                                ApplicationCommandInfo::isDisabled);
                result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
                break;
            case CommandIDs::windowToggleOpenGL:
                result.setInfo ("OpenGL Backend", "", CommandCategories::window,
                                mainWindow && mainWindow->context.isAttached() ? ApplicationCommandInfo::isTicked : 0);
                result.defaultKeypresses.add (KeyPress ('g', ModifierKeys::ctrlModifier, 0));
                break;
            default:
                JUCEApplication::getCommandInfo (commandID, result);
                break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::openDocument:              return true;
            case CommandIDs::windowToggleOpenGL:        mainWindow->toggleOpenGL(); return true;
            default:                                    return JUCEApplication::perform (info);
        }
    }

private:
    TooltipWindow tooltipWindow;
    ApplicationCommandManager commandManager;
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<MainMenuBarModel> menuModel;
};




//==============================================================================
MonocleApplication::MainWindow::MainWindow (String name)
: DocumentWindow (name, Colours::white, DocumentWindow::allButtons)
{
    auto mainContent = new MainComponent;
    addKeyListener (getApp().commandManager.getKeyMappings());
    getApp().commandManager.registerAllCommandsForTarget (mainContent);
    setUsingNativeTitleBar (true);
    setContentOwned (mainContent, true);
    setResizable (true, true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

void MonocleApplication::MainWindow::closeButtonPressed()
{
    getApp().systemRequestedQuit();
}

void MonocleApplication::MainWindow::toggleOpenGL()
{
    if (! context.isAttached())
        context.attachTo (*this);
    else
        context.detach();
}




//==============================================================================
MonocleApplication::MainMenuBarModel::MainMenuBarModel()
{
    setApplicationCommandManagerToWatch (&getApp().commandManager);
}

StringArray MonocleApplication::MainMenuBarModel::getMenuBarNames()
{
    return getApp().getMenuNames();
}

PopupMenu MonocleApplication::MainMenuBarModel::getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName)
{
    return getApp().createMenu (menuName);
}

void MonocleApplication::MainMenuBarModel::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
    // Not sure what this method is for...
}




//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MonocleApplication)
