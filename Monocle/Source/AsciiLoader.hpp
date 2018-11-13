#pragma once
#define DONT_SET_USING_JUCE_NAMESPACE 1
#include <vector>
#include <string>
#include "JuceHeader.h"




namespace mcl {
	class AsciiLoader;
}




// =============================================================================
class mcl::AsciiLoader
{
public:
    AsciiLoader (std::istream& stream);
    unsigned long getNumColumns() const;
    unsigned long getNumRows() const;
    std::vector<double> getColumnData (int index) const;
    std::vector<double> getRowData (int index) const;
    std::string getColumnName (int index) const;
    bool hasColumnWithName (std::string name) const;
    juce::String getStatusMessage() const;
private:
    unsigned long numColumns = 0;
    unsigned long numRows = 0;
    std::vector<double> data;
    std::vector<std::string> names;
    juce::String status;
};
