#pragma once

#include <fstream>
#include <string>

#include "include/ast/translation_unit.h"
#include "parser.hh"
#include "scanner.h"

class Driver {
public:
    Driver();

    int Parse(const std::string& filename);
    void SetTranslationUnit(TranslationUnit* unit);

    bool debug_parse = false;
    bool debug_scan = false;
    bool print_ast = false;
    bool compile = true;

    friend class Scanner;

private:
    void ScanBegin();
    void ScanEnd();

    std::string file_;
    std::ifstream stream_;

    yy::location location_;
    Scanner scanner_;
    yy::parser parser_;
    TranslationUnit* translation_unit_;  // to do!
};
