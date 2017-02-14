// VProf headers
#include "FileFinder.h"
#include "Utils.h"

// STL headers
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <iostream>
#include <array>

std::string FileFinder::cdCommand() {
    return std::string("cd " + sourceBaseDir);
}

// Build a cscope query which will return files which contain functionName
std::string FileFinder::buildQuery(const std::string &functionName) {
    return std::string(cdCommand() + " && cscope -L3" + functionName);
}

// Parses cscope output and returns a vector of the unique files 
// represented in the CScope output.
std::vector<std::string> FileFinder::parseCScopeOutput(const std::string &output) {
    std::stringstream ss;
    ss.str(output);

    std::vector<std::string> filenames;
    std::string line;

    while (getline(ss, line)) {
        // The filename is in index 0
        filenames.push_back(SplitString(line, ' ')[0]);
    }

    // Ensure files are unique
    std::sort(filenames.begin(), filenames.end());
    filenames.erase(std::unique(filenames.begin(), filenames.end()), filenames.end());

    return filenames;
}

std::vector<std::string> FileFinder::FindFunctionPotentialFiles(const std::string &functionName) {
    std::string query = buildQuery(functionName);

    std::string output = exec(query); 

    return parseCScopeOutput(output);
}

std::vector<std::string> FileFinder::FindFunctionsPotentialFiles(const std::vector<std::string> &functions) {
    std::vector<std::string> result;

    for (std::string &function : functions) {
        std::vector<std::string> functionResult = FindFunctionPotentialFiles(function);

        result.insert(result.end(), functionResult.begin(), functionResult.end());
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
}

FileFinder::FileFinder(const std::string _sourceBaseDir): sourceBaseDir(_sourceBaseDir) {}

void FileFinder::BuildCScopeDB() {
    exec(cdCommand() + " && cscope -R");
}
