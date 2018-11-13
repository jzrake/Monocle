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
    if (! files.contains (file))
    {
        files.add (file);
        listeners.call (&Listener::fileAdded, file);
    }
}

void FileManager::removeFile (File file)
{
    if (files.contains (file))
    {
        files.removeAllInstancesOf (file);
        listeners.call (&Listener::fileRemoved, file);
    }
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
