#include <iostream>
#include <string>
#include <vector>

#include "include/driver/driver.h"

struct Options {
    bool debug_parse = false;
    bool debug_scan = false;
    bool print_ast = false;
    bool compile = true;
    bool debug_output = false;
    bool keep_asm = false;
    bool keep_tac = false;
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
        } else if (arg == "--debug") {
            opts.debug_output = true;
        } else if (arg == "--keep-asm") {
            opts.keep_asm = true;
        } else if (arg == "--keep-tac") {
            opts.keep_tac = true;
        } else {
            opts.files.push_back(arg);
        }
    }
    return opts;
}

int RunCompiler(const std::string& original_file, const std::string& preprocessed_file,
                const Options& opts) {
    Driver driver;
    driver.debug_parse = opts.debug_parse;
    driver.debug_scan = opts.debug_scan;
    driver.print_ast = opts.print_ast;
    driver.compile = opts.compile;
    driver.debug_output = opts.debug_output;

    driver.SetFileName(original_file);

    if (opts.debug_output) {
        std::cout << "Running mlcc compiler: " << std::endl;
    }

    int result = driver.CompileFile(preprocessed_file);
    return result;
}

std::string RunPreprocessor(const std::string& filename, const Options& opts) {
    std::string preprocessed_file =
        std::filesystem::temp_directory_path() / "tmp_preprocessed.c";
    std::string command = "clang -E " + filename + " -o " + preprocessed_file;

    if (opts.debug_output) {
        std::cout << "Running preprocessor: " << command << std::endl;
    }

    int res = std::system(command.c_str());
    if (res != 0) {
        std::cerr << "Preprocessing failed\n";
        exit(res);
    }

    return preprocessed_file;
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
        if (opts.debug_output) {
            std::cout << "==> Processing: " << file << "\n";
        }
        std::string preprocessed = RunPreprocessor(file, opts);
        exit_code = RunCompiler(file, preprocessed, opts);
        if (exit_code != 0) {
            return exit_code;
        }

        if (opts.compile) {
            std::filesystem::path path(file);
            std::string asm_file = path.replace_extension(".s").string();
            std::string out_file = path.replace_extension("").string();

            std::string clang_cmd = "clang " + asm_file + " -o " + out_file;

            if (opts.debug_output) {
                std::cout << "Running: " << clang_cmd << std::endl;
            }

            int clang_res = std::system(clang_cmd.c_str());
            if (clang_res != 0) {
                std::cerr << "clang failed with exit code " << clang_res << "\n";
                return clang_res;
            }

            if (opts.compile && !opts.keep_asm) {
                std::filesystem::remove(asm_file);
            }

            if (!opts.keep_tac) {
                std::filesystem::path tac_file = path.replace_extension(".tac.txt");
                std::filesystem::remove(tac_file);
                tac_file = path.replace_extension(".tac_optimized.txt");
                std::filesystem::remove(tac_file);
            }
        }
    }
    return 0;
}
