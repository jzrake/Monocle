#include "JuceHeader.h"
#include "MainComponent.hpp"




//==============================================================================
void ActionDispatcher::dispatch (const Component* subject, const Action& action) const
{
    auto store = subject->findParentComponentOfClass<StateManager>();

    if (store == nullptr)
    {
        jassertfalse; // There must be a store attached somewhere above an
                      // ActionDispatcher component!
    }
    store->dispatch (action);
}




//==============================================================================
class MonocleApplication  : public JUCEApplication
{
public:
    //==========================================================================
    MonocleApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise (const String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));

        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::backgroundColourId, Colours::white);
        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::textColourId, Colours::black);
        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::highlightColourId, Colours::lightblue);
        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::highlightedTextColourId, Colours::black);
        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        Desktop::getInstance().getDefaultLookAndFeel().setColour (TextEditor::focusedOutlineColourId, Colours::lightblue);
    }

    void shutdown() override
    {
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
    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (String name) : DocumentWindow (name,
                                                   Colours::black,
                                                   DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent, true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
private:
    TooltipWindow tooltipWindow;
    std::unique_ptr<MainWindow> mainWindow;
};




//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MonocleApplication)
