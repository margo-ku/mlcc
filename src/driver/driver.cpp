#include "include/driver/driver.h"

#include <iostream>

#include "include/visitors/compile_visitor.h"
#include "include/visitors/print_visitor.h"
#include "parser.hh"

Driver::Driver()
    : trace_parsing(false),
      trace_scanning(false),
      location_debug(false),
      scanner(*this),
      parser(scanner, *this) {}

int Driver::parse(const std::string& f) {
    file = f;
    location.initialize(&file);
    scan_begin();
    parser.set_debug_level(trace_parsing);
    int res = parser();

    std::string asm_file = f;
    asm_file[asm_file.size() - 1] = 's';
    std::cout << "asm file: " << asm_file << std::endl;

    std::ofstream out_file;
    out_file.open(asm_file);
    CompileVisitor compile_visitor(out_file);
    translation_unit->Accept(&compile_visitor);
    scan_end();
    return res;
}

void Driver::scan_begin() {
    scanner.set_debug(trace_scanning);
    if (file.empty() || file == "-") {
    } else {
        stream.open(file);
        // std::cout << file << std::endl;
        scanner.yyrestart(&stream);
    }
}

void Driver::scan_end() { stream.close(); }