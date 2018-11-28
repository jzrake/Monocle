#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "FigureView.hpp"
#include "AppSkeleton.hpp"
#include "ExpressionEditor.hpp"
#include "KernelEditor.hpp"




//==============================================================================
class MainComponent
: public Component
, public ApplicationCommandTarget
, public Timer
, public FileDragAndDropTarget
, public DragAndDropContainer
, private FigureView::Listener
, private ExpressionEditor::Listener
, private KernelEditor::Listener
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

    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    bool isInterestedInFileDrag (const StringArray& files) override;
    void fileDragEnter (const StringArray& files, int x, int y) override;
    void fileDragMove (const StringArray& files, int x, int y) override;
    void fileDragExit (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;

private:
    //==========================================================================
    void updateKernel (const Kernel::set_t& dirty);
    void createNewRule (const std::string& key, const crt::expression& expr);

    //==========================================================================
    void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) override;
    void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) override;
    void figureViewSetXlabel (FigureView* figure, const String& value) override;
    void figureViewSetYlabel (FigureView* figure, const String& value) override;
    void figureViewSetTitle (FigureView* figure, const String& value) override;

    //==========================================================================
    void expressionEditorNewExpression (const crt::expression&) override;
    void expressionEditorEncounteredError (const std::string&) override;

    //==========================================================================
    void kernelEditorSelectionChanged() override;
    void kernelEditorRulePunched (const std::string& key) override;
    void kernelEditorWantsNewRule (const crt::expression&) override;
    void kernelEditorWantsRuleRemoved (const std::string&) override;
    void kernelEditorWantsRuleRelabeled (const std::string& from, const std::string& to) override;
    void kernelEditorEncounteredError (const std::string&) override;

    //==========================================================================
    AppSkeleton       skeleton;
    FigureView        figure;
    TextEditor        notesPage;
    ExpressionEditor  expressionEditor;
    KernelEditor      kernelEditor;

    //==========================================================================
    FigureModel model;
    Kernel kernel;
};
