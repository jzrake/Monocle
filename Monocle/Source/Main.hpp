#pragma once
#include "JuceHeader.h"




//==============================================================================
//#include "Database.hpp"
//
//class DataSource
//{
//public:
//    virtual ~DataSource() {}
//    virtual Array<Database::Entry> load (File file) = 0;
//};
//
//
//
//
////==============================================================================
//class AsciiTableDataSource : public DataSource
//{
//public:
//    Array<Database::Entry> load (File file) override;
//};




//==============================================================================
/**
 Not used yet...
*/
class MonocleLookAndFeel : public LookAndFeel_V4
{
public:
    void fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor) override;
    void drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor) override;
};
