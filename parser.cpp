#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

// Removes comments (both inline // and block /* */) from a line
string removeComments(const string& line) {
    string cleaned = line;
    size_t pos = cleaned.find("//");
    if (pos != string::npos)
        cleaned = cleaned.substr(0, pos); // Remove inline comment
    regex inlineBlockComment(R"(/\.?\*/)"); // Matches block comment start
    cleaned = regex_replace(cleaned, inlineBlockComment, ""); // Removes partial block comment
    return cleaned;
}

// Trims leading and trailing whitespace from a string
string trim(const string& str) {
    const string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

// Main parser function
int run_parser(const string& input_file, const string& output_file) {
    ifstream file(input_file);
    if (!file.is_open()) {
        cerr << "Failed to open " << input_file << "\n";
        return 1;
    }

    // Read and clean all lines from the input file
    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(removeComments(line));
    }

    // Regex to match dynamic memory allocations using typecasting (e.g., int* a = (int*)malloc(...))
    regex dmaRegex(R"((int|float|double|char)\s*\b([a-zA-Z_]\w*)\b\s*=\s*\([^)]+\)\s*(malloc|calloc|realloc)\s*\()");
    unordered_map<string, int> lastOccurrence; // Tracks last line where variable appears
    vector<string> dmaVars; // Stores names of dynamically allocated variables

    // Pass 1: Match variables declared and allocated in loops or normal lines
    for (int i = 0; i < lines.size(); ++i) {
        smatch match;
        string currentLine = trim(lines[i]);
        if (currentLine.find("for") != string::npos || currentLine.find("while") != string::npos) {
            if (regex_search(currentLine, match, dmaRegex)) {
                string varName = match[2];
                dmaVars.push_back(varName);
                lastOccurrence[varName] = i + 1;
            }
        } else if (regex_search(currentLine, match, dmaRegex)) {
            string varName = match[2];
            dmaVars.push_back(varName);
            lastOccurrence[varName] = i + 1;
        }
    }

    // Regex for pointer assignments without declaration (e.g., ptr = (type*)malloc(...))
    regex pointerAssignRegex(R"(\b([a-zA-Z_]\w*)\s*=\s*\([^)]+\)\s*(malloc|calloc|realloc)\s*\()");
    for (int i = 0; i < lines.size(); ++i) {
        smatch match;
        string currentLine = trim(lines[i]);
        if (currentLine.find("for") != string::npos || currentLine.find("while") != string::npos) {
            if (regex_search(currentLine, match, pointerAssignRegex)) {
                string varName = match[1];
                if (lastOccurrence.find(varName) == lastOccurrence.end()) {
                    dmaVars.push_back(varName);
                }
                lastOccurrence[varName] = i + 1;
            }
        } else if (regex_search(currentLine, match, pointerAssignRegex)) {
            string varName = match[1];
            if (lastOccurrence.find(varName) == lastOccurrence.end()) {
                dmaVars.push_back(varName);
            }
            lastOccurrence[varName] = i + 1;
        }
    }

    // Pass 2: Track last usage of each DMA variable across the code
    for (const string& var : dmaVars) {
        regex usageRegex("\\b" + var + "\\b");
        for (int i = 0; i < lines.size(); ++i) {
            if (regex_search(lines[i], usageRegex)) {
                lastOccurrence[var] = i + 1;
            }
        }
    }

    // Sort the variable:line mappings and write to output file
    vector<pair<string, int>> sorted(lastOccurrence.begin(), lastOccurrence.end());
    sort(sorted.begin(), sorted.end());

    ofstream outFile(output_file);
    for (const auto& entry : sorted) {
        outFile << entry.first << " : " << entry.second << "\n";
    }

    // Detect reassignment without freeing
    unordered_map<string, bool> isFreed;
    for (int i = 0; i < lines.size(); ++i) {
        smatch match;
        string currentLine = trim(lines[i]);

        // Check for reallocation
        if (regex_search(currentLine, match, pointerAssignRegex)) {
            string varName = match[1];
            if (isFreed.find(varName) != isFreed.end() && !isFreed[varName]) {
                cout << "Warning: Variable '" << varName << "' reassigned without being freed at line " << i + 1 << "\n";
            }
            isFreed[varName] = false;
        }

        // Detect free(varName)
        regex freeRegex(R"(free\s*\(\s*([a-zA-Z_]\w*)\s*\))");
        if (regex_search(currentLine, match, freeRegex)) {
            string varName = match[1];
            isFreed[varName] = true;
        }
    }

    // Detect pointer assignments from function calls
    regex funcPtrAssignRegex(R"((int|float|double|char)\s*\*+\s*([a-zA-Z_]\w*)\s*=\s*([a-zA-Z_]\w*)\s*\([^;]*\))");
    unordered_map<string, int> funcInitLine;  // Line where pointer is initialized by a function
    unordered_map<string, bool> funcVarUsed;  // Whether the variable is later used

    // Pass 3: Track initialization from functions
    for (int i = 0; i < lines.size(); ++i) {
        smatch match;
        string currentLine = trim(lines[i]);
        if (regex_search(currentLine, match, funcPtrAssignRegex)) {
            string varName = match[2];
            funcInitLine[varName] = i + 1;
            funcVarUsed[varName] = false;
        }
    }

    // Pass 4: Mark usage of function-initialized pointers
    for (int i = 0; i < lines.size(); ++i) {
        for (auto& entry : funcInitLine) {
            string varName = entry.first;
            regex usageRegex("\\b" + varName + "\\b");
            if (regex_search(lines[i], usageRegex) && lines[i].find(varName + " =") == string::npos) {
                funcVarUsed[varName] = true;
            }
        }
    }

    // Warn if function-initialized pointers are declared but never used
    for (const auto& entry : funcInitLine) {
        string varName = entry.first;
        int lineNum = entry.second;
        if (!funcVarUsed[varName]) {
            cout << "Warning: Function-initialized pointer '" << varName << "' at line " << lineNum << " is never used.\n";
        }
    }

    cout << "Output written to " << output_file << "\n";
    return 0;
}
