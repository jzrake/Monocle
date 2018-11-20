#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "FileManager.hpp"
#include "Database.hpp"
#include "FileListView.hpp"
#include "FileDetailsView.hpp"
#include "SymbolListView.hpp"
#include "SymbolDetailsView.hpp"
#include "Kernel/AcyclicGraph.hpp"




//==============================================================================
class DefinitionEditor : public Component, public TextEditor::Listener
{
public:
    using Validator = std::function<std::string(const std::string& key, const std::string& expression)>;

    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void definitionEditorCommited (const std::string& key, const std::string& expression) = 0;
        virtual void definitionEditorCanceled() = 0;
    };

    struct Geometry
    {
        Rectangle<int> cancelIconArea;
        Rectangle<int> commitIconArea;
        Rectangle<int> editorArea;
        Rectangle<int> equalsSignArea;
        Rectangle<int> listArea;
        Rectangle<int> validationMessageArea;
    };

    //==========================================================================
    DefinitionEditor();
    void setValidator (Validator validatorToUse);
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void addPart (const String& part);
    void cancel();
    void commit();
    bool isCommittable() const;
    std::string getKey() const;
    std::string getExpression() const;

    //==========================================================================
    void resized() override;
    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent&) override;

    //==========================================================================
    void textEditorTextChanged (TextEditor&) override;
    void textEditorReturnKeyPressed (TextEditor&) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;

private:
    Geometry computeGeometry() const;
    Validator validator;
    ListenerList<Listener> listeners;
    Font font;
    StringArray parts;
    TextEditor symbolNameEditor;
    std::unique_ptr<Drawable> cancelIcon;
    std::unique_ptr<Drawable> commitIcon;
};




//==============================================================================
class MainComponent
: public Component
, public ApplicationCommandTarget
, private FileManager::Listener
, private FileListView::Listener
, private SymbolListView::Listener
, private SymbolDetailsView::Listener
, private FileDetailsView::Listener
, private DefinitionEditor::Listener
, private FigureView::Listener
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent();
    void paint (Graphics&) override;
    void resized() override;

    //==========================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

private:
    //==========================================================================
    void fileManagerFileChangedOnDisk (File) override;

    //==========================================================================
    void fileListFilesInserted (const StringArray& files, int index) override;
    void fileListFilesRemoved (const StringArray& files) override;
    void fileListSelectionChanged (const StringArray& files) override;

    //==========================================================================
    void symbolListSelectionChanged (const StringArray& symbols) override;
    void symbolListSymbolsRemoved (const StringArray& symbols) override;
    void symbolListSymbolPunched (const String& symbol) override;

    //==========================================================================
    void symbolDetailsItemPunched (const std::string& expression) override;

    //==========================================================================
    void definitionEditorCommited (const std::string& key, const std::string& expression) override;
    void definitionEditorCanceled() override;

    //==========================================================================
    void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) override;
    void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) override;
    void figureViewSetXlabel (FigureView* figure, const String& value) override;
    void figureViewSetYlabel (FigureView* figure, const String& value) override;
    void figureViewSetTitle (FigureView* figure, const String& value) override;

    //==========================================================================
    AppSkeleton       skeleton;
    FigureView        figure;
    TextEditor        notesPage;
    FileListView      fileList;
    FileDetailsView   fileDetails;
    DualComponentView fileListAndDetail;
    SymbolListView    symbolList;
    SymbolDetailsView symbolDetails;
    DualComponentView symbolListAndDetail;
    DefinitionEditor  definitionEditor;

    //==========================================================================
    FileManager fileManager;
    FigureModel model;
    mcl::AcyclicGraph kernel;
};
