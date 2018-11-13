#include "MainComponent.hpp"
#include "MaterialIcons.hpp"




//==============================================================================
SourceListView::SourceListView()
{
    setModel (this);
    setOutlineThickness (2);
    setMultipleSelectionEnabled (true);
    setColour (ListBox::ColourIds::outlineColourId, Colours::transparentBlack);

    fileIcon      = material::util::icon (material::editor::ic_insert_drive_file, Colours::seagreen);
    directoryIcon = material::util::icon (material::file::ic_folder, Colours::cyan);
    nonexistIcon  = material::util::icon (material::alert::ic_error_outline, Colours::red);
}

void SourceListView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void SourceListView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void SourceListView::setFileList (const Array<File>& filesToDisplay)
{
    files = filesToDisplay;
    updateContent();

    if (getSelectedRow() == -1)
        selectRow (files.size() - 1);
}

void SourceListView::updateFileDisplayStatus (File file)
{
    repaintRow (files.indexOf (file));
}

//==============================================================================
void SourceListView::focusGained (FocusChangeType)
{
    repaint();
}

void SourceListView::focusLost (FocusChangeType)
{
    repaint();
}

//==============================================================================
int SourceListView::getNumRows()
{
    return files.size();
}

void SourceListView::paintListBoxItem (int row, Graphics& g, int w, int h, bool selected)
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

void SourceListView::listBoxItemClicked (int row, const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, String ("Remove file") + (getNumSelectedRows() > 1 ? "s" : ""));
        menu.showMenuAsync(PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: sendDeleteSelectedFiles(); break;
                default: break;
            }
        });
    }
}

void SourceListView::listBoxItemDoubleClicked (int row, const MouseEvent&)
{
    startSelectedFilesAsProcess();
}

void SourceListView::backgroundClicked (const MouseEvent&)
{
    deselectAllRows();
}

void SourceListView::selectedRowsChanged (int lastRowSelected)
{
}

void SourceListView::deleteKeyPressed (int)
{
    sendDeleteSelectedFiles();
}

void SourceListView::returnKeyPressed (int)
{
    startSelectedFilesAsProcess();
}

void SourceListView::listWasScrolled()
{
}

String SourceListView::getTooltipForRow (int row)
{
    return files[row].getFullPathName();
}

//==============================================================================
bool SourceListView::isInterestedInFileDrag (const StringArray& files)
{
    return true;
}
void SourceListView::fileDragEnter (const StringArray& files, int x, int y)
{
    setColour (ListBox::ColourIds::outlineColourId, Colours::lightblue);
}
void SourceListView::fileDragMove (const StringArray& files, int x, int y)
{
}
void SourceListView::fileDragExit (const StringArray& files)
{
    setColour (ListBox::ColourIds::outlineColourId, Colours::transparentBlack);
}
void SourceListView::filesDropped (const StringArray& files, int x, int y)
{
    setColour (ListBox::ColourIds::outlineColourId, Colours::transparentBlack);
    listeners.call (&Listener::sourceListFilesInserted, files, getInsertionIndexForPosition (x, y));
}

//==============================================================================
void SourceListView::startSelectedFilesAsProcess()
{
    for (int n = 0; n < files.size(); ++n)
        if (isRowSelected (n))
            files[n].startAsProcess();
}

void SourceListView::sendDeleteSelectedFiles()
{
    StringArray filesToDelete;
    auto newSelection = getSelectedRows().getRange(0).getStart();

    for (int n = 0; n < files.size(); ++n)
        if (isRowSelected (n))
            filesToDelete.add (files[n].getFullPathName());

    selectRow (newSelection);
    listeners.call (&Listener::sourceListFilesRemoved, filesToDelete);
}




//==============================================================================
MainComponent::MainComponent()
{
    skeleton.addNavButton ("Sources", material::bintos (material::action::ic_list));
    skeleton.addNavButton ("Files", material::bintos (material::file::ic_folder_open));
    skeleton.addNavButton ("Notes", material::bintos (material::action::ic_speaker_notes));
    skeleton.addNavButton ("Settings", material::bintos (material::action::ic_settings));
    skeleton.setMainContent (figure);

    skeleton.setNavPage ("Notes", notesPage);
    skeleton.setNavPage ("Files", sourceList);

    notesPage.setMultiLine (true);
    notesPage.setReturnKeyStartsNewLine (true);
    notesPage.setBorder ({0, 0, 0, 0});
    notesPage.setTextToShowWhenEmpty ("Notes", Colours::lightgrey);
    notesPage.setFont (Font ("Optima", 14, 0));

    LinePlotModel linePlot;

    for (int n = 0; n < 200; ++n)
    {
        double t = 2 * M_PI * n / 200.0;
        linePlot.x.add (cos (t));
        linePlot.y.add (sin (t));
    }
    linePlot.lineWidth = 4;
    model.linePlots.add (linePlot);
    figure.setModel (model);

    fileManager.setPollingInterval (100);
    fileManager.addListener (this);
    sourceList.addListener (this);

    addAndMakeVisible (skeleton);
    setSize (800, 600);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    skeleton.setBounds (getLocalBounds());
}

void MainComponent::dispatch (const Action& action)
{
    if (action.name == set_figure_margin)
    {
        model.margin = ModelHelpers::borderSizeFromVar (action.value);
        figure.setModel (model);
    }
    else if (action.name == set_figure_domain)
    {
        auto domain = ModelHelpers::rectangleFromVar (action.value);
        model.xmin = domain.getX();
        model.xmax = domain.getRight();
        model.ymin = domain.getY();
        model.ymax = domain.getBottom();
        figure.setModel (model);
    }
    else if (action.name == set_figure_xlabel)
    {
        model.xlabel = action.value;
        figure.setModel (model);
    }
    else if (action.name == set_figure_ylabel)
    {
        model.ylabel = action.value;
        figure.setModel (model);
    }
    else if (action.name == set_figure_title)
    {
        model.title = action.value;
        figure.setModel (model);
    }
}

//==========================================================================
void MainComponent::sourceListFilesInserted (const StringArray& files, int index)
{
    fileManager.insertFiles (files, index);
    sourceList.setFileList (fileManager.getFiles());
}
void MainComponent::sourceListFilesRemoved (const StringArray& files)
{
    fileManager.removeFiles (files);
    sourceList.setFileList (fileManager.getFiles());
}

//==========================================================================
void MainComponent::fileManagerFileChangedOnDisk (File file)
{
    sourceList.updateFileDisplayStatus (file);
}
