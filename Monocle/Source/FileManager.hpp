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
        virtual void fileManagerFileDeletedOnDisk (File) = 0;
    };

    // ========================================================================
    FileManager();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void addFile (File);
    void removeFile (File);
    void insertFiles (const StringArray& filenames, int index);
    void removeFiles (const StringArray& filenames);
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
