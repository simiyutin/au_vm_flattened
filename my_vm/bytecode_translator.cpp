#include "../include/mathvm.h"
#include "../include/ast.h"
#include "../vm/parser.h"
#include <fstream>
#include "../my_include/bytecode_translator_visitor.h"
#include "../my_include/interpreter.h"

using namespace mathvm;
using namespace std;

//code is output parameter
Status *BytecodeTranslatorImpl::translate(const string &program, Code **code) {

    Parser parser;
    Status * status = parser.parseProgram(program);
    if (status->isError()) {
        return status;
    }

    BytecodeTranslatorVisitor visitor;
    FunctionNode * node = parser.top()->node();

    node->visitChildren(&visitor);

    Bytecode bytecode = visitor.getBytecode();
    map<string, int> topMostVars = visitor.getTopMostVars();
    vector<string> stringConstants = visitor.getStringConstants();
    std::map<uint16_t, size_t> functionOffsets = visitor.getFunctionOffsetsMap();
    (*code) = new Interpreter(bytecode, topMostVars, stringConstants, functionOffsets);
    std::ofstream ofs("lastBytecode.txt");
    (*code)->disassemble(ofs);

    return status;
}
