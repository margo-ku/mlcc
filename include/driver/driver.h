#pragma once

#include <fstream>
#include <map>
#include <string>

#include "include/ast/translation_unit.h"
#include "parser.hh"
#include "scanner.h"

class Driver {
public:
    Driver();
    int result;
    int parse(const std::string& f);
    std::string file;
    bool trace_parsing;

    void scan_begin();
    void scan_end();

    bool trace_scanning;
    yy::location location;
    bool location_debug;

    friend class Scanner;
    Scanner scanner;
    yy::parser parser;
    TranslationUnit* translation_unit;

private:
    std::ifstream stream;
};