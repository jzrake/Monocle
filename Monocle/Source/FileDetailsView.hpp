#pragma once
#include "JuceHeader.h"




//==============================================================================
class FileDetailsView : public Component, private TextEditor::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        // virtual void filterNameChanged (const String& newName) = 0;
    };

    struct Geometry
    {
        Rectangle<int> icon;
        Rectangle<int> editor;
        Rectangle<int> fileSize;
        Rectangle<int> modified;
    };

    FileDetailsView();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setCurrentlyActiveFiles (const StringArray& names);
    // void setFilterIsValid (bool isValid);
    void updateFileDetailsIfShowing (File file);

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    void textEditorTextChanged (TextEditor&) override;
    Geometry computeGeometry() const;

    ListenerList<Listener> listeners;
    // TextEditor filterNameEditor;
    // std::unique_ptr<Drawable> filterKnown;
    // std::unique_ptr<Drawable> filterUnknown;

    StringArray currentFilenames;
    bool filterIsCurrentlyValid = false;
};
