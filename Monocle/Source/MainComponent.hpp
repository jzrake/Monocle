#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "FileManager.hpp"
#include "Database.hpp"
#include "SymbolListView.hpp"
#include "SymbolDetailsView.hpp"
#include "Kernel/AcyclicGraph.hpp"




//==============================================================================
class FileListView
: public ListBox
, public ListBoxModel
, public FileDragAndDropTarget
{
public:

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void fileListFilesInserted (const StringArray& files, int index) = 0;
        virtual void fileListFilesRemoved (const StringArray& files) = 0;
        virtual void fileListSelectionChanged (const StringArray& files) = 0;
    };

    //==========================================================================
    FileListView();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setFileList (const Array<File>& filesToDisplay);
    void updateFileDisplayStatus (File file);
    Array<File> getSelectedFiles() const;
    StringArray getSelectedFullPathNames() const;

    //==========================================================================
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;

    //==========================================================================
    int getNumRows() override;
    void paintListBoxItem (int, Graphics&, int, int, bool) override;
    void listBoxItemClicked (int, const MouseEvent&) override;
    void listBoxItemDoubleClicked (int, const MouseEvent&) override;
    void backgroundClicked (const MouseEvent&) override;
    void selectedRowsChanged (int) override;
    void deleteKeyPressed (int) override;
    void returnKeyPressed (int) override;
    void listWasScrolled() override;
    String getTooltipForRow (int) override;

    //==========================================================================
    bool isInterestedInFileDrag (const StringArray& files) override;
    void fileDragEnter (const StringArray& files, int x, int y) override;
    void fileDragMove (const StringArray& files, int x, int y) override;
    void fileDragExit (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;

private:
    void startSelectedFilesAsProcess();
    void sendDeleteSelectedFiles();

    class DropOutline : public Component
    {
    public:
        void paint (Graphics&) override;
    };

    Array<File> files;
    ListenerList<Listener> listeners;
    std::unique_ptr<Drawable> fileIcon;
    std::unique_ptr<Drawable> directoryIcon;
    std::unique_ptr<Drawable> nonexistIcon;
    DropOutline outline;
};




//==============================================================================
class FileDetailsView : public Component, private TextEditor::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void filterNameChanged (const String& newName) = 0;
    };

    struct Geometry
    {
        Rectangle<int> icon;
        Rectangle<int> editor;
        Rectangle<int> fileSize;
        Rectangle<int> modified;
    };

    FileDetailsView();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setCurrentlyActiveFiles (const StringArray& names, const StringArray& filters);
    void setFilterIsValid (bool isValid);
    void updateFileDetailsIfShowing (File file);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    void textEditorTextChanged (TextEditor&) override;
    Geometry computeGeometry() const;

    ListenerList<Listener> listeners;
    TextEditor filterNameEditor;
    std::unique_ptr<Drawable> filterKnown;
    std::unique_ptr<Drawable> filterUnknown;

    StringArray currentFilenames;
    bool filterIsCurrentlyValid = false;
};




//==============================================================================
class MainComponent
: public Component
, public ApplicationCommandTarget
, private FileManager::Listener
, private FileListView::Listener
, private SymbolListView::Listener
, private FileDetailsView::Listener
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
    void kernelListSelectionChanged (const StringArray& symbols) override;
    void kernelListSymbolsRemoved (const StringArray& symbols) override;

    //==========================================================================
    void filterNameChanged (const String& newName) override;

    //==========================================================================
    void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) override;
    void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) override;
    void figureViewSetXlabel (FigureView* figure, const String& value) override;
    void figureViewSetYlabel (FigureView* figure, const String& value) override;
    void figureViewSetTitle (FigureView* figure, const String& value) override;

    static StringArray stringArrayFromStdStringVector (const std::vector<std::string>&);

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

    //==========================================================================
    FileManager fileManager;
    FigureModel model;
    mcl::AcyclicGraph kernel;
};
