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

void FileManager::setUniqueKey (const String &filename, const std::string &key)
{
    auto n = statuses.indexOf (File (filename));
    statuses.getReference(n).uniqueKey = key;
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

std::string FileManager::getUniqueKey (const String& filename) const
{
    auto n = statuses.indexOf (File (filename));
    return statuses.getReference(n).uniqueKey;
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
