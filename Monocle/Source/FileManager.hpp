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
        virtual void fileAdded (File) = 0;
        virtual void fileRemoved (File) = 0;
        virtual void fileChangedOnDisk (File) = 0;
        virtual void fileDeletedOnDisk (File) = 0;
    };

    // ========================================================================
    FileManager();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void addFile (File);
    void removeFile (File);
    Array<File> getFiles() const;

private:
    // ========================================================================
//    struct FileInfo
//    {
//        File file;
//        Time lastModified;
//    };

    void timerCallback() override;
    void pollForChanges();
    Array<File> files;
    ListenerList<Listener> listeners;
};
