#include "MainComponent.hpp"
#include "MaterialIcons.hpp"
#include "Runtime.hpp"




//==============================================================================
MainComponent::MainComponent()
{
    kernel.insert ("list", var::NativeFunction (Runtime::list), Runtime::locked);
    kernel.insert ("dict", var::NativeFunction (Runtime::dict), Runtime::locked);
    kernel.insert ("item", var::NativeFunction (Runtime::item), Runtime::locked);
    kernel.insert ("attr", var::NativeFunction (Runtime::attr), Runtime::locked);
    kernel.insert ("add", var::NativeFunction (Runtime::add), Runtime::locked);
    kernel.insert ("sub", var::NativeFunction (Runtime::sub), Runtime::locked);
    kernel.insert ("mul", var::NativeFunction (Runtime::mul), Runtime::locked);
    kernel.insert ("div", var::NativeFunction (Runtime::div), Runtime::locked);
    kernel.insert ("file", var::NativeFunction (Runtime::file), Runtime::locked);

    kernel.insert ("data", JSON::fromString ("[1, 2, 3, ['a', 'b', 'c', [1, 2, 3]]]"));
    kernel.insert ("a", crt::parser::parse ("(add 1 2)"));
    kernel.insert ("b", crt::parser::parse ("(add a 3)"));
    kernel.insert ("array", new Runtime::Data<nd::ndarray<double, 1>>());
    kernel.insert ("data-file", crt::parser::parse ("(file '/Users/jzrake/Desktop/test.dat')"));

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
    kernel.update_all (kernel.dirty_rules());
    kernelEditor.setKernel (&kernel);

    figure          .addListener (this);
    expressionEditor.addListener (this);
    kernelEditor    .addListener (this);

    startTimer (100);
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
        CommandIDs::kernelCreateRule,
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
        case CommandIDs::kernelCreateRule:
            result.setInfo ("Create Rule", "", CommandCategories::edit, 0);
            result.defaultKeypresses.add (KeyPress ('T', ModifierKeys::ctrlModifier, 0));
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
        case CommandIDs::kernelCreateRule: createNewRule ("new-rule", {}); return true;
        default: return false;
    }
}

//==========================================================================
void MainComponent::timerCallback()
{
    for (const auto& rule : kernel)
    {
        if (rule.second.expr.size() >= 2)
        {
            if (rule.second.expr.at(0) == Runtime::Symbols::file)
            {
                if (auto fileDict = rule.second.value.getDynamicObject())
                {
                    auto& status = Runtime::Data<WatchedFile>::check (fileDict->getProperty ("status"));

                    if (status.hasChanged())
                    {
                        DBG("touching file " << status.getFile().getFullPathName());
                        kernel.touch (rule.first);
                    }
                }
            }
        }
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
void MainComponent::updateKernel (const Kernel::set_t& dirty)
{
    kernel.update_all (dirty);
    kernelEditor.setKernel (&kernel);
}

void MainComponent::createNewRule (const std::string& key, const crt::expression& expr)
{
    updateKernel (kernel.insert (key, expr));
    expressionEditor.setExpression (expr);
    kernelEditor.setEmphasizedKey (key);
    kernelEditor.selectRule (key);
    kernelEditor.showEditorInSelectedItem();
    skeleton.setBackdropRevealed (true);
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
    updateKernel (kernel.insert (key, expr));
    expressionEditor.setExpression (expr);
}

void MainComponent::expressionEditorEncounteredError (const std::string& what)
{
    skeleton.flashAlertLabel (what);
}

//==========================================================================
void MainComponent::kernelEditorSelectionChanged()
{
}

void MainComponent::kernelEditorRulePunched (const std::string& key)
{
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
    createNewRule (expr.key(), expr.keyed (std::string()));
}

void MainComponent::kernelEditorWantsRuleRemoved (const std::string& key)
{
    if (key == kernelEditor.getEmphasizedKey())
    {
        expressionEditor.setExpression ({});
        kernelEditor.setEmphasizedKey (std::string());
        skeleton.setBackdropRevealed (false);
    }
    kernelEditor.selectNext();
    updateKernel (kernel.erase (key));
}

void MainComponent::kernelEditorWantsRuleRelabeled (const std::string& from, const std::string& to)
{
    kernel.relabel (from, to);
    kernelEditor.setKernel (&kernel);
    kernelEditor.selectRule (to);

    if (kernelEditor.getEmphasizedKey() == from)
    {
        kernelEditor.setEmphasizedKey (to);
    }
}

void MainComponent::kernelEditorEncounteredError (const std::string& what)
{
    skeleton.flashAlertLabel (what);
}
