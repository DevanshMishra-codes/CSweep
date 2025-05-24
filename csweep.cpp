#include <iostream>
#include <string>
#include <cstdio>   // for std::remove
#include <cstdlib>  // for system()

using namespace std;

// Forward declarations
int run_parser(const string& input_file, const string& output_file);
int run_augment(const string& mapping_file, const string& source_file);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: csweep <source_file.c>\n";
        return 1;
    }
    string source_file = argv[1];
    string mapping_file = "output.txt";

    // Step 1: Run parser to generate mapping
    if (run_parser(source_file, mapping_file) != 0) {
        cerr << "Parser failed.\n";
        return 1;
    }

    // Step 2: Run augment to update code
    if (run_augment(mapping_file, source_file) != 0) {
        cerr << "Augment failed.\n";
        return 1;
    }

    // Step 3: Find the sweeped file name
    size_t last_slash = source_file.find_last_of("/\\");
    string filename = (last_slash == string::npos) ? source_file : source_file.substr(last_slash + 1);
    size_t dot = filename.find_last_of('.');
    string base = (dot == string::npos) ? filename : filename.substr(0, dot);
    string sweeped_file = "sweeped" + base + ".c";

    // Step 4: Compile the sweeped file to a.exe
    string compile_cmd = "gcc \"" + sweeped_file + "\" -o a.exe";
    int compile_result = system(compile_cmd.c_str());
    if (compile_result != 0) {
        cerr << "Compilation failed.\n";
        remove(mapping_file.c_str());
        remove(sweeped_file.c_str());
        return 1;
    }

    // Step 5: Delete intermediate files
    remove(mapping_file.c_str());
    remove(sweeped_file.c_str());

    cout << "CSweep completed successfully. Executable saved as a.exe\n";
    return 0;
}