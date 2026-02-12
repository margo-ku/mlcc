#pragma once

#include <fstream>
#include <memory>
#include <string>

#include "include/ast/translation_unit.h"
#include "include/semantic/symbol_table.h"
#include "include/tac/instruction.h"
#include "parser.hh"
#include "scanner.h"

class Driver {
public:
    Driver();

    int CompileFile(const std::string& filename);
    void SetTranslationUnit(std::unique_ptr<TranslationUnit> unit);
    void SetFileName(const std::string& name);

    bool debug_parse = false;
    bool debug_scan = false;
    bool print_ast = false;
    bool compile = true;
    bool debug_output = false;

    friend class Scanner;

private:
    bool Scan();
    bool Parse();
    bool AnalyzeSemantics();
    bool GenerateTAC();
    bool OptimizeTAC();
    bool GenerateASM();

    void ScanBegin();
    void ScanEnd();

    std::string ReplaceExtension(const std::string& filename,
                                 const std::string& new_ext) const;

    std::string file_;
    std::string original_filename_;
    std::ifstream stream_;

    yy::location location_;
    Scanner scanner_;
    yy::parser parser_;
    std::unique_ptr<TranslationUnit> translation_unit_;
    SymbolTable symbol_table_;
    std::vector<std::vector<TACInstruction>> tac_instructions_;
};
