#include "include/driver/driver.h"

#include "include/asm/ir_builder.h"
#include "include/semantic/analyzer.h"
#include "include/visitors/print_visitor.h"

Driver::Driver() : scanner_(*this), parser_(scanner_, *this) {}

int Driver::CompileFile(const std::string& filename) {
    file_ = filename;
    location_.initialize(&filename);

    bool ok = Scan() && Parse();

    if (ok && print_ast) {
        if (debug_output) {
            std::cout << "Printing parsed AST:" << std::endl;
        }
        PrintVisitor printer(std::cout);
        translation_unit_->Accept(&printer);
    }

    if (ok && compile) {
        ok = AnalyzeSemantics() && GenerateTAC() && GenerateASM();
    }

    ScanEnd();
    return ok ? 0 : 1;
}

void Driver::ScanBegin() {
    scanner_.set_debug(debug_scan);

    if (!file_.empty() && file_ != "-") {
        stream_.open(file_);
        scanner_.yyrestart(&stream_);
    }
}

void Driver::ScanEnd() { stream_.close(); }

void Driver::SetTranslationUnit(std::unique_ptr<TranslationUnit> unit) {
    translation_unit_ = std::move(unit);
}

bool Driver::Scan() {
    ScanBegin();
    scanner_.set_debug(debug_scan);
    return true;
}

bool Driver::Parse() {
    parser_.set_debug_level(debug_parse);
    return parser_() == 0;
}

bool Driver::AnalyzeSemantics() {
    SemanticAnalyzer analyzer;
    analyzer.Analyze(translation_unit_.get());
    if (analyzer.HasErrors()) {
        for (const auto& error : analyzer.GetErrors()) {
            std::cerr << error << std::endl;
        }
        return false;
    }
    return true;
}

bool Driver::GenerateTAC() {
    TACVisitor tac_visitor;
    translation_unit_->Accept(&tac_visitor);
    tac_instructions_ = tac_visitor.GetTACInstructions();

    std::string tac_file = ReplaceExtension(original_filename_, ".tac.txt");
    if (debug_output) {
        std::cout << "Generated TAC: " << tac_file << std::endl;
    }
    std::ofstream out(tac_file);
    tac_visitor.PrintTACInstructions(out);
    return true;
}

bool Driver::GenerateASM() {
    LinearIRBuilder builder(tac_instructions_);
    builder.Build();

    std::string asm_file = ReplaceExtension(original_filename_, ".s");
    if (debug_output) {
        std::cout << "Generated ASM: " << asm_file << std::endl;
    }
    std::ofstream out(asm_file);
    builder.Print(out);
    return true;
}

std::string Driver::ReplaceExtension(const std::string& filename,
                                     const std::string& new_ext) const {
    size_t dot = filename.find_last_of('.');
    if (dot != std::string::npos) {
        return filename.substr(0, dot) + new_ext;
    }
    return filename + new_ext;
}

void Driver::SetFileName(const std::string& name) { original_filename_ = name; }