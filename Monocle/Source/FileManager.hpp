#pragma once
#include "JuceHeader.h"




// ============================================================================
class FileManager : private Timer
{
public:
    // ========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void fileManagerFileChangedOnDisk (File) = 0;
    };

    // ========================================================================
    FileManager();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void setPollingInterval (int millisecondsBetweenPolling);
    void addFile (File);
    void removeFile (File);
    void insertFiles (const StringArray& filenames, int index);
    void removeFiles (const StringArray& filenames);
    Array<File> getFiles() const;

private:
    // ========================================================================
    struct FileStatus
    {
        FileStatus();
        FileStatus (File file);
        bool operator== (const FileStatus& other) const;
        bool refreshFromDisk(); /**< Updates the statys and returns true if there was a change. */
        File file;
        Time modified;
        bool existed = false;
        
    };

    void timerCallback() override;
    void pollForChanges();
    FileStatus getStatusForFile (File file) const;

    Array<FileStatus> statuses;
    ListenerList<Listener> listeners;
};
