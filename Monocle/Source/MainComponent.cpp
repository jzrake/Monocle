#include "MainComponent.hpp"
#include "MaterialIcons.hpp"
#include "Kernel/Builtin.hpp"
#include "Kernel/Expression.hpp"
#include "Loaders.hpp"
#include "NumericData.hpp"




//==============================================================================
DefinitionEditor::DefinitionEditor()
{
    font = Font ("Monaco", 14, 0);

    symbolNameEditor.setMultiLine (false);
    symbolNameEditor.setReturnKeyStartsNewLine (false);
    symbolNameEditor.setBorder ({0, 0, 0, 0});
    symbolNameEditor.setTextToShowWhenEmpty ("Symbol name", Colours::lightgrey);
    symbolNameEditor.setFont (font);
    symbolNameEditor.addListener (this);
    symbolNameEditor.setVisible (false);
    symbolNameEditor.setColour (TextEditor::ColourIds::outlineColourId, Colours::lightgrey);

    cancelIcon = material::util::icon (material::navigation::ic_cancel, Colours::grey);
    commitIcon = material::util::icon (material::navigation::ic_check, Colours::green);

    addAndMakeVisible (symbolNameEditor);
}

void DefinitionEditor::setValidator (Validator validatorToUse)
{
    validator = validatorToUse;
}

void DefinitionEditor::addListener (Listener* listener)
{
    listeners.add (listener);
}

void DefinitionEditor::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void DefinitionEditor::setSymbolToEdit (const std::string& key, const std::string& expression)
{
    symbolNameEditor.setText (key);
    parts.clear();

    for (const auto& part : mcl::Expression (expression).getListParts())
        parts.add (part);

    repaint();
}

void DefinitionEditor::addPart (const String& part)
{
    parts.add (part);
    repaint();
}

void DefinitionEditor::cancel()
{
    parts.clear();
    symbolNameEditor.clear();
    listeners.call (&Listener::definitionEditorCanceled);
    repaint();
}

void DefinitionEditor::commit()
{
    jassert (isCommittable());
    listeners.call (&Listener::definitionEditorCommited, getKey(), getExpression());
    symbolNameEditor.clear();
    parts.clear();
    repaint();
}

bool DefinitionEditor::isCommittable() const
{
    return validator (getKey(), getExpression()).empty();
}

std::string DefinitionEditor::getKey() const
{
    return symbolNameEditor.getText().toStdString();;
}

std::string DefinitionEditor::getExpression() const
{
    if (parts.size() == 1)
        return parts.joinIntoString (" ").toStdString();
    return ("(" + parts.joinIntoString (" ") + ")").toStdString();
}

//==============================================================================
void DefinitionEditor::resized()
{
    symbolNameEditor.setBounds (computeGeometry().editorArea);
}

void DefinitionEditor::paint (Graphics& g)
{
    auto geom = computeGeometry();

    g.fillAll (Colours::lightgrey);
    g.setColour (Colours::black);
    g.setFont (font);

    if (true)            cancelIcon->drawWithin (g, geom.cancelIconArea.reduced (5).toFloat(), RectanglePlacement::stretchToFit, 1.f);
    if (isCommittable()) commitIcon->drawWithin (g, geom.commitIconArea.reduced (5).toFloat(), RectanglePlacement::stretchToFit, 1.f);

    g.drawMultiLineText (parts.joinIntoString ("\n"), geom.listArea.getX(), geom.listArea.getY(), 10000);
    g.drawText (" = ", geom.equalsSignArea, Justification::centredLeft);

    if (validator)
    {
        g.setColour (Colours::darkred);
        g.setFont (font.withHeight (10));
        g.drawMultiLineText (validator (getKey(), getExpression()),
                             geom.validationMessageArea.getX(),
                             geom.validationMessageArea.getY(),
                             geom.validationMessageArea.getWidth());
    }
}

void DefinitionEditor::mouseDown (const MouseEvent& e)
{
    if (computeGeometry().cancelIconArea.toFloat().contains (e.position))
    {
        cancel();
    }
    else if (computeGeometry().commitIconArea.toFloat().contains (e.position))
    {
        if (isCommittable())
        {
            commit();
        }
    }
}

//==============================================================================
void DefinitionEditor::textEditorTextChanged (TextEditor&)
{
    repaint();
}

void DefinitionEditor::textEditorReturnKeyPressed (TextEditor&)
{
    if (isCommittable())
        commit();
}

void DefinitionEditor::textEditorEscapeKeyPressed (TextEditor&)
{
    cancel();
}

