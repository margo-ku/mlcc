#include "include/driver/driver.h"

#include <iostream>

#include "include/visitors/compile_visitor.h"
#include "include/visitors/print_visitor.h"
#include "parser.hh"

Driver::Driver() : scanner_(*this), parser_(scanner_, *this) {}

int Driver::Parse(const std::string& filename) {
    file_ = filename;
    location_.initialize(&file_);

    ScanBegin();
    parser_.set_debug_level(debug_parse);
    int result = parser_();

    if (compile) {
        std::string asm_file = file_;
        size_t dot = asm_file.find_last_of('.');
        if (dot != std::string::npos) {
            asm_file.replace(dot, std::string::npos, ".s");
        } else {
            asm_file += ".s";
        }

        std::cout << "asm file: " << asm_file << std::endl;

        std::ofstream out_file(asm_file);
        CompileVisitor compile_visitor(out_file);
        translation_unit_->Accept(&compile_visitor);
    }

    if (print_ast) {
        PrintVisitor print_visitor(std::cout);
        translation_unit_->Accept(&print_visitor);
    }

    ScanEnd();
    return result;
}

void Driver::ScanBegin() {
    scanner_.set_debug(debug_scan);

    if (!file_.empty() && file_ != "-") {
        stream_.open(file_);
        scanner_.yyrestart(&stream_);
    }
}

void Driver::ScanEnd() { stream_.close(); }

void Driver::SetTranslationUnit(TranslationUnit* unit) { translation_unit_ = unit; }