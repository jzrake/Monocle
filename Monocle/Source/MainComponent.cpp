#include "MainComponent.hpp"
#include "MaterialIcons.hpp"
#include "Runtime.hpp"




//==============================================================================
MainComponent::MainComponent()
{
    kernel.insert ("list", var::NativeFunction (Runtime::builtin_list));
    kernel.insert ("dict", var::NativeFunction (Runtime::builtin_dict));
    kernel.insert ("data", JSON::fromString ("[1, 2, 3]"));
    kernel.insert ("expr", crt::parser::parse ("(a b c)"));

    kernel.insert ("array", new Runtime::Data<nd::ndarray<double, 1>>());

    skeleton.addNavButton ("Kernel",   material::bintos (material::action::ic_list));
    skeleton.addNavButton ("Files",    material::bintos (material::file::ic_folder_open));
    skeleton.addNavButton ("Notes",    material::bintos (material::action::ic_speaker_notes));
    skeleton.addNavButton ("Settings", material::bintos (material::action::ic_settings));
    skeleton.setMainContent (figure);
    skeleton.setNavPage ("Notes", notesPage);
    skeleton.setNavPage ("Kernel", kernelEditor);
    skeleton.setBackdrop ("Kernel", expressionEditor);

    notesPage.setMultiLine (true);
    notesPage.setReturnKeyStartsNewLine (true);
    notesPage.setBorder ({0, 0, 0, 0});
    notesPage.setTextToShowWhenEmpty ("Notes", Colours::lightgrey);
    notesPage.setFont (Font ("Optima", 14, 0));

    figure.setModel (model = FigureModel::createExample());
    kernelEditor.setKernel (&kernel);

    figure          .addListener (this);
    expressionEditor.addListener (this);
    kernelEditor    .addListener (this);

    addAndMakeVisible (skeleton);
    setSize (800, 600);
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

//==========================================================================
void MainComponent::expressionEditorNewExpression (const crt::expression& expr)
{
    auto key = kernelEditor.getEmphasizedKey();

    if (key.empty())
    {
        return;
    }
    kernel.insert (key, expr);
    kernelEditor.setKernel (&kernel);
    expressionEditor.setExpression (expr);
}

void MainComponent::expressionEditorParserError (const std::string& what)
{
    DBG(what);
}

//==========================================================================
void MainComponent::kernelEditorSelectionChanged()
{
}

void MainComponent::kernelEditorRulePunched (const std::string& key)
{
    if (! kernel.contains (key))
    {
        jassertfalse;
        return;
    }
    if (! kernel.at (key).isVoid() && kernel.expr_at (key).empty())
    {
        jassertfalse;
        return;
    }
    if (kernelEditor.getEmphasizedKey() != key)
    {
        auto expr = kernel.expr_at (key);
        expressionEditor.setExpression (expr);
        kernelEditor.setEmphasizedKey (key);
        skeleton.setBackdropRevealed (true);
    }
    else
    {
        expressionEditor.setExpression ({});
        kernelEditor.setEmphasizedKey (std::string());
        skeleton.setBackdropRevealed (false);
    }
}

void MainComponent::kernelEditorWantsNewRule (const crt::expression& expr)
{
    assert(! expr.key().empty());
    kernel.insert (expr.key(), expr.keyed (std::string()));
    kernelEditor.setKernel (&kernel);
    kernelEditor.setEmphasizedKey (expr.key());
    expressionEditor.setExpression (expr.keyed (std::string()));
}

void MainComponent::kernelEditorWantsRuleRemoved (const std::string& key)
{
    kernel.erase (key);
    kernelEditor.selectNext();
    kernelEditor.setKernel (&kernel);
    expressionEditor.setExpression ({});
}

void MainComponent::kernelEditorWantsRuleRenamed (const std::string& oldKey, const std::string& newKey)
{
    auto val = kernel.at (oldKey);
    auto expr = kernel.expr_at (oldKey);

    kernel.erase (oldKey);

    if (! expr.empty())
    {
        kernel.insert (newKey, expr);
    }
    else
    {
        kernel.insert (newKey, val);
    }
    kernelEditor.setKernel (&kernel);
    kernelEditor.selectRule (newKey);
}
