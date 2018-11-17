#include "MainComponent.hpp"
#include "MaterialIcons.hpp"
#include "Kernel/Expression.hpp" // Need ???




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




//==============================================================================
FileDetailsView::FileDetailsView()
{
    filterKnown   = material::util::icon (material::navigation::ic_check, Colours::green);
    filterUnknown = material::util::icon (material::navigation::ic_close, Colours::red);

    filterNameEditor.setMultiLine (false);
    filterNameEditor.setReturnKeyStartsNewLine (false);
    filterNameEditor.setBorder ({0, 0, 0, 0});
    filterNameEditor.setTextToShowWhenEmpty ("Filter name", Colours::lightgrey);
    filterNameEditor.setFont (Font ("Monaco", 14, 0));
    filterNameEditor.addListener (this);
    filterNameEditor.setVisible (false);
    filterNameEditor.setColour (TextEditor::ColourIds::outlineColourId, Colours::lightgrey);

    addChildComponent (filterNameEditor);
}

void FileDetailsView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void FileDetailsView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void FileDetailsView::setCurrentlyActiveFiles (const StringArray& filenames, const StringArray& filters)
{
    jassert (filenames.size() == filters.size());

    if (filenames.size() == 1)
    {
        filterNameEditor.setText (filters[0]);
        filterNameEditor.setVisible (true);
    }
    else
    {
        filterNameEditor.clear();
        filterNameEditor.setVisible (false);
    }

    currentFilenames = filenames;
    repaint();
}

void FileDetailsView::setFilterIsValid (bool isValid)
{
    filterIsCurrentlyValid = isValid;
    repaint();
}

void FileDetailsView::updateFileDetailsIfShowing (File file)
{
    if (currentFilenames.contains (file.getFullPathName()))
        repaint();
}

void FileDetailsView::paint (Graphics& g)
{
    if (currentFilenames.isEmpty())
        return;

    int64 size = 0;

    for (const auto& filename : currentFilenames)
        size += File (filename).getSize();
    auto modifiedString = File (currentFilenames[0]).getLastModificationTime().toString (true, true);
    auto sizeString = String (size / 1024) + "kB";
    auto geom = computeGeometry();

    g.fillAll (Colours::whitesmoke);

    if (! filterNameEditor.isEmpty())
    {
        if (filterIsCurrentlyValid)
            filterKnown->drawWithin (g, geom.icon.toFloat(), RectanglePlacement::fillDestination, 1.f);
        else
            filterUnknown->drawWithin (g, geom.icon.toFloat(), RectanglePlacement::fillDestination, 1.f);
    }

    g.setFont (Font ("Monaco", 10, 0));
    g.setColour (Colours::black);

    if (currentFilenames.size() > 0)
        g.drawText ("size: " + sizeString, geom.fileSize, Justification::bottomLeft);
    if (currentFilenames.size() == 1)
        g.drawText ("modified: " + modifiedString, geom.modified, Justification::bottomLeft);
}

void FileDetailsView::resized()
{
    filterNameEditor.setBounds (computeGeometry().editor);
}

void FileDetailsView::textEditorTextChanged (TextEditor&)
{
    listeners.call (&Listener::filterNameChanged, filterNameEditor.getText());
}

FileDetailsView::Geometry FileDetailsView::computeGeometry() const
{
    auto rowHeight = 22;
    auto area = getLocalBounds().withTrimmedTop (rowHeight).withTrimmedRight(8);
    auto col1 = area.removeFromLeft (rowHeight);
    auto col2 = area;

    Geometry g;
    g.icon     = col1.removeFromTop (rowHeight).reduced(6);
    g.editor   = col2.removeFromTop (rowHeight);
    g.fileSize = col2.removeFromTop (rowHeight);
    g.modified = col2.removeFromTop (rowHeight);
    return g;
}




//==============================================================================
#include <fstream>
#include "AsciiLoader.hpp"

static mcl::Object load_ascii (const mcl::Object::List& args, const mcl::Object::Dict&)
{
    File f (args.at(0).get<std::string>());

    std::fstream input (f.getFullPathName().toStdString());
    mcl::AsciiLoader loader (input);

    if (loader.getStatusMessage().isNotEmpty())
        throw std::runtime_error (loader.getStatusMessage().toStdString());

    return f.getLastModificationTime().toString (false, true).toStdString();
}

static mcl::Object list (const mcl::Object::List& args, const mcl::Object::Dict&)
{
    return args;
}

static mcl::Object dict (const mcl::Object::List&, const mcl::Object::Dict& kwar)
{
    return kwar;
}




