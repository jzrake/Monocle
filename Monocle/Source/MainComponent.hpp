#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "FileManager.hpp"
#include "Database.hpp"
#include "FileListView.hpp"
#include "FileDetailsView.hpp"
#include "SymbolListView.hpp"
#include "SymbolDetailsView.hpp"
#include "Kernel/AcyclicGraph.hpp"




//==============================================================================
class MainComponent
: public Component
, public ApplicationCommandTarget
, private FileManager::Listener
, private FileListView::Listener
, private SymbolListView::Listener
, private SymbolDetailsView::Listener
, private FileDetailsView::Listener
, private FigureView::Listener
{
public:
    //==========================================================================
    MainComponent();
    ~MainComponent();
    void paint (Graphics&) override;
    void resized() override;

    //==========================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

private:
    //==========================================================================
    void fileManagerFileChangedOnDisk (File) override;

    //==========================================================================
    void fileListFilesInserted (const StringArray& files, int index) override;
    void fileListFilesRemoved (const StringArray& files) override;
    void fileListSelectionChanged (const StringArray& files) override;

    //==========================================================================
    void symbolListSelectionChanged (const StringArray& symbols) override;
    void symbolListSymbolsRemoved (const StringArray& symbols) override;

    //==========================================================================
    void symbolDetailsWantsNewDefinition (int code, const StringArray& argumentKeys) override;

    //==========================================================================
    // void filterNameChanged (const String& newName) override;

    //==========================================================================
    void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) override;
    void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) override;
    void figureViewSetXlabel (FigureView* figure, const String& value) override;
    void figureViewSetYlabel (FigureView* figure, const String& value) override;
    void figureViewSetTitle (FigureView* figure, const String& value) override;

    //==========================================================================
    AppSkeleton       skeleton;
    FigureView        figure;
    TextEditor        notesPage;
    FileListView      fileList;
    FileDetailsView   fileDetails;
    DualComponentView fileListAndDetail;
    SymbolListView    symbolList;
    SymbolDetailsView symbolDetails;
    DualComponentView symbolListAndDetail;

    //==========================================================================
    FileManager fileManager;
    FigureModel model;
    mcl::AcyclicGraph kernel;
};
