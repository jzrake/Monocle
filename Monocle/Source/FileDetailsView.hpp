#pragma once
#include "JuceHeader.h"




//==============================================================================
class FileDetailsView : public Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
    };

    struct Geometry
    {
        Rectangle<int> icon;
        Rectangle<int> fileSize;
        Rectangle<int> modified;
    };

    FileDetailsView();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setCurrentlyActiveFiles (const StringArray& names);
    void updateFileDetailsIfShowing (File file);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    Geometry computeGeometry() const;

    ListenerList<Listener> listeners;
    StringArray currentFilenames;
    bool filterIsCurrentlyValid = false;
};