//==============================================================================
DefinitionEditor::Geometry DefinitionEditor::computeGeometry() const
{
    int rowHeight = 22;
    int rowTop = (getHeight() - rowHeight) / 2;

    Geometry g;
    g.cancelIconArea = Rectangle<int> (rowHeight / 4, rowTop, rowHeight, rowHeight);
    g.commitIconArea = Rectangle<int> (g.cancelIconArea.getRight(), rowTop, rowHeight, rowHeight);
    g.editorArea     = Rectangle<int> (g.commitIconArea.getRight() + rowHeight / 4, rowTop, 100, rowHeight);
    g.equalsSignArea = Rectangle<int> (g.editorArea.getRight(), rowTop, font.getStringWidth (" = "), rowHeight);
    g.listArea       = getLocalBounds().withTrimmedLeft (g.equalsSignArea.getRight()).reduced (0, 14);
    g.validationMessageArea = Rectangle<int>::leftTopRightBottom (rowHeight / 2,
                                                                  g.editorArea.getBottom() + rowHeight,
                                                                  g.equalsSignArea.getRight(),
                                                                  getHeight());
    return g;
}




//==============================================================================
MainComponent::MainComponent()
{
    skeleton.addNavButton ("Symbols",  material::bintos (material::action::ic_list));
    skeleton.addNavButton ("Files",    material::bintos (material::file::ic_folder_open));
    skeleton.addNavButton ("Notes",    material::bintos (material::action::ic_speaker_notes));
    skeleton.addNavButton ("Settings", material::bintos (material::action::ic_settings));
    skeleton.setMainContent (figure);
    skeleton.setNavPage ("Notes", notesPage);
    skeleton.setNavPage ("Files", fileListAndDetail);
    skeleton.setNavPage ("Symbols", symbolListAndDetail);
    skeleton.setBackdrop ("Symbols", definitionEditor);

    notesPage.setMultiLine (true);
    notesPage.setReturnKeyStartsNewLine (true);
    notesPage.setBorder ({0, 0, 0, 0});
    notesPage.setTextToShowWhenEmpty ("Notes", Colours::lightgrey);
    notesPage.setFont (Font ("Optima", 14, 0));

    fileListAndDetail.setContent1 (fileList);
    fileListAndDetail.setContent2 (fileDetails);

    symbolListAndDetail.setContent1 (symbolList);
    symbolListAndDetail.setContent2 (symbolDetails);

    fileManager.setPollingInterval (100);
    figure     .setModel (model = FigureModel::createExample());

    definitionEditor.setValidator ([this] (const auto& key, const auto& expr) -> std::string
    {
        if (key.empty() || expr.empty())
        {
            return " ";
        }
        try {
            auto e = mcl::Expression (expr);
            kernel.throwIfWouldCreateCycle (key, e.symbols());
        }
        catch (std::exception& e) {
            return e.what();
        }
        return std::string();
    });

    fileManager     .addListener (this);
    fileList        .addListener (this);
    symbolList      .addListener (this);
    symbolDetails   .addListener (this);
    fileDetails     .addListener (this);
    definitionEditor.addListener (this);
    figure          .addListener (this);

    addAndMakeVisible (skeleton);
    setSize (800, 600);

    mcl::Expression::testParser();

    // Initial kernel configuration
    // ========================================================================
    kernel.setListener ([this] (const std::string& key, const mcl::Object& val)
    {
        auto status = kernel.status (key);
        symbolList.updateSymbolStatus (key, status);

        if (symbolDetails.getCurrentSymbol() == key)
            symbolDetails.setViewedObject (key, val);

        if (key == "F")
        {
            try {
                figure.setModel (model = val.get_data<FigureModel>());
            }
            catch (...) {
                figure.setModel (model = FigureModel::createExample());
            }
        }
    });

    kernel.setErrorLog ([this] (const std::string& key, const std::string& msg) { DBG("error: " << key << " " << msg); });
    kernel.import (mcl::Builtin::builtin());
    kernel.import (Loaders::loaders());
    kernel.import (PlotModels::plot_models());

    kernel.insert ("L", mcl::Object::Expr ("(line-plot x y)"));
    kernel.insert ("F", mcl::Object::Expr ("(figure L)"));

    symbolList.setSymbolList (kernel.status (kernel.select()));
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    skeleton.setBounds (getLocalBounds());
}

//==========================================================================
ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return nullptr;
}

void MainComponent::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::windowToggleBackdrop,
        CommandIDs::windowToggleNavPages,
    };
    commands.addArray (ids, numElementsInArray (ids));
}

void MainComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::windowToggleNavPages:
            result.setInfo ("Toggle Navigation Pages", "", CommandCategories::window, 0);
            result.defaultKeypresses.add (KeyPress ('K', ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::windowToggleBackdrop:
            result.setInfo ("Toggle Backdrop", "", CommandCategories::window, 0);
            result.defaultKeypresses.add (KeyPress ('L', ModifierKeys::commandModifier, 0));
            break;
        default: break;
    }
}

