#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "FileManager.hpp"




//==============================================================================
class SourceListView : public ListBox, public ListBoxModel, public FileDragAndDropTarget
{
public:

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void sourceListFilesInserted (const StringArray& files, int index) = 0;
        virtual void sourceListFilesRemoved (const StringArray& files) = 0;
    };

    //==========================================================================
    SourceListView();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setFileList (const Array<File>& filesToDisplay);

    //==========================================================================
    int getNumRows() override;
    void paintListBoxItem (int, Graphics&, int, int, bool) override;
    void listBoxItemClicked (int row, const MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const MouseEvent&) override;
    void backgroundClicked (const MouseEvent&) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void deleteKeyPressed (int lastRowSelected) override;
    void returnKeyPressed (int lastRowSelected) override;
    void listWasScrolled() override;
    String getTooltipForRow (int row) override;

    //==========================================================================
    bool isInterestedInFileDrag (const StringArray& files) override;
    void fileDragEnter (const StringArray& files, int x, int y) override;
    void fileDragMove (const StringArray& files, int x, int y) override;
    void fileDragExit (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;

private:
    void startSelectedFilesAsProcess();
    void sendDeleteSelectedFiles();

    Array<File> files;
    ListenerList<Listener> listeners;
};




//==============================================================================
class MainComponent
: public Component
, public StateManager
, private SourceListView::Listener
, private FileManager::Listener
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent();
    void paint (Graphics&) override;
    void resized() override;

    //==========================================================================
    void dispatch (const Action& action) override;

private:
    //==========================================================================
    void sourceListFilesInserted (const StringArray& files, int index) override;
    void sourceListFilesRemoved (const StringArray& files) override;

    //==========================================================================
    void fileManagerFileChangedOnDisk (File) override;
    void fileManagerFileDeletedOnDisk (File) override;

    //==========================================================================
    AppSkeleton skeleton;
    FigureView figure;
    TextEditor notesPage;
    SourceListView sourceList;

    FileManager fileManager;
    FigureModel model;
};
