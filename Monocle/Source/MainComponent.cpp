#include "MainComponent.hpp"
#include "MaterialIcons.hpp"



static var builtin_list (var::NativeFunctionArgs args)
{
    return Array<var>(args.arguments, args.numArguments);
}

static var builtin_dict (var::NativeFunctionArgs args)
{
    return args.thisObject;
}



// ============================================================================
class VarCallAdapter
{
public:
    using ObjectType = var;
    using list_t = std::vector<ObjectType>;
    using dict_t = std::unordered_map<std::string, ObjectType>;
    using func_t = std::function<ObjectType(list_t, dict_t)>;

    template<typename Mapping>
    static ObjectType call(const Mapping& scope,
                           const std::string& key,
                           const list_t& args,
                           const dict_t& kwar)
    {
        auto self = new DynamicObject;

        for (const auto& kw : kwar)
        {
            self->setProperty (String (kw.first), kw.second);
        }
        auto f = scope.at(key).getNativeFunction();
        auto a = var::NativeFunctionArgs (var (self), &args[0], int (args.size()));
        return f(a);
    }
};
using Kernel = crt::kernel<var, VarCallAdapter>;




//==============================================================================
class ExpressionEditorItem;
class ExpressionEditorItemView;




//==============================================================================
class MainComponent::ExpressionEditor : public TreeView
{
public:
    ExpressionEditor();
    bool keyPressed (const KeyPress& key) override;
private:
    std::unique_ptr<ExpressionEditorItem> root;
};




//==========================================================================
class ExpressionEditorItem : public TreeViewItem
{
public:
    ExpressionEditorItem (crt::expression expr);

    //==========================================================================
    void paintItem (Graphics& g, int width, int height) override;
    String getUniqueName() const override;
    bool mightContainSubItems() override;
    Component* createItemComponent() override;
    void itemClicked (const MouseEvent&) override;
    void itemDoubleClicked (const MouseEvent&) override;
    void itemSelectionChanged (bool isNowSelected) override;

    //==========================================================================
    crt::expression expr;
    Label label;
};




//==========================================================================
class ExpressionEditorItemView : public Component
{
public:
    ExpressionEditorItemView (ExpressionEditorItem& item, Label& label);
    void resized() override;
    void paint (Graphics& g) override;
    ExpressionEditorItem& item;
    Label& label;
};




//==========================================================================
ExpressionEditorItemView::ExpressionEditorItemView (ExpressionEditorItem& item, Label& label) : item (item), label (label)
{
    label.setText (item.expr.str(), NotificationType::dontSendNotification);
    label.setColour (Label::textColourId, Colours::black);
    label.setFont (Font ("Monaco", 12, 0));
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);
}

void ExpressionEditorItemView::resized()
{
    label.setBounds (getLocalBounds());
}

void ExpressionEditorItemView::paint (Graphics& g)
{
}




//==========================================================================
ExpressionEditorItem::ExpressionEditorItem (crt::expression expr) : expr (expr)
{
    setDrawsInLeftMargin (true);

    for (const auto& e : expr)
    {
        addSubItem (new ExpressionEditorItem (e));
    }
}

//==========================================================================
void ExpressionEditorItem::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
    {
        g.fillAll (Colours::lightblue);
    }
}

String ExpressionEditorItem::getUniqueName() const
{
    return expr.str();
}

bool ExpressionEditorItem::mightContainSubItems()
{
    return expr.dtype() == crt::data_type::composite;
}

Component* ExpressionEditorItem::createItemComponent()
{
    return new ExpressionEditorItemView (*this, label);
}

void ExpressionEditorItem::itemClicked (const MouseEvent&)
{
}

void ExpressionEditorItem::itemDoubleClicked (const MouseEvent&)
{
    label.showEditor();
}
void ExpressionEditorItem::itemSelectionChanged (bool isNowSelected)
{
}




//==============================================================================
MainComponent::ExpressionEditor::ExpressionEditor()
{
    crt::expression e {
        1,
        2.3,
        std::string("sdf"),
        crt::expression::symbol("a"),
        {1, crt::expression::symbol("b"), crt::expression::symbol("b")}};

    root = std::make_unique<ExpressionEditorItem>(e);

    setRootItem (root.get());
    setIndentSize (12);
    setMultiSelectEnabled (true);
    setRootItemVisible (true);
}

bool MainComponent::ExpressionEditor::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::returnKey && getNumSelectedItems() == 1)
    {
        dynamic_cast<ExpressionEditorItem*>(getSelectedItem(0))->label.showEditor();
        return true;
    }
    return TreeView::keyPressed (key);
}



//==============================================================================
class MainComponent::KernelView : public TreeView
{
public:
    KernelView()
    {
        kernel.insert ("list", var::NativeFunction (builtin_list));
        kernel.insert ("dict", var::NativeFunction (builtin_dict));
    }
private:
    Kernel kernel;
};




//==============================================================================
MainComponent::MainComponent()
{
    kernelView       = std::make_unique<KernelView>();
    expressionEditor = std::make_unique<ExpressionEditor>();

    skeleton.addNavButton ("Symbols",  material::bintos (material::action::ic_list));
    skeleton.addNavButton ("Files",    material::bintos (material::file::ic_folder_open));
    skeleton.addNavButton ("Notes",    material::bintos (material::action::ic_speaker_notes));
    skeleton.addNavButton ("Settings", material::bintos (material::action::ic_settings));
    skeleton.setMainContent (figure);
    skeleton.setNavPage ("Notes", notesPage);
    skeleton.setNavPage ("Symbols", *kernelView);
    skeleton.setBackdrop ("Symbols", *expressionEditor);

    notesPage.setMultiLine (true);
    notesPage.setReturnKeyStartsNewLine (true);
    notesPage.setBorder ({0, 0, 0, 0});
    notesPage.setTextToShowWhenEmpty ("Notes", Colours::lightgrey);
    notesPage.setFont (Font ("Optima", 14, 0));

    figure.setModel (model = FigureModel::createExample());
    figure.addListener (this);

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