bool MainComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::windowToggleNavPages: skeleton.toggleNavPagesRevealed(); return true;
        case CommandIDs::windowToggleBackdrop: skeleton.toggleBackdropRevealed(); return true;
        default: return false;
    }
}

//==========================================================================
bool MainComponent::isInterestedInFileDrag (const StringArray& files)
{
    return true;
}

void MainComponent::fileDragEnter (const StringArray& files, int x, int y)
{
    skeleton.openNavSection ("Files");
}

void MainComponent::fileDragMove (const StringArray& files, int x, int y)
{
}

void MainComponent::fileDragExit (const StringArray& files)
{
}

void MainComponent::filesDropped (const StringArray& files, int x, int y)
{
}

//==========================================================================
void MainComponent::fileManagerFileChangedOnDisk (File file)
{
    fileList.updateFileDisplayStatus (file);
    fileDetails.updateFileDetailsIfShowing (file);
    kernel.touch (File (file).getFileNameWithoutExtension().toStdString());
}

//==========================================================================
void MainComponent::fileListFilesInserted (const StringArray& files, int index)
{
    fileManager.insertFiles (files, index);
    fileList.setFileList (fileManager.getFiles());

    for (const auto& file : files)
    {
        auto key = File (file).getFileNameWithoutExtension().toStdString();
        fileManager.setUniqueKey (file, key);
        kernel.insert (key, file.toStdString());
    }
}

void MainComponent::fileListFilesRemoved (const StringArray& files)
{
    for (const auto& file : files)
    {
        kernel.remove (fileManager.getUniqueKey (file));
    }
    fileManager.removeFiles (files);
    fileList.setFileList (fileManager.getFiles());
}

void MainComponent::fileListSelectionChanged (const StringArray& files)
{
    fileDetails.setCurrentlyActiveFiles (files);
}

void MainComponent::fileListWantsToApplyFilter (const StringArray& files, const String& name)
{
    if (! files.isEmpty())
    {
        skeleton.openNavSection ("Symbols");
        symbolList.deselectAllRows();
    }
    for (const auto& file : files)
    {
        auto fileKey = fileManager.getUniqueKey (file);
        auto newSymbol = fileKey + "-data";
        kernel.insert (newSymbol, mcl::Object::expr ("(load-txt " + fileKey + ")"));
        symbolList.addKeyToSelection (newSymbol);
    }
}

//==========================================================================
void MainComponent::symbolListSelectionChanged (const StringArray& symbols)
{
    if (symbols.size() == 1)
        symbolDetails.setViewedObject (symbols[0].toStdString(), kernel.concrete (symbols[0].toStdString()));
    else
        symbolDetails.setViewedObject ("", mcl::Object());
}

void MainComponent::symbolListSymbolsRemoved (const StringArray& symbols)
{
    for (const auto& key : symbols)
        kernel.remove (key.toStdString());
}

void MainComponent::symbolListSymbolPunched (const String& symbol)
{
    definitionEditor.addPart (symbol);
    skeleton.setBackdropRevealed (true);
}

void MainComponent::symbolListExpressionShouldBeEdited (const String& key)
{
    definitionEditor.setSymbolToEdit (key.toStdString(), kernel.abstract (key.toStdString()).get<mcl::Object::Expr>().source);
    skeleton.setBackdropRevealed (true);
    definitionEditor.grabKeyboardFocus();
}

//==========================================================================
void MainComponent::symbolDetailsItemPunched (const std::string& expression)
{
    definitionEditor.addPart (expression);
    skeleton.setBackdropRevealed (true);
}

void MainComponent::symbolDetailsWantsNewDefinition (const std::string& key, const std::string& expression)
{
    kernel.insert (key, mcl::Object::expr (expression));
    symbolList.selectOnlyKeys ({key});
}

//==========================================================================
void MainComponent::definitionEditorCommited (const std::string& key, const std::string& expression)
{
    kernel.insert (key, mcl::Object::expr (expression));
    skeleton.setBackdropRevealed (false);
    symbolList.grabKeyboardFocus();
}

void MainComponent::definitionEditorCanceled()
{
    skeleton.setBackdropRevealed (false);
    symbolList.grabKeyboardFocus();
}

//==========================================================================
void MainComponent::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
{
    model.margin = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    model.xmin = domain.getX();
    model.xmax = domain.getRight();
    model.ymin = domain.getY();
    model.ymax = domain.getBottom();
    figure->setModel (model);
}

void MainComponent::figureViewSetXlabel (FigureView* figure, const String& value)
{
    model.xlabel = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetYlabel (FigureView* figure, const String& value)
{
    model.ylabel = value;
    figure->setModel (model);
}

void MainComponent::figureViewSetTitle (FigureView* figure, const String& value)
{
    model.title = value;
    figure->setModel (model);
}
