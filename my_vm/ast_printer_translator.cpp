#include "../my_include/ast_printer_translator.h"
#include "../my_include/ast_printer_visitor.h"
#include "../include/visitors.h"
#include "../vm/parser.h"

using namespace mathvm;

AstPrinterTranslatorImpl::AstPrinterTranslatorImpl() {}

Status *AstPrinterTranslatorImpl::translate(const string &program, Code **code) {
    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    AstPrinterVisitor visitor;
    FunctionNode * node = parser.top()->node();
    node->visitChildren(&visitor);
    cout << visitor.get_program();

    return status;
}

AstPrinterTranslatorImpl::~AstPrinterTranslatorImpl() {

}
