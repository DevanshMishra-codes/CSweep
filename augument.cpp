#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

using namespace std;

// Reads a mapping file with format: <variable> : <line_number>
// Returns a map from variable name to line number
map<string, int> parse_mapping_file(const string& file_path) {
    ifstream file(file_path);
    map<string, int> mapping;
    string line;
    while (getline(file, line)) {
        size_t delim = line.find(" : ");
        if (delim != string::npos) {
            string key = line.substr(0, delim);
            string value = line.substr(delim + 3);
            // Trim whitespace from both ends
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            mapping[key] = stoi(value); // Store variable and its line
        }
    }
    return mapping;
}

// Reads the source code file and returns a map of line numbers to code lines
map<int, string> read_source_code(const string& file_path) {
    ifstream file(file_path);
    map<int, string> line_mapping;
    string line;
    int line_num = 1;
    while (getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        line_mapping[line_num++] = line;
    }
    return line_mapping;
}

// Inserts 'free(var);' after the last usage of each variable based on mapping
map<int, string> process_mapping_and_code(const map<string, int>& mapping, const map<int, string>& line_mapping) {
    map<int, string> updated_lines;
    for (auto it_map = mapping.begin(); it_map != mapping.end(); ++it_map) {
        const string& key = it_map->first;
        int value = it_map->second;
        auto it_line = line_mapping.find(value);
        if (it_line != line_mapping.end()) {
            // Append free statement after original line
            updated_lines[value] = it_line->second + "\n    free(" + key + ");";
        } else {
            updated_lines[value] = "";
        }
    }
    return updated_lines;
}

// Writes the updated code to a new file with 'sweeped' prefix
void write_updated_code_to_file(const map<int, string>& updated_lines, const string& original_file_path) {
    // Extract directory and filename
    size_t last_slash = original_file_path.find_last_of("/\\");
    string dir = (last_slash == string::npos) ? "" : original_file_path.substr(0, last_slash + 1);
    string filename = (last_slash == string::npos) ? original_file_path : original_file_path.substr(last_slash + 1);
    size_t dot = filename.find_last_of('.');
    string base = (dot == string::npos) ? filename : filename.substr(0, dot);
    string new_file_name = "sweeped" + base + ".c";
    string new_file_path = dir + new_file_name;

    ifstream original_file(original_file_path);
    ofstream new_file(new_file_path);

    string line;
    int line_num = 1;
    while (getline(original_file, line)) {
        // If this line has an update, write the modified line
        auto it = updated_lines.find(line_num);
        if (it != updated_lines.end() && !it->second.empty()) {
            new_file << it->second << endl;
        } else {
            new_file << line << endl;
        }
        ++line_num;
    }

    cout << "Updated code written to: " << new_file_path << endl;
}

// Main function that runs the augmentation step by calling helper functions
int run_augment(const string& mapping_file, const string& source_file) {
    auto mapping = parse_mapping_file(mapping_file);         // Read variable to line map
    auto line_mapping = read_source_code(source_file);       // Read original source code lines
    auto updated_lines = process_mapping_and_code(mapping, line_mapping); // Add 'free' where needed
    write_updated_code_to_file(updated_lines, source_file);  // Write new file
    return 0;
}
