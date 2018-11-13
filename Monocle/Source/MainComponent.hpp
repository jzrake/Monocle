#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "FileManager.hpp"




//==============================================================================
class SourceListView : public ListBox, public ListBoxModel
{
public:
    //==========================================================================
    SourceListView();
    void setFileList (Array<File> filesToDisplay);

    //==========================================================================
    int getNumRows() override;
    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected) override;

private:
    Array<File> files;
};




//==============================================================================
class MainComponent : public Component, public StateManager
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

    //==========================================================================
    void dispatch (const Action& action) override;

private:
    //==========================================================================
    AppSkeleton skeleton;
    FigureView figure;
    TextEditor notesPage;
    SourceListView sourceList;

    FileManager fileManager;
    FigureModel model;
};
