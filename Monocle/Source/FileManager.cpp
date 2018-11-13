#include "FileManager.hpp"




// ============================================================================
FileManager::FileManager()
{
    startTimer (1000);
}

void FileManager::addListener (Listener* listener)
{
    listeners.add (listener);
}

void FileManager::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void FileManager::addFile (File file)
{
    files.addIfNotAlreadyThere (file);
}

void FileManager::removeFile (File file)
{
    files.removeFirstMatchingValue (file);
}

void FileManager::insertFiles (const StringArray& filenames, int index)
{
    for (const auto& filename : filenames)
        if (! files.contains (filename))
            files.insert (index, filename);
}

void FileManager::removeFiles (const StringArray& filenames)
{
    for (const auto& filename : filenames)
        files.removeFirstMatchingValue (filename);
}

Array<File> FileManager::getFiles() const
{
    return files;
}




// ============================================================================
void FileManager::timerCallback()
{
}

void FileManager::pollForChanges()
{
}
