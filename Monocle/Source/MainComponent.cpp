#include "MainComponent.hpp"
#include "MaterialIcons.hpp"
#include "Kernel/Builtin.hpp"
#include "Loaders.hpp"




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

    fileManager.addListener (this);
    fileList   .addListener (this);
    symbolList .addListener (this);
    fileDetails.addListener (this);
    figure     .addListener (this);

    addAndMakeVisible (skeleton);
    setSize (800, 600);

    // Initial kernel configuration
    // ========================================================================
    kernel.setListener ([this] (const std::string& key, const mcl::Object& val)
    {
        auto status = kernel.status (key);
        symbolList.updateSymbolStatus (key, status);

        if (symbolDetails.getCurrentSymbol() == key)
            symbolDetails.setViewedObject (key, val);
    });

    kernel.setErrorLog ([this] (const std::string& key, const std::string& msg) { DBG("error: " << key << " " << msg); });
    kernel.import (mcl::Builtin::builtin());
    kernel.import (Loaders::loaders());
    kernel.import (PlotModels::plot_models());
//    kernel.insert ("test-object", mcl::Object::expr ("(list (dict a=123 b=543) 2 3 (list 5 6 7 (list 6 7 8 9 10 2 load-txt 4 3 2 1 3 4 4)))"));
    kernel.insert ("data1", mcl::Object::expr ("(load-txt test)"));
    kernel.insert ("plot1", mcl::Object::expr ("(line-plot (attr data1 'A') (attr data1 'B'))"));
    kernel.insert ("fig1", mcl::Object::expr ("(figure plot1)"));

    //kernel.insert ("fig1", mcl::Object::expr ("(figure plot1 limits=fig1:limits title=fig1:title xlabel=fig1:xlabel ylabel=fig1:ylabel margins=fig:margins)"));
    //kernel.insert ("fig1", mcl::Object::expr ("(concat (figure plot1) fig1:format)"));

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
        kernel.insert (File (file).getFileNameWithoutExtension().toStdString(), file.toStdString());
}

void MainComponent::fileListFilesRemoved (const StringArray& files)
{
    for (const auto& file : files)
        kernel.remove (File (file).getFileNameWithoutExtension().toStdString());

    fileManager.removeFiles (files);
    fileList.setFileList (fileManager.getFiles());
}

void MainComponent::fileListSelectionChanged (const StringArray& files)
{
    fileDetails.setCurrentlyActiveFiles (files);
}

//==========================================================================
void MainComponent::kernelListSelectionChanged (const StringArray& symbols)
{
    if (symbols.size() == 1)
        symbolDetails.setViewedObject (symbols[0].toStdString(), kernel.concrete (symbols[0].toStdString()));
    else
        symbolDetails.setViewedObject ("", mcl::Object());
}

void MainComponent::kernelListSymbolsRemoved (const StringArray& symbols)
{
    for (const auto& key : symbols)
        kernel.remove (key.toStdString());
}

//==========================================================================
//void MainComponent::filterNameChanged (const String& newName)
//{
//    for (const auto& file : fileList.getSelectedFullPathNames())
//    {
//        fileManager.setFilterName (file, newName);
//        fileDetails.setFilterIsValid (newName == "Ascii");
//    }
//}

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
