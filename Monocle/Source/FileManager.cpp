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

void FileManager::setPollingInterval (int millisecondsBetweenPolling)
{
    startTimer (millisecondsBetweenPolling);
}

void FileManager::setFilterName (const String &filename, const String &filter)
{
    auto n = statuses.indexOf ( File (filename));
    statuses.getReference(n).filter = filter;
}

void FileManager::addFile (File file)
{
    statuses.addIfNotAlreadyThere (file);
}

void FileManager::removeFile (File file)
{
    statuses.removeFirstMatchingValue (file);
}

void FileManager::insertFiles (const StringArray& filenames, int index)
{
    for (const auto& filename : filenames)
        if (! statuses.contains (File (filename)))
            statuses.insert (index, File (filename));
}

void FileManager::removeFiles (const StringArray& filenames)
{
    for (const auto& filename : filenames)
        statuses.removeFirstMatchingValue (File (filename));
}

Array<File> FileManager::getFiles() const
{
    Array<File> files;

    for (const auto& status : statuses)
        files.add (status.file);
    return files;
}

StringArray FileManager::getFilterNames (const StringArray& filenames) const
{
    StringArray filters;

    for (const auto& status : statuses)
        if (filenames.contains (status.file.getFullPathName()))
            filters.add (status.filter);
    return filters;
}




// ============================================================================
FileManager::FileStatus::FileStatus()
{
}

FileManager::FileStatus::FileStatus (File file) : file (file)
{
    refreshFromDisk();
}

bool FileManager::FileStatus::operator== (const FileStatus& other) const
{
    return file == other.file;
}

bool FileManager::FileStatus::refreshFromDisk()
{
    auto last = *this;
    existed = file.existsAsFile();
    modified = file.getLastModificationTime();
    return last.existed != existed || last.modified != modified;
}




// ============================================================================
void FileManager::timerCallback()
{
    pollForChanges();
}

void FileManager::pollForChanges()
{
    for (auto& status : statuses)
        if (status.refreshFromDisk())
            listeners.call (&Listener::fileManagerFileChangedOnDisk, status.file);
}

FileManager::FileStatus FileManager::getStatusForFile (File file) const
{
    for (const auto& status : statuses)
        if (status.file == file)
            return status;

    jassertfalse; // That file is not being tracked!
    return FileStatus();
}