//==============================================================================
MainComponent::MainComponent()
{
    skeleton.addNavButton ("Symbols",  material::bintos (material::action::ic_list));
    skeleton.addNavButton ("Files",    material::bintos (material::file::ic_folder_open));
    skeleton.addNavButton ("Notes",    material::bintos (material::action::ic_speaker_notes));
    skeleton.addNavButton ("Settings", material::bintos (material::action::ic_settings));
    skeleton.setMainContent (figure);
    skeleton.setNavPage ("Notes", notesPage);
    skeleton.setNavPage ("Files", fileListAndDetail);
    skeleton.setNavPage ("Symbols", symbolListAndDetail);

    notesPage.setMultiLine (true);
    notesPage.setReturnKeyStartsNewLine (true);
    notesPage.setBorder ({0, 0, 0, 0});
    notesPage.setTextToShowWhenEmpty ("Notes", Colours::lightgrey);
    notesPage.setFont (Font ("Optima", 14, 0));

    fileListAndDetail.setContent1 (fileList);
    fileListAndDetail.setContent2 (fileDetails);

    symbolListAndDetail.setContent1 (symbolList);
    symbolListAndDetail.setContent2 (symbolDetails);

    fileManager.setPollingInterval (100);
    figure     .setModel (model = FigureModel::createExample());

    fileManager.addListener (this);
    fileList   .addListener (this);
    symbolList .addListener (this);
    fileDetails.addListener (this);
    figure     .addListener (this);

    addAndMakeVisible (skeleton);
    setSize (800, 600);

    // Initial kernel configuration
    // ========================================================================
    kernel.setListener ([this] (const std::string& key, const mcl::Object& val)
    {
        auto status = kernel.status (key);
        symbolList.updateSymbolStatus (key, status);
    });

    kernel.setErrorLog ([this] (const std::string& key, const std::string& msg) { DBG("error: " << key << " " << msg); });
    kernel.insert ("load-ascii", mcl::Object::Func (load_ascii));
    kernel.insert ("list", mcl::Object::Func (list));
    kernel.insert ("dict", mcl::Object::Func (dict));
    kernel.insert ("test-data", mcl::Object::expr ("(load-ascii test)"));
    kernel.insert ("test-object", mcl::Object::expr ("(list (dict a=123 b=543) 2 3 (list 5 6 7 (list 6 7 8 9 10 2 load-ascii 4 3 2 1 3 4 4)))"));

    symbolList.setSymbolList (kernel.status (kernel.select()));
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

//==========================================================================
ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return nullptr;
}

void MainComponent::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::windowToggleBackdrop,
        CommandIDs::windowToggleNavPages,
    };
    commands.addArray (ids, numElementsInArray (ids));
}

void MainComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::windowToggleNavPages:
            result.setInfo ("Toggle Navigation Pages", "", CommandCategories::window, 0);
            result.defaultKeypresses.add (KeyPress ('K', ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::windowToggleBackdrop:
            result.setInfo ("Toggle Backdrop", "", CommandCategories::window, 0);
            result.defaultKeypresses.add (KeyPress ('L', ModifierKeys::commandModifier, 0));
            break;
        default: break;
    }
}

bool MainComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::windowToggleNavPages: skeleton.toggleNavPagesRevealed(); return true;
        case CommandIDs::windowToggleBackdrop: skeleton.toggleBackdropRevealed(); return true;
        default: return false;
    }
}

//==========================================================================
void MainComponent::fileManagerFileChangedOnDisk (File file)
{
    fileList.updateFileDisplayStatus (file);
    fileDetails.updateFileDetailsIfShowing (file);
    kernel.touch (File (file).getFileNameWithoutExtension().toStdString());
}

//==========================================================================
void MainComponent::fileListFilesInserted (const StringArray& files, int index)
{
    fileManager.insertFiles (files, index);
    fileList.setFileList (fileManager.getFiles());

    for (const auto& file : files)
        kernel.insert (File (file).getFileNameWithoutExtension().toStdString(), file.toStdString());
}

void MainComponent::fileListFilesRemoved (const StringArray& files)
{
    for (const auto& file : files)
        kernel.remove (File (file).getFileNameWithoutExtension().toStdString());

    fileManager.removeFiles (files);
    fileList.setFileList (fileManager.getFiles());
}

void MainComponent::fileListSelectionChanged (const StringArray& files)
{
    fileDetails.setCurrentlyActiveFiles (files, fileManager.getFilterNames (files));
}

//==========================================================================
void MainComponent::kernelListSelectionChanged (const StringArray& symbols)
{
    if (symbols.size() == 1)
        symbolDetails.setViewedObject (symbols[0].toStdString(), kernel.concrete (symbols[0].toStdString()));
    else
        symbolDetails.setViewedObject ("", mcl::Object());
}

void MainComponent::kernelListSymbolsRemoved (const StringArray& symbols)
{
    for (const auto& key : symbols)
        kernel.remove (key.toStdString());
}

//==========================================================================
void MainComponent::filterNameChanged (const String& newName)
{
    for (const auto& file : fileList.getSelectedFullPathNames())
    {
        fileManager.setFilterName (file, newName);
        fileDetails.setFilterIsValid (newName == "Ascii");
    }
}

//==========================================================================
void MainComponent::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
{
    model.margin = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    model.xmin = domain.getX();
    model.xmax = domain.getRight();
    model.ymin = domain.getY();
    model.ymax = domain.getBottom();
    figure->setModel (model);
}

void MainComponent::figureViewSetXlabel (FigureView* figure, const String& value)
{
    model.xlabel = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetYlabel (FigureView* figure, const String& value)
{
    model.ylabel = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetTitle (FigureView* figure, const String& value)
{
    model.title = value;
    figure->setModel (model);
}

//==========================================================================
StringArray MainComponent::stringArrayFromStdStringVector (const std::vector<std::string>& strings)
{
    StringArray res;
    res.ensureStorageAllocated (int (strings.size()));

    for (const auto& s : strings)
        res.add (s);
    return res;
}
