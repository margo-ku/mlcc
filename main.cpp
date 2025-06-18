#include <iostream>

#include "include/driver/driver.h"

int RunCompiler(const std::string& filename, bool trace_parsing,
                bool trace_scanning, bool print_ast = false) {
    Driver driver;
    driver.trace_parsing = trace_parsing;
    driver.trace_scanning = trace_scanning;
    return driver.parse(filename);
}

int main(int argc, char* argv[]) {
    bool trace_parsing = false;
    bool trace_scanning = false;
    std::vector<std::string> input_files;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            trace_parsing = true;
        } else if (arg == "-s") {
            trace_scanning = true;
        } else {
            input_files.push_back(arg);
        }
    }

    if (input_files.empty()) {
        std::cerr << "Usage: compiler [-p] [-s] <source-files>\n";
        return 1;
    }

    int exit_code = 0;
    for (const auto& file : input_files) {
        std::cout << "==> Processing: " << file << "\n";
        exit_code = RunCompiler(file, trace_parsing, trace_scanning);
    }

    return exit_code;
}