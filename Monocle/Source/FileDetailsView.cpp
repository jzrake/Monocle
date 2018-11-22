#include "FileDetailsView.hpp"




//==============================================================================
FileDetailsView::FileDetailsView()
{
}

void FileDetailsView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void FileDetailsView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void FileDetailsView::setCurrentlyActiveFiles (const StringArray& filenames)
{
    currentFilenames = filenames;
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
    for (const auto& filename : currentFilenames) size += File (filename).getSize();

    auto modifiedString = File (currentFilenames[0]).getLastModificationTime().toString (true, true);
    auto sizeString = String (size / 1024) + "kB";
    auto geom = computeGeometry();

    g.fillAll (Colours::whitesmoke);

    g.setFont (Font ("Monaco", 10, 0));
    g.setColour (Colours::black);

    if (currentFilenames.size() > 0)
        g.drawText ("size: " + sizeString, geom.fileSize, Justification::bottomLeft);
    if (currentFilenames.size() == 1)
        g.drawText ("modified: " + modifiedString, geom.modified, Justification::bottomLeft);
}

void FileDetailsView::resized()
{
}

FileDetailsView::Geometry FileDetailsView::computeGeometry() const
{
    auto rowHeight = 22;
    auto area = getLocalBounds().withTrimmedTop (rowHeight).withTrimmedRight(8);
    auto col1 = area.removeFromLeft (rowHeight);
    auto col2 = area;

    Geometry g;
    g.icon     = col1.removeFromTop (rowHeight).reduced(6);
    g.fileSize = col2.removeFromTop (rowHeight);
    g.modified = col2.removeFromTop (rowHeight);
    return g;
}
