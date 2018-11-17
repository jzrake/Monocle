#include "FileDetailsView.hpp"
#include "MaterialIcons.hpp"




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
