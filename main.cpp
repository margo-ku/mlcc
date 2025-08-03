#include <iostream>
#include <string>
#include <vector>

#include "include/driver/driver.h"

struct Options {
    bool debug_parse = false;
    bool debug_scan = false;
    bool print_ast = false;
    bool compile = true;
    std::vector<std::string> files;
};

Options ParseCommandLine(int argc, char* argv[]) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            opts.debug_parse = true;
        } else if (arg == "-s") {
            opts.debug_scan = true;
        } else if (arg == "--print-ast") {
            opts.print_ast = true;
        } else if (arg == "--print-ast-only") {
            opts.print_ast = true;
            opts.compile = false;
        } else {
            opts.files.push_back(arg);
        }
    }
    return opts;
}

int RunCompiler(const std::string& filename, const Options& opts) {
    Driver driver;
    driver.debug_parse = opts.debug_parse;
    driver.debug_scan = opts.debug_scan;
    driver.print_ast = opts.print_ast;
    driver.compile = opts.compile;
    return driver.CompileFile(filename);
}

int main(int argc, char* argv[]) {
    Options opts = ParseCommandLine(argc, argv);

    if (opts.files.empty()) {
        std::cerr << "Usage: compiler [-p] [-s] [--print-ast] [--print-ast-only] "
                     "<source-files>\n";
        return 1;
    }

    int exit_code = 0;
    for (const auto& file : opts.files) {
        std::cout << "==> Processing: " << file << "\n";
        exit_code = RunCompiler(file, opts);
        if (exit_code != 0) {
            return exit_code;
        }

        if (opts.compile) {
            std::filesystem::path path(file);
            std::string asm_file = path.replace_extension(".s").string();
            std::string out_file = path.replace_extension("").string();

            std::string clang_cmd = "clang " + asm_file + " -o " + out_file;
            std::cout << "Running: " << clang_cmd << std::endl;
            int clang_res = std::system(clang_cmd.c_str());
            if (clang_res != 0) {
                std::cerr << "clang failed with exit code " << clang_res << "\n";
                return clang_res;
            }
        }
    }
    return 0;
}
