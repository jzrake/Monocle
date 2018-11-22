#pragma once
#include "JuceHeader.h"




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
        virtual void fileListWantsToApplyFilter (const StringArray& files, const String& name) = 0;
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
    void sendApplyFilter (const String& name);

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
