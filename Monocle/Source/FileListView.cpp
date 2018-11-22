#include "FileListView.hpp"
#include "MaterialIcons.hpp"




//==============================================================================
void FileListView::DropOutline::paint (Graphics& g)
{
    g.setColour (Colours::lightblue);
    g.drawRect (getLocalBounds(), 4);
}




//==============================================================================
FileListView::FileListView()
{
    setModel (this);
    setMultipleSelectionEnabled (true);
    setOutlineThickness (0);
    setColour (ListBox::ColourIds::outlineColourId, Colours::transparentBlack);
    addChildComponent (outline);
    outline.setInterceptsMouseClicks (false, false);

    fileIcon      = material::util::icon (material::editor::ic_insert_drive_file, Colours::seagreen);
    directoryIcon = material::util::icon (material::file::ic_folder, Colours::skyblue);
    nonexistIcon  = material::util::icon (material::alert::ic_error_outline, Colours::red);
}

void FileListView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void FileListView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void FileListView::setFileList (const Array<File>& filesToDisplay)
{
    files = filesToDisplay;
    updateContent();

    if (getSelectedRow() == -1)
        selectRow (files.size() - 1);
}

void FileListView::updateFileDisplayStatus (File file)
{
    repaintRow (files.indexOf (file));
}

Array<File> FileListView::getSelectedFiles() const
{
    Array<File> selectedFiles;

    for (int n = 0; n < files.size(); ++n)
        if (isRowSelected (n))
            selectedFiles.add (files[n]);
    return selectedFiles;
}

StringArray FileListView::getSelectedFullPathNames() const
{
    StringArray selectedFiles;

    for (int n = 0; n < files.size(); ++n)
        if (isRowSelected (n))
            selectedFiles.add (files[n].getFullPathName());
    return selectedFiles;
}

//==============================================================================
void FileListView::focusGained (FocusChangeType)
{
    repaint();
}

void FileListView::focusLost (FocusChangeType)
{
    repaint();
}

//==============================================================================
int FileListView::getNumRows()
{
    return files.size();
}

void FileListView::paintListBoxItem (int row, Graphics& g, int w, int h, bool selected)
{
    if (selected)
    {
        g.setColour (hasKeyboardFocus (false) ? Colours::lightblue : Colours::lightgrey);
        g.fillRect (0, 0, w, h);
    }

    auto iconArea = Rectangle<float> (0, 0, h, h);
    auto iconRect = iconArea.withSizeKeepingCentre (10, 10);

    if (files[row].existsAsFile())
    {
        fileIcon->drawWithin (g, iconRect, RectanglePlacement::fillDestination, 1.f);
    }
    else if (files[row].exists())
    {
        directoryIcon->drawWithin (g, iconRect, RectanglePlacement::fillDestination, 1.f);
    }
    else
    {
        nonexistIcon->drawWithin (g, iconRect, RectanglePlacement::fillDestination, 1.f);
    }

    g.setColour (files[row].exists() ? Colours::black : Colours::red);
    g.drawText (files[row].getFileName(), h, 0, w, h, Justification::centredLeft);
}

void FileListView::listBoxItemClicked (int row, const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, String ("Remove file") + (getNumSelectedRows() > 1 ? "s" : ""));
        menu.addItem (2, "Load as Ascii Table");
        menu.showMenuAsync(PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: sendDeleteSelectedFiles(); break;
                case 2: sendApplyFilter ("load-ascii"); break;
                default: break;
            }
        });
    }
}

void FileListView::listBoxItemDoubleClicked (int row, const MouseEvent&)
{
    startSelectedFilesAsProcess();
}

void FileListView::backgroundClicked (const MouseEvent&)
{
    deselectAllRows();
}

void FileListView::selectedRowsChanged (int)
{
    listeners.call (&Listener::fileListSelectionChanged, getSelectedFullPathNames());
}

void FileListView::deleteKeyPressed (int)
{
    sendDeleteSelectedFiles();
}

void FileListView::returnKeyPressed (int)
{
    startSelectedFilesAsProcess();
}

void FileListView::listWasScrolled()
{
}

String FileListView::getTooltipForRow (int row)
{
    return files[row].getFullPathName();
}

//==============================================================================
bool FileListView::isInterestedInFileDrag (const StringArray& files)
{
    return true;
}

void FileListView::fileDragEnter (const StringArray& files, int x, int y)
{
    outline.setBounds (getLocalBounds());
    outline.setVisible (true);
}

void FileListView::fileDragMove (const StringArray& files, int x, int y)
{
}

void FileListView::fileDragExit (const StringArray& files)
{
    outline.setVisible (false);
}

void FileListView::filesDropped (const StringArray& files, int x, int y)
{
    outline.setVisible (false);
    listeners.call (&Listener::fileListFilesInserted, files, getInsertionIndexForPosition (x, y));
}

//==============================================================================
void FileListView::startSelectedFilesAsProcess()
{
    for (const auto& file : getSelectedFiles())
        file.startAsProcess();
}

void FileListView::sendDeleteSelectedFiles()
{
    auto newSelection = getSelectedRows().getRange(0).getStart();
    auto filesToDelete = getSelectedFullPathNames();
    selectRow (newSelection);
    listeners.call (&Listener::fileListFilesRemoved, filesToDelete);
}

void FileListView::sendApplyFilter (const String& name)
{
    listeners.call (&Listener::fileListWantsToApplyFilter, getSelectedFullPathNames(), name);
}
